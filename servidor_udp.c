/*		PARA COMPILAR		 */
/* gcc -o servidor `xml2-config --cflags` servidor_udp.c `xml2-config --libs` */
/* sudo apt-get install libxml2 libxml2-dev */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <libxml/xpath.h>

#define MAX_CLI	  30
#define MAX_DATO  1000
#define ADMIN 	  0

typedef enum {true=1, false=0} bool;

#include "struct_clientes.h"

bool exist_admin;

//SOCKET SERVIDOR
int sd;	

#include "mensajes.h"	


//FUNCIONES
void error (char *s) { exit ((perror(s),-1)); }		//captura errores, cierra el programa e informa del error	
void procesar_mens(const char*,struct sockaddr_in);	//procesa los mensajes recibidos
bool conectar(char *,int,char *);			//guardo el nick y simulo una coneccion, solo así acepto mensajes de ese cliente
bool desconectar(char *,int,char *);			//elimino al cliente con ese nick de la lista


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

	sd = socket(AF_INET, SOCK_DGRAM, 0);
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
		procesar_mens(mesg,addrInCli);	
	}

	return 0;
}

void procesar_mens(const char *mensaje,struct sockaddr_in addrInCli){
	
	int i;
	char *ip  =  inet_ntoa(addrInCli.sin_addr);
	int port =  ntohs(addrInCli.sin_port);
	char *tmp;
	
	printf("recibido:");
	printf("%s\n",mensaje);
	
	char respuesta[MAX_DATO];
/****************************************************************************/
	xmlDocPtr doc; 

	doc = xmlParseMemory(mensaje,strlen(mensaje));
	if (doc == NULL) exit ((perror("parser\n"),-1));

	xmlNodePtr root;
	xmlNodePtr node;

	root = xmlDocGetRootElement(doc);
	
	if (!strcmp(root->name,"connect")){
		node = root->xmlChildrenNode;;
		if (!strcmp(node->name,"nick")){
			char *nick;
			nick = xmlNodeGetContent(node);
			if(!exist_admin){
				listC[ADMIN].ip    =  ip;
				listC[ADMIN].port  =  port;
				listC[ADMIN].nick  =  nick;
				listC[ADMIN].libre =  false;
				exist_admin = true;
				struct_cliente_imprimir();
				sprintf(respuesta,"<connect><action>ACCEPTED</action></connect>");
				if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));
			} else if (nick_valido(nick) && new_client(ip,port)){
				if (!conectar(ip,port,nick)) exit ((perror("conectar"),-1));
				sprintf(respuesta,"<connect><action>ACCEPTED</action></connect>");
				if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));
			} else {
				sprintf(respuesta,"<connect><action>REJECTED</action></connect>");
				if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));	
			}		
		}
	} else if (!strcmp(root->name,"chatroom")){
		if (!new_client(ip,port)){
			#ifdef DEBUG
				printf("estoy en chatroom\n");
			#endif
			node = root->xmlChildrenNode;
			if (!strcmp(node->name,"request")){
				char *req;
				req = xmlNodeGetContent(node);
				if (!strcmp(req,"ADMIN")){
					sprintf(respuesta,"<chatroom><admin><nick>%s</nick></admin></chatroom>",listC[ADMIN].nick);
					if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));
				} else if (!strcmp(req,"USERS")){
					sprintf(respuesta,"<chatroom><users>");
					for (i=0;i<MAX_CLI;i++){
						if (!listC[i].libre){	
							sprintf(tmp,"<nick>%s</nick>",listC[i].nick);	
							#ifdef DEBUG
								printf("%s\n",tmp);
							#endif						
							strcat(respuesta,tmp);
						}
					}
					strcat(respuesta,"</users></chatroom>");
					if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));
				}					
			}
		}//termina el if que revisa si el remitente esta logueado
	} else if (!strcmp(root->name,"message")){
		if (!new_client(ip,port)){
			#ifdef DEBUG
				printf("estoy en mensaje\n");
			#endif
			xmlXPathContextPtr xpathCtx; 
		    	xmlXPathObjectPtr to, text; 

			/* Create xpath evaluation context */
			xpathCtx = xmlXPathNewContext(doc);
			if(xpathCtx == NULL) exit ((perror("linea 158"),-1));

			/* Evaluate xpath expression */
			to = xmlXPathEvalExpression(BAD_CAST "//to", xpathCtx);
			if(to == NULL) exit ((perror("linea 162"),-1));
			text = xmlXPathEvalExpression(BAD_CAST "//text", xpathCtx);
			if(text == NULL) exit ((perror("linea 164"),-1));

			char *remitente,*destino,*texto;
			remitente = nick_obtener(ip,port);
			destino = xmlNodeGetContent(to->nodesetval->nodeTab[0]);
			texto = xmlNodeGetContent(text->nodesetval->nodeTab[0]);
			#ifdef DEBUG
				printf("r:%s,d:%s,t:%s\n",remitente,destino,texto);
			#endif
			if (!strcmp("GLOBAL",destino)){
				#ifdef DEBUG
					printf("estoy en GLOBAL\n");
				#endif
				if (!strcmp(listC[ADMIN].nick,remitente)){
					#ifdef DEBUG
						printf("estoy en mensaje de admin\n");
					#endif
					if (!strncmp(texto,"/kick",5)) {
						char *ptr, user[20];
						ptr = strtok(texto," \t");    // Primera llamada => Primer token
						while( (ptr = strtok( NULL," \t")) != NULL )    // Posteriores llamadas
							sprintf(user,"%s",ptr);
						if (!desconectar(obtener_ip(user),obtener_port(user),user)) exit ((perror("desconectar"),-1));
					} else {
						mens_global(remitente,texto);
					}
				} else {
					if (!strncmp(texto,"/kick",5)) {
					#ifdef DEBUG
						printf("estoy en mensaje de no admin\n");
					#endif
						char *ptr, user[20];
						ptr = strtok(texto," \t");    // Primera llamada => Primer token
						while( (ptr = strtok( NULL," \t")) != NULL )    // Posteriores llamadas
							sprintf(user,"%s",ptr);
						if (!strcmp(user,remitente)){
							if (!desconectar(obtener_ip(user),obtener_port(user),user)) exit ((perror("desconectar"),-1));
						} else {
							sprintf(respuesta,"<message><from>SERVIDOR</from><type>PRIVATE</type><text>Usted no es admin. Solo puede hacerse /kick a usted mismo.</text></users></message>");
							if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));							
						}
					} else {
						mens_global(remitente,texto);
					}
				}
			} else {
				if (!new_client(ip,port)){
					#ifdef DEBUG
						printf("estoy en PRIVATE\n");
					#endif
					if (nick_existe(destino)){
						mens_priv(remitente,destino,texto);
						#ifdef DEBUG
							printf("volvi de mens_priv\n");
						#endif
					} else {
						sprintf(respuesta,"<message><from>SERVIDOR</from><type>PRIVATE</type><text>Destino incorrecto</text></users></message>");
						if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));	
					}
				}
			}
		}//termina el if que revisa si el remitente esta logueado		
	} else {
		#ifdef DEBUG
			printf("estoy en niguno o sea default, el ultimo else\n");
		#endif
		sprintf(respuesta,"<message><from>SERVIDOR</from><type>PRIVATE</type><text>Mensaje estructurado incorrectamente</text></users></message>");
		if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInCli,sizeof(addrInCli)) < 0) exit ((perror("sendto"),-1));	
	}
	printf("-------------------------------------------------------\n");
/****************************************************************************/
}



bool conectar(char *ip,int port,char *nick){
	int i;
	for (i=0;i<MAX_CLI;i++){
		if (listC[i].libre){
			listC[i].ip    =  ip;
			listC[i].port  =  port;
			listC[i].nick  =  nick;
			listC[i].libre =  false;
			struct_cliente_imprimir();
			return true;
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
						informar_admin();
					}
				}
			} else {
				listC[i].ip    =  "vacio";
				listC[i].port  =  0;
				listC[i].nick  =  "vacio";
				listC[i].libre =  true;
			}
			struct_cliente_imprimir();
			return true;
		}
	}
	return false;
}



