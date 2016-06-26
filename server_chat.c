#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <strings.h>

#define LISTENQ 5
#define MAXLINE 256

void command_send ();
void command_create_group ();
void command_join_group (); 
void command_send_group ();
void command_who ();
void command_exit ();

int main(int argc, char * argv[])
{
	int	maxfd, listenfd, connfd;
	int 	len;
	fd_set	rset, allset;
	char	buf[MAXLINE];
	socklen_t	clilen;
	struct sockaddr_in	cliaddr, servaddr;
	pid_t child;
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
	int activate = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &activate, sizeof(activate));

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


	FD_ZERO(&allset);
	if (listenfd >= FD_SETSIZE || listenfd < 0) {
		perror("FD_SET error (invalid listenfd)");
		exit(1);
	}
	FD_SET(listenfd, &allset);

	for ( ; ; ) {
		rset = allset;		/* structure assignment */
		if ((select(maxfd+1, &rset, NULL, NULL, NULL)) < 0){
			perror("select error");
			exit(1);
		}

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
				perror("accept error");
				exit(1);
			}

			/* fork */
			child = fork();
			if(child == -1) {
				perror("fork");
				exit(1);
			}
			/* processo filho */
			if(child == 0) {
				close(listenfd);

				while ((len = recv(connfd, buf, sizeof(buf), 0))){
					//fputs(buf, stdout);
					send(connfd, buf, len, 0);
				}
				close(connfd);
				exit(0);
			}
			close(connfd);
		}
	}
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

