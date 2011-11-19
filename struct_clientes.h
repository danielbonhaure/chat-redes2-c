
// ESTRUCTURA PARA MANTENER LOS CLIENTES

struct cliente {
	char *ip;
	int port;
	char *nick;
	bool libre;
}listC[MAX_CLI];


void inicializar(){
	int i;
	for(i=0;i<MAX_CLI;i++){
		listC[i].libre = true;
	}
}

bool nick_valido(char *nick){
	int i;
	for (i=0;i<MAX_CLI;i++){
		if (!listC[i].libre){
			if (!strcmp(nick,listC[i].nick)){
				return false;
			}
		}
	}
	return true;
}

bool nick_existe(char *nick){
	int i;
	for (i=0;i<MAX_CLI;i++){
		if (!listC[i].libre){
			if (!strcmp(nick,listC[i].nick)){
				return true;
			}
		}
	}
	return false;
}

bool new_client(char *ip,int port){
	int i;
	for (i=0;i<MAX_CLI;i++){
		if ((!listC[i].libre) && (!strcmp(ip,listC[i].ip)) && (port==listC[i].port)){
			return false;
		}
	}
	return true;
}

char* nick_obtener(char *ip,int port){
	/* NO DEBE RECIBIR UN DESTINO QUE NO EXISTE*/
	int i;
	for (i=0;i<MAX_CLI;i++){
		if ((!listC[i].libre) && (!strcmp(ip,listC[i].ip)) && (port==listC[i].port)){
			return listC[i].nick;
		}
	}
}

void struct_cliente_imprimir(){
	int i;
	for (i=0;i<MAX_CLI;i++){
		if (!listC[i].libre) printf("%d=%s,%d,%s,%d\n",i,listC[i].ip,listC[i].port,listC[i].nick,listC[i].libre);
	}
}
