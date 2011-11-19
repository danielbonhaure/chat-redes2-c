
// FUNCIONES PARA MANEJAR MENSAJES

char* obtener_ip(char *nick){
	int i;
	for (i=0;i<MAX_CLI;i++){
		if (!listC[i].libre && !strcmp(nick,listC[i].nick)){
			return listC[i].ip;
		} 
	}
}

int obtener_port(char *nick){
	int i;
	for (i=0;i<MAX_CLI;i++){
		if (!listC[i].libre && !strcmp(nick,listC[i].nick)){
			return listC[i].port;
		} 
	}
}


void mens_priv(char *remitente,char *destino,char *mensaje){
	char respuesta[MAX_DATO];
	struct sockaddr_in addrInDest;
	bzero(&addrInDest,sizeof(addrInDest));
	addrInDest.sin_family      = AF_INET;
	addrInDest.sin_addr.s_addr = inet_addr(obtener_ip(destino));
	addrInDest.sin_port        = htons(obtener_port(destino));
	sprintf(respuesta,"<message><from>%s</from><type>PRIVATE</type><text>%s</text></message>",remitente,mensaje);
	if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInDest,sizeof(addrInDest)) < 0) exit ((perror("sendto"),-1));
}

void mens_global(char *remitente,char *mensaje){
	char respuesta[MAX_DATO];
	struct sockaddr_in addrInDest;
	int i;
	for (i=0;i<MAX_CLI;i++){
		if (!listC[i].libre){
			bzero(&addrInDest,sizeof(addrInDest));
			addrInDest.sin_family      = AF_INET;
			addrInDest.sin_addr.s_addr = inet_addr(listC[i].ip);
			addrInDest.sin_port        = htons(listC[i].port);
			sprintf(respuesta,"<message><from>%s</from><type>GLOBAL</type><text>%s</text></message>",remitente,mensaje);
			if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInDest,sizeof(addrInDest)) < 0) exit ((perror("sendto"),-1));
				
		}
	}
}

void informar_admin(){
	char *respuesta;
	struct sockaddr_in addrInDest;
	int i;
	
	sprintf(respuesta,"<chatroom><admin><nick>%s</nick></admin></chatroom>",listC[ADMIN].nick);

	for (i=0;i<MAX_CLI;i++){
		if (!listC[i].libre){
			bzero(&addrInDest,sizeof(addrInDest));
			addrInDest.sin_family      = AF_INET;
			addrInDest.sin_addr.s_addr = inet_addr(listC[i].ip);
			addrInDest.sin_port        = htons(listC[i].port);
			if (sendto(sd,respuesta,MAX_DATO,0,(struct sockaddr *)&addrInDest,sizeof(addrInDest)) < 0) exit ((perror("sendto"),-1));
				
		}
	}
	return ;
}
