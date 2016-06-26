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


