
/*		PARA COMPILAR		 */
/* gcc -pthread -o cliente cliente_udp.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_DATO 1000

void error (char *s)
{
	exit ((perror(s),-1));
}

void thread_escuchar(void *s)
{

	char dato[MAX_DATO];
	int  n,sd = (int)s;
	char recvdata[MAX_DATO];

	struct sockaddr_in addrInServ;
	struct sockaddr*   addrServ = (struct sockaddr*)&addrInServ;
	int                addrLen  = sizeof(struct sockaddr_in);
	
	socklen_t len;

	for (;;)
	{
		len  =  sizeof(struct sockaddr_in);
		if ((n=recvfrom(sd,recvdata,MAX_DATO,0,(struct sockaddr *)&addrInServ,&len)) < 0) exit ((perror("recvfrom"),-1));
		printf("Datagrama correcto desde %s puerto %d\n",inet_ntoa(addrInServ.sin_addr), ntohs(addrInServ.sin_port));
		recvdata[n]=0;
		printf("recibido: %s",recvdata);
		printf("-------------------------------------------------------\n");
	}

}

int main(int argc, char *argv[]) {

	// ARGUMENTS
	if (argc != 4) printf("usage: <ip_serv> <port_serv> <port_cli>\n"), exit(1);
	int port_serv = strtol(argv[2], 0, 10);
	int port_cli = strtol(argv[3], 0, 10);

	// SERVIDOR
	struct sockaddr_in addrInServ;
	bzero(&addrInServ,sizeof(addrInServ));
	addrInServ.sin_family      = AF_INET;
	addrInServ.sin_addr.s_addr = inet_addr(argv[1]);
	addrInServ.sin_port        = htons(port_serv);

	struct sockaddr*   addrServ = (struct sockaddr*)&addrInServ;

	// CLIENTE
	struct sockaddr_in addrInCli;
	bzero(&addrInCli,sizeof(addrInCli));
	addrInCli.sin_family      = AF_INET;
	addrInCli.sin_addr.s_addr = htonl(INADDR_ANY);
	addrInCli.sin_port        = htons(port_cli);

	struct sockaddr*   addrCli  = (struct sockaddr*)&addrInCli;
	int                addrLen  = sizeof(struct sockaddr_in);

	// PREPARE SOCKET
	int sdCli = socket(AF_INET, SOCK_DGRAM, 0);
	if (sdCli < 0) exit ((perror("socket"),-1));

	int rbCli = bind (sdCli, addrCli, addrLen);
	if (rbCli < 0) exit ((perror("bind"),-1));

	// ESCUCHO AL SERVIDOR
	pthread_t thread;
	pthread_create (&(thread), 0, (void*) &thread_escuchar, (void*)sdCli);

	// INIT PROGRAM
	char senddata[MAX_DATO];

	printf("-------------------------------------------------------\n");
	while (fgets(senddata,MAX_DATO,stdin) != NULL)
	{
		
		printf("envio: %s",senddata);
		printf("-------------------------------------------------------\n");
		if (sendto(sdCli,senddata,strlen(senddata),0,(struct sockaddr *)&addrInServ,sizeof(addrInServ)) < 0) exit ((perror("sendto"),-1));
		
	}
	return 1;
}

