#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <strings.h>

#define LISTENQ 5
#define MAXLINE 256

int main(int argc, char * argv[])
{
	int	maxfd, udpfd, listenfd, connfd;
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

	/* UDP */
	if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket error");
		return 1;
	}

	if (bind(udpfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)  {
		perror("bind error");
		close(udpfd);
		return 1;
	}

	if (listenfd > udpfd)
		maxfd = listenfd;
	else
		maxfd = udpfd;


	FD_ZERO(&allset);
	if (listenfd >= FD_SETSIZE || listenfd < 0) {
		perror("FD_SET error (invalid listenfd)");
		exit(1);
	}
	FD_SET(listenfd, &allset);
	if (udpfd >= FD_SETSIZE || udpfd < 0) {
		perror("FD_SET error (invalid udpfd)");
		exit(1);
	}
	FD_SET(udpfd, &allset);

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


		if (FD_ISSET(udpfd, &rset)) { /*new datagram */
			clilen = sizeof(cliaddr);
		
			if((len = recvfrom(udpfd, buf, MAXLINE, 0, (struct sockaddr *) &cliaddr, &clilen)) < 0){
				perror("recvfrom error");
				exit(1);
			}
			
			if(sendto(udpfd, buf, len, 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0){
				perror("sendto error");
				exit(1);
			}
		}

	}
}
