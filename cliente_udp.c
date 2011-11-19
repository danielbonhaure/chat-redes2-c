
/*		PARA COMPILAR		 */
/* gcc -pthread -o cliente `xml2-config --cflags` cliente_udp.c `xml2-config --libs` */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <libxml/xpath.h>

#define MAX_DATO 1000

char nick[20];
char dest[20];

typedef enum {true=1, false=0} bool;
bool conectado;

void error (char *s) { exit ((perror(s),-1)); }		//captura errores, cierra el programa e informa del error

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
		printf("\n-------------------------------------------------------\n");

		xmlDocPtr doc; 

		doc = xmlParseMemory(recvdata,strlen(recvdata));
		if (doc == NULL) exit ((perror("parser\n"),-1));

		xmlNodePtr root;
		xmlNodePtr node;

		root = xmlDocGetRootElement(doc);
	
		if (!strcmp(root->name,"connect")){
			node = root->xmlChildrenNode;;
			if (!strcmp(node->name,"action")){
				char *text;
				text = xmlNodeGetContent(node);
				if (!strcmp(text,"ACCEPTED")) conectado = true;		
			}
		}
	}//fin del for(;;)

}

int main(int argc, char *argv[]) {

	sprintf(nick,"sin nick");
	sprintf(dest,"GLOBAL");
	conectado = false;
	// ARGUMENTS
	if (argc != 4) printf("usage: <ip_serv> <port_serv> <port_cli>\n"), exit(1);
	int port_serv = strtol(argv[2], 0, 10);
	int port_cli = strtol(argv[3], 0, 10);
	char *ptr;

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
	char texto[MAX_DATO];

	printf("-------------------------------------------------------\n");
	while (gets(texto))
	{
		if (!strcmp(texto,"salir")) break;
/*********************************************************************************************/
		if (!strncmp(texto,"#nick",5)) {
			if (!conectado){
				ptr = strtok(texto," \t");    // Primera llamada => Primer token
				while( (ptr = strtok( NULL," \t")) != NULL )    // Posteriores llamadas
					sprintf(nick,"%s",ptr);
				printf ("new nick: %s",nick);
				printf("\n-------------------------------------------------------\n");
			} else {
				printf ("Para cambiar su nick primero debe cerrar la sesion con el nick actual");
				printf("\n-------------------------------------------------------\n");
			}
			continue;			
		} else if (!strncmp(texto,"?nick",5)) {
			printf ("nick: %s",nick);
			printf("\n-------------------------------------------------------\n");
			continue;
		} else if (!strncmp(texto,"!connect",8)) {
			if (!strcmp(nick,"sin nick")){
				printf (" No puede conectarse sin haber introducido un nick.\n");
				printf("-------------------------------------------------------\n");
			} else {
				sprintf(senddata,"<connect><nick>%s</nick></connect>",nick);
				printf("envio: %s",senddata);
				printf("\n-------------------------------------------------------\n");
				if (sendto(sdCli,senddata,strlen(senddata),0,(struct sockaddr *)&addrInServ,sizeof(addrInServ)) < 0) exit ((perror("sendto"),-1));
				continue;
			}
		} else if (!strncmp(texto,"?admin",6)) {
			sprintf(senddata,"<chatroom><request>ADMIN</request></chatroom>");
			printf("envio: %s",senddata);
			printf("\n-------------------------------------------------------\n");
			if (sendto(sdCli,senddata,strlen(senddata),0,(struct sockaddr *)&addrInServ,sizeof(addrInServ)) < 0) exit ((perror("sendto"),-1));
			continue;
		} else if (!strncmp(texto,"?miembros",9)) {
			sprintf(senddata,"<chatroom><request>USERS</request></chatroom>");
			printf("envio: %s",senddata);
			printf("\n-------------------------------------------------------\n");
			if (sendto(sdCli,senddata,strlen(senddata),0,(struct sockaddr *)&addrInServ,sizeof(addrInServ)) < 0) exit ((perror("sendto"),-1));
			continue;
		}else if (!strncmp(texto,"#dest",5)) {
			ptr = strtok(texto," \t");    // Primera llamada => Primer token
			while( (ptr = strtok( NULL," \t")) != NULL )    // Posteriores llamadas
				sprintf(dest,"%s",ptr);
			printf ("new dest: %s",dest);
			printf("\n-------------------------------------------------------\n");
			continue;
		} else if (!strncmp(texto,"?dest",5)) {
			printf ("dest: %s",dest);
			printf("\n-------------------------------------------------------\n");
			continue;
		} else {
			sprintf(senddata,"<message><to>%s</to><text>%s</text></message>",dest,texto);
			printf("envio: %s",senddata);
			printf("\n-------------------------------------------------------\n");
			if (sendto(sdCli,senddata,strlen(senddata),0,(struct sockaddr *)&addrInServ,sizeof(addrInServ)) < 0) exit ((perror("sendto"),-1));
			continue;
		}
/*********************************************************************************************/		
	}
	sprintf(senddata,"<message><to>GLOBAL</to><text>/kick %s</text></message>",nick);
	printf("envio: %s",senddata);
	printf("\n-------------------------------------------------------\n");
	if (sendto(sdCli,senddata,strlen(senddata),0,(struct sockaddr *)&addrInServ,sizeof(addrInServ)) < 0) exit ((perror("sendto"),-1));
	
	return 1;
}

