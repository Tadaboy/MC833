#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <strings.h>
#include <string.h>

#define LISTENQ 5
#define MAXLINE 256
#define MAXNAME 10

typedef struct _Usuario {
	int userfd;
	char name[MAXNAME];
} Usuario;

typedef struct Elemento {
    int vertice;
    struct Elemento* proximo;
} Elemento;

typedef struct Lista {
    int tamanho;
    Elemento* inicio;
} Lista;

typedef struct _Grupo {
	char name[MAXNAME];
	Lista membros;
} Grupo;
/*
  Inicializa os campos da estrutura Lista.  Esta funcao deve ser
  chamada antes que qualquer elemento seja inserido. Complexidade:
  constante.
  Parametros:
  lista: apontador para a lista
*/
void inicializa(Lista* lista);

/*
  Insere um elemento no inicio da lista. Complexidade: constante.
  Parametros:
  lista: apontador para a lista
  vertice: inteiro representando o vertice a ser inserido
*/
void insere(Lista* lista, int vertice);

/*
  Remove todos os elementos da lista. Complexidade: linear no tamanho
  da lista.
  Parametros:
  lista: apontador para a lista
*/
void esvazia(Lista* lista);

void command_send ();
void command_create_group (int* groupsNumber, int sockfd, int fundador, char* buf, Grupo* grupo);
void command_join_group (int groupsNumber, int sockfd, int candidato, char* buf, Grupo* grupo); 
void command_send_group ();
void command_who (Usuario client[], int clientsNumber, int sockfd);
void command_exit ();

