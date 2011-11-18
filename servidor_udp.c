/*		PARA COMPILAR		 */
/* gcc -o servidor `xml2-config --cflags` servidor_udp.c `xml2-config --libs` */
/* sudo apt-get install libxml2 libxml2-dev */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <libxml/xpath.h>



#define MAX_CLI	  30
#define MAX_DATO  1000
#define ADMIN 	  0

void error (char *s) { exit ((perror(s),-1)); }
void procesar_mens(const char*,struct sockaddr_in,int);

typedef enum {true=1, false=0} bool;

struct cliente {
	char *ip;
	int port;
	char *nick;
	bool libre;
} listC[MAX_CLI];

bool exist_admin;

void inicializar(){
	int i;
	for(i=0;i<MAX_CLI;i++){
		listC[i].libre = true;
	}
}

bool nick_valido(char *);		//verifico si el nick esta disponible
bool conectar(char *,int,char *);	//guardo el nick y simulo una coneccion, solo así acepto mensajes de ese cliente
bool desconectar(char *,int,char *);	//elimino al cliente con ese nick de la lista

int main(int argc, char *argv[]) {

	// INICIALIZAR POS
	exist_admin = false;
	inicializar();
	// ARGUMENTS
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
		procesar_mens(mesg,addrInCli,sd);	
	}

	return 0;
}

void procesar_mens(const char *mensaje,struct sockaddr_in addrInCli,int sd){

	char *ip   =  inet_ntoa(addrInCli.sin_addr);
	char port =  ntohs(addrInCli.sin_port);
	printf("recibido:");
	printf("%s",mensaje);
	
	char *respuesta;
/****************************************************************************/
	xmlDocPtr doc; 

	doc = xmlParseMemory(mensaje,strlen(mensaje));
	if (doc == NULL) exit ((perror("error al parsear"),-1));

	xmlNodePtr root;
	
	root = xmlDocGetRootElement(doc);
	
	if (!strcmp(root->name,"connect")){
		xmlNodePtr node = root->xmlChildrenNode;;
		if (!strcmp(node->name,"nick")){
			char *nick;
			nick = xmlNodeGetContent(node);
			if(!exist_admin){
				listC[ADMIN].ip    =  ip;
				listC[ADMIN].port  =  port;
				listC[ADMIN].nick  =  nick;
				listC[ADMIN].libre =  false;
				exist_admin = true;
				respuesta = "<connect><action>ACCEPTED</action></connect>";
			} else if (nick_valido(nick)){
				if (conectar(ip,port,nick)) exit ((perror("error al guardar nick valido"),-1));
				respuesta = "<connect><action>ACCEPTED</action></connect>";
			} else {
				respuesta = "<connect><action>REJECTED</action></connect>";				
			}		
		}
	}
	printf("-------------------------------------------------------\n");
/****************************************************************************/
	if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));
}


bool nick_valido(char *nick){
	int i;
	for (i=0;i<MAX_CLI;i++){
		if (!listC[i].libre){
			if (!strcmp(nick,listC[i].nick)){
				return false;
			}
		} else {
			continue;
		}
	}
	return true;
}

bool conectar(char *ip,int port,char *nick){
	int i;
	for (i=0;i<MAX_CLI;i++){
		if (listC[i].libre){
			listC[i].ip    =  ip;
			listC[i].port  =  port;
			listC[i].nick  =  nick;
			listC[i].libre =  false;
			return true;
		} else {
			continue;
		}
	}
	return false;
}

bool desconectar(char *ip,int port,char *nick){
	int i,j;
	for (i=0;i<MAX_CLI;i++){
		if ((!listC[i].libre) && (!strcmp(ip,listC[i].ip)) && (!strcmp(nick,listC[i].nick)) && (port==listC[i].port)){
			if (i==ADMIN){
				for (j=0;j<MAX_CLI;j++){
					if(!listC[j].libre){
						listC[i].ip    =  listC[ADMIN].ip;
						listC[i].port  =  listC[ADMIN].port;
						listC[i].nick  =  listC[ADMIN].nick;
						listC[i].libre =  false;
						listC[j].ip    =  "vacio";
						listC[j].port  =  0;
						listC[j].nick  =  "vacio";
						listC[j].libre =  true;
					}
				}
			} else {
				listC[i].ip    =  "vacio";
				listC[i].port  =  0;
				listC[i].nick  =  "vacio";
				listC[i].libre =  true;
			}
			return true;
		} else {
			continue;
		}
	}
	return false;
}


