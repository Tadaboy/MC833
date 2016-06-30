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

void command_send ();
void command_create_group ();
void command_join_group (); 
void command_send_group ();
void command_who ();
void command_exit ();

int main(int argc, char * argv[])
{
	int	maxfd, listenfd, connfd, sockfd;
	int 	len,i,nready;
	int 	clientsNumber = 0;
	Usuario	client[FD_SETSIZE];
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
					clientsNumber++;
                                        if (sockfd >= FD_SETSIZE || sockfd < 0) {
                                                perror("FD_CLR error");
                                                exit(1);
          				}
                                        FD_CLR(sockfd, &allset);
                                        client[i].userfd = -1;
                                } else
                                        send(sockfd, buf, len, 0);

                                if (--nready <= 0)
                                        break;                          /* no more readable descriptors */
                        }
                }
        }
}

void command_send(){
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