int main(int argc, char * argv[])
{
	int	maxfd, listenfd, connfd, sockfd;
	int 	len,i,nready,size;
	int 	clientsNumber = 0, groupsNumber = 0;
	Usuario	client[FD_SETSIZE];
	Grupo	grupo[FD_SETSIZE];
	fd_set	rset, allset;
	char	buf[MAXLINE];
	socklen_t	clilen;
	struct sockaddr_in	cliaddr, servaddr;
	int serv_port;

	if (argc==2) {
        serv_port = atoi(argv[1]);
    }
    else {
        fprintf(stderr, "usage: ./server port\n");
		exit(1);
	}

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket error");
		return 1;
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(serv_port);

	/* SO_REUSEADDR */
	/*int activate = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &activate, sizeof(activate));*/

	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)  {
		perror("bind error");
		close(listenfd);
		return 1;
	}

	if (listen(listenfd, LISTENQ) < 0) {
		perror("listen error");
		close(listenfd);
		return 1;
	}

	maxfd = listenfd;
	for(i = 0; i < FD_SETSIZE; i++) {
		client[i].userfd = -1;
		client[i].name[0] = '\0';
		grupo[i].name[0] = '\0';
		inicializa(&(grupo[i].membros));
	}

	FD_ZERO(&allset);
	if (listenfd >= FD_SETSIZE || listenfd < 0) {
		perror("FD_SET error (invalid listenfd)");
		exit(1);
	}
	FD_SET(listenfd, &allset);
        for ( ; ; ) {
                rset = allset;          /* structure assignment */
                if ((nready = select(maxfd+1, &rset, NULL, NULL, NULL)) < 0){
                        perror("select error");
                        exit(1);
                }

                if (FD_ISSET(listenfd, &rset)) {        /* new client connection */
                        clilen = sizeof(cliaddr);
                        if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
                                perror("accept error");
                                exit(1);
                        }

			if ((len = recv(connfd, buf, MAXLINE, 0)) < 0) {
				perror("recv name error");
				exit(1);
			}

			printf("New Connection!\n");	
			for(i = 0; i < clientsNumber; i++) {
				if(strcmp(client[i].name, buf) == 0) {
					printf("Old User!\n");
					break;
				}
			}
			
                        if (i == FD_SETSIZE) {
                                perror("too many clients");
                                exit(1);
                        }

                        if (client[i].userfd < 0) {
                                client[i].userfd = connfd;     /* save descriptor */
				if(i == clientsNumber) {
					clientsNumber++;
					strcpy(client[i].name, buf);
					printf("New client registered!\n");
				}
				else {
					printf("Welcome back %s\n", client[i].name);
				}
                       		FD_SET(connfd, &allset);        /* add new descriptor to set */
	                        if (connfd > maxfd)
        	                        maxfd = connfd;                 /* for select */
				if (send(connfd, buf, len, 0) < 0) {
					perror("send confirm name");
					exit(1);
				}
            }
			else {
				close(connfd); //already connected
			}
                        if (connfd >= FD_SETSIZE || connfd < 0) {
                                perror("FD_SET error (invalid connfd)");
                                exit(1);
                        }

                        if (--nready <= 0)
                                continue;                               /* no more readable descriptors */
                }

                for (i = 0; i <= clientsNumber; i++) {   /* check all clients for data */
                        if ( (sockfd = client[i].userfd) < 0)
                                continue;
                        if (FD_ISSET(sockfd, &rset)) {
                                if ( (len = read(sockfd, buf, MAXLINE)) == 0) {
                                                /* connection closed by client */
                                        close(sockfd);
										printf("Client %s logout!\n", client[i].name);
                                        if (sockfd >= FD_SETSIZE || sockfd < 0) {
                                                perror("FD_CLR error");
                                                exit(1);
										}
                                        FD_CLR(sockfd, &allset);
                                        client[i].userfd = -1;
                                } else {
									if((size = strlen(buf)) < len - 2) {
									}
									else {
										if(strncmp(buf, "WHO", 3) == 0) {
											printf("WHO\n");
											command_who(client, clientsNumber, sockfd);
										}
										else if (strncmp(buf, "CREATEG ", 8) == 0) {
											printf("CREATEG\n");
											command_create_group(&groupsNumber, sockfd, i, buf+8, grupo);
										}
										else if (strncmp(buf, "JOING ", 6) == 0) {
											printf("JOING\n");
											command_join_group(groupsNumber, sockfd, i, buf+6, grupo);
										}
										else if (strncmp(buf, "SEND ", 5) == 0) {
											printf("SEND\n");
											command_send(client[i].name, buf, clientsNumber, client);
										}
										else if (strncmp(buf, "SENDG ", 6) == 0) {
											printf("SENDG\n");
											command_send_group();
										}
										
									}
                                        //send(sockfd, buf, len, 0);
								}
                                if (--nready <= 0)
                                        break;                          /* no more readable descriptors */
                        }
                }
        }
}

