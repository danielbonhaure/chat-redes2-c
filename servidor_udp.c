
/*		PARA COMPILAR		 */
/*    gcc -o servidor servidor_udp.c     */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>


void error (char *s) { exit ((perror(s),-1)); }

typedef enum {true=1, false=0} bool;

int main(int argc, char *argv[]) {

	if (argc != 2) printf("usage: <port_serv>\n"), exit(1);
	int puerto = strtol(argv[1], 0, 10);
	
	struct sockaddr_in addrInServ;
	bzero(&addrInServ,sizeof(addrInServ));
	addrInServ.sin_family      = AF_INET;
	addrInServ.sin_addr.s_addr = htonl(INADDR_ANY);
	addrInServ.sin_port        = htons(puerto);

	struct sockaddr_in addrInCli;
	struct sockaddr*   addrCli  = (struct sockaddr*)&addrInCli;
	struct sockaddr*   addrServ = (struct sockaddr*)&addrInServ;
	int                addrLen  = sizeof(struct sockaddr_in);

	int sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0) exit ((perror("socket"),-1));

	int rb = bind (sd, addrServ, addrLen);
	if (rb < 0) exit ((perror("bind"),-1));

	socklen_t len;
   	char mesg[MAX_DATO];
	int cto;
	
	printf("-------------------------------------------------------\n");
	
	for (;;)
	{
		len  =  sizeof(struct sockaddr_in);
		if ((cto = recvfrom(sd,mesg,MAX_DATO,0,(struct sockaddr *)&addrInCli,&len)) < 0) exit ((perror("recvfrom"),-1));
      		printf("Datagrama correcto desde %s puerto %d\n",inet_ntoa(addrInCli.sin_addr), ntohs(addrInCli.sin_port));
		mesg[cto] = '\0';
		if (sendto(sd,mesg,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));	
	}

	return 0;
}

