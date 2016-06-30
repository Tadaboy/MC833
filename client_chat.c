#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_LINE 256
#define MAX_NAME 10

void command_send ();
void command_create_group ();
void command_join_group (); 
void command_send_group ();
void command_who ();
void command_exit ();

int main(int argc, char * argv[])
{
    struct hostent *hp;
    struct sockaddr_in sin;
    char* host;
    char name[MAX_NAME];
    char buf[MAX_LINE];
    int s;
    int len;
    int server_port;
    int i, n;
    pid_t child;
    
    
    if (argc==4) {
        host = argv[1];
    }
    else {
        fprintf(stderr, "usage: ./client host port name\n");
		exit(1);
	}
	
	server_port = atoi(argv[2]);
	
	if (strlen(argv[3]) == 0 || strlen(argv[2]) > MAX_NAME) {
		fprintf(stderr, "name maximum length: %d", MAX_NAME);
		exit(1);
	}
	else {
		strcpy(name, argv[3]);
	}
		

    /* translate host name into peer’s IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(server_port);


	/* active open TCP*/
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk: socket");
		exit(1);
	}
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("simplex-talk: connect");
		close(s);
		exit(1);
	}

	strcpy(buf, name);
	len = strlen(buf)+1;
	if (send(s, buf, len, 0) < 0) {
		perror("send");
		exit(1);
	}
	if (recv(s, buf, sizeof(buf), 0) <= 0) {
		perror("name already connected");
		exit(1);
	}

	child = fork();
	if(child == -1) {
		perror("fork");
		close(s);
		exit(1);
	}
	
	if(child == 0) {
		/* main loop: get and send lines of text */
		while (fgets(buf, sizeof(buf), stdin)) {
			buf[MAX_LINE-1] = '\0';
			len = strlen(buf) + 1;
					
			if(strncmp(buf, "SEND ", 5) == 0) {
				if (send(s, buf, len, 0) < 0) {
					perror("send");
					exit(1);
				}
			}
			else if (strncmp(buf, "CREATEG ", 8) == 0) {
				if (send(s, buf, len, 0) < 0) {
					perror("send");
					exit(1);
				}
			}
			else if (strncmp(buf, "JOING ", 6) == 0) {
				
				if (send(s, buf, len, 0) < 0) {
					perror("send");
					exit(1);
				}	
			}
			else if (strncmp(buf, "SENDG ", 6) == 0) {
				command_send_group();
			}
			else if (strncmp(buf, "WHO" , 3) == 0) {
				strcpy(buf, "WHO");
			
				if (send(s, buf, len, 0) < 0) {
					perror("send");
					exit(1);
				}
				
			}
			else if (strncmp(buf, "EXIT", 4) == 0) {
				break;
			}
			else
				printf("Comando invalido!\n");
		}
	}
	else if (child != 0) {
		int jindex = 6;
		int cindex = 8;
		int sindex;
		int sgindex;
		while(1) {
			memset(buf, 0, sizeof(buf));
			if ((len = recv(s, buf, sizeof(buf), 0)) < 0) {
				perror("recv");
				exit(1);
			}
			printf("len: %d\n", len);
			if(strncmp(buf, "WHO", 3) == 0) {
				printf("WHO\n");
				printf("%c   usuario  %c status  %c\n",124, 124, 124);
				char* aux = buf + 3;
				n = atoi(aux);
				aux = aux + 7;
				for(i = 0; i < n; i++) {
					printf("%c", 124);
					printf(" %*.*s %c ", MAX_NAME,MAX_NAME, aux, 124);
					aux = aux + MAX_NAME;
					if(strncmp(aux, "1", 1) == 0) {
						printf("ONLINE  %c\n",  124);
					}
					else {
						printf("OFFLINE %c\n",124);
					}
					aux = aux + 1;	
				}
			}
			else if (strncmp(buf, "CREATEG ", 8) == 0 || cindex == 0) {
				if(buf[cindex] == 0) {
					if(cindex == 8)
						cindex = 0;
				}
				else if(buf[cindex] == '1') {
					printf("Grupo criado com sucesso!\n");
					cindex = 8;
				}
				else {
					printf("Nao possivel criar o grupo\n");
					cindex = 8;
				}
			}
			else if (strncmp(buf, "JOING ", 6) == 0 || jindex == 6) {
				if(buf[jindex] == 0)
					jindex = 0;
				else if(buf[jindex] == 'j') {
					printf("Entrou no grupo!\n");
					jindex = 6;
				}
				else if (buf[jindex] == 'm') {
					printf("Voce ja eh membro!\n");
					jindex = 6;
				}
				else {
					printf("Nao existe esse grupo!\n");
					jindex = 6;
				}
				
			}
			else if (strncmp(buf, "SEND ", 5) == 0) {
				printf("SEND\n");
				char* remetente = buf+5;
				char* msg = remetente + strlen(remetente);
				printf("[%s] %s\n", remetente, msg);
				
			}
			else if (strncmp(buf, "SENDG ", 6) == 0) {
				printf("SENDG\n");
				
			}
		}
	}
	close(s);
	return 0;
}
void command_send (){
}
void command_create_group () {
}
void command_join_group () {
}
void command_send_group () {
}
void command_who () {
}
void command_exit () {
}