void command_send(char* name, char* buf, int clientNumber, Usuario* client){
	
	int targetfd = -1;
	char aux[MAXNAME];
	int i,j = MAXNAME+4;
	for(i=0;i<MAXNAME;i++) {
		if (buf[i+5] != ' ')
			aux[i] = buf[i+5];
		else {
			j = i+6;
			while(i < MAXNAME) {
				aux[i] = 0;
				i++;
			}
			break;
		}
	}
	
	
	for(i = 0; i < clientNumber; i++) {
		if(strcmp(client[i].name, aux) == 0) {
			targetfd = client[i].userfd;
		}
	}
	if (targetfd == -1) {
		perror("offline");
		exit(1);
	}
	
	char mensagem[MAXLINE];
	char msg[MAXLINE];
	for(i=0;i<MAXLINE;i++) {
		if(buf[j+i] != 0) {
			mensagem[i] = buf[j+i];
		}
		else {
			while(i < MAXLINE) {
				mensagem[i] = 0;
				i++;
			}
			break;
		}
	}
		
	sprintf(mensagem, "SEND ");
	sprintf(mensagem+5, "%s", name);
	sprintf(mensagem+5+strlen(name)+1, "%s\n\0", msg);
	
	if (send(targetfd, mensagem, strlen(mensagem), 0) < 0) {
		perror("send confirm send");
		exit(1);
	}
}
void command_create_group (int* groupsNumber, int sockfd, int fundador, char* buf, Grupo* grupo) {
	
	int i;
	if (send(sockfd, "CREATEG ", 8*sizeof(char), 0) < 0) {
		perror("send confirm createg");
		exit(1);
	}
	for(i = 0; i < *groupsNumber; i++) {
		if(strcmp(grupo[i].name, buf) == 0) {
			printf("Group already exists!\n");
			i = i - 1;
			break;
		}
	}
	if (i == FD_SETSIZE) {
		perror("too many groups");
		exit(1);
	}

	if(i == *groupsNumber) {
		(*groupsNumber)++;
		printf("Numero de grupos criados: %d\n", *groupsNumber);
		strcpy(grupo[i].name, buf);
		insere(&(grupo[i].membros), fundador);
		printf("New group created!\n");
		if (send(sockfd, "1\n", 2*sizeof(char), 0) < 0) {
			perror("send confirm create");
			exit(1);
		}
	}
	else {
		if(send(sockfd, "0\n", 2*sizeof(char), 0) < 0) {
			perror("send error");
			exit(1);
		}
	}
}
void command_join_group (int groupsNumber, int sockfd, int candidato, char* buf, Grupo* grupo) {
	int i;
	
	if(send(sockfd, "JOING ", 6*sizeof(char), 0) < 0){
		perror("send join");
		exit(1);						
	}	
	
	for(i = 0; i < groupsNumber; i++) {
		if(strcmp(grupo[i].name, buf) == 0) {
			Elemento* member = grupo[i].membros.inicio;
			while (member != NULL) {
				if(member->vertice == candidato) {
					if(send(sockfd, "m\n", 2*sizeof(char), 0) < 0){
						perror("send join already");
						exit(1);						
					}
					return;
				}
				member = member->proximo;
			}
			insere(&(grupo[i].membros), candidato);
			if(send(sockfd, "j\n", 2*sizeof(char), 0) < 0){
				perror("send join success");
				exit(1);						
			}	
			return;
		}
	}
	if(send(sockfd, "n\n", 2*sizeof(char), 0) < 0){
		perror("send join dont exist");
		exit(1);						
	}
	return;
}
void command_send_group () {
}
void command_who (Usuario client[], int clientsNumber, int sockfd) {
	int i;
	char buf[7];
	char who[3];
	sprintf(who,"%s","WHO");
	if (send(sockfd, who, sizeof(who), 0) < 0) {
		perror("send who");
		exit(1);
	}
	sprintf(buf, "%d", clientsNumber);
	if (send(sockfd, buf, sizeof(buf), 0) < 0) {
		perror("send who number");
		exit(1);
	}
	for(i = 0; i < clientsNumber; i++) {
		if (send(sockfd, client[i].name, sizeof(client[i].name), 0) < 0) {
			perror("send who 1");
			exit(1);
		}
		if (client[i].userfd < 0) {
			if (send(sockfd, "0", sizeof(char), 0) < 0) {
				perror("send who 2");
				exit(1);
			}
		}
		else if (send(sockfd, "1", sizeof(char), 0) < 0) {
				perror("send who 3");
				exit(1);
		}
	}
	if (send(sockfd, "\n", sizeof(char), 0) < 0) {
		perror("send who 2");
		exit(1);
	}		
}

void command_exit () {
}

void inicializa(Lista* lista) {
    lista->tamanho = 0;
    lista->inicio = NULL;
}

void insere(Lista* lista, int vertice) {
    Elemento* novo = (Elemento*)malloc(sizeof(Elemento));
    novo->vertice = vertice;
    novo->proximo = lista->inicio;
    lista->tamanho++;
    lista->inicio = novo;
}

void esvazia(Lista* lista) {
    Elemento* atual = lista->inicio;
    while (atual != NULL) {
        Elemento* proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    lista->tamanho = 0;
    lista->inicio = NULL;
}

