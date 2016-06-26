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
#define UDP 1
#define TCP 2

int main(int argc, char * argv[])
{
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;
    char buf[MAX_LINE];
    int s,protocol;
    int len;
    struct sockaddr_in servaddr;
    
    
    if (argc==3) {
        host = argv[1];
    }
    else {
        fprintf(stderr, "usage: ./client host protocol\n");
		exit(1);
	}
	
	if (strcmp(argv[2], "udp") == 0)
		protocol = UDP;
	else if (strcmp(argv[2], "tcp") == 0)
		protocol = TCP;
	else {
		fprintf(stderr, "protocol must be udp or tcp\n");
		exit(1);
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
    sin.sin_port = htons(SERVER_PORT);


	if (protocol == UDP) {
		/* active open UDP*/
		if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("simplex-talk: socket");
			exit(1);
		}

		/* main loop: get and send lines of text */
		while (fgets(buf, sizeof(buf), stdin)) {
			buf[MAX_LINE-1] = '\0';
			unsigned int servlen = sizeof(servaddr);
			len = strlen(buf) + 1;
			if(sendto(s, buf, len, 0, (struct sockaddr*)&sin, sizeof(sin)) < 0){
				perror("sendto");
				exit(1);
			}
			/*recv para receber a msg de volta do server e puts pra exibir*/
			while(recvfrom(s, buf, MAX_LINE, 0, (struct sockaddr *) &servaddr, &servlen)){
				/*espera por resposta do servidor para onde a mensagem foi enviada*/
				if(servaddr.sin_addr.s_addr == sin.sin_addr.s_addr && servaddr.sin_port == htons(SERVER_PORT)){
					fputs(buf, stdout); 
					break;
				}
			}	
		}
	}
	else if (protocol == TCP) {
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
		/* main loop: get and send lines of text */
		while (fgets(buf, sizeof(buf), stdin)) {
			buf[MAX_LINE-1] = '\0';
			len = strlen(buf) + 1;
			if (send(s, buf, len, 0) < 0) {
				perror("send");
				exit(1);
			}
			/*recv para receber a msg de volta do server e puts pra exibir*/
			recv(s, buf, sizeof(buf), 0);
			fputs(buf, stdout);		
		}
	}
}



