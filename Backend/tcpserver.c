/*
   Codigo del servidor

   Nombre Archivo: tcpserver.c   

   Compilacion: cc tcpserver.c cJSON.c -lnsl -o tcpserver
   Ejecución: ./tcpserver
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#include <sys/types.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 

/* The following headers was required in old or some compilers*/
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>	// it is required to call signal handler functions
#include <unistd.h>  // it is required to close the socket descriptor


#include "cJSON.h"

#define  jsonSIZE  10000
#define  msgSIZE   2048      /* longitud maxima parametro entrada/salida */
#define  PUERTO    5003	     /* numero puerto arbitrario */
#define MAXLINE 1000 

int                  sd, sd_actual, sd_actual2;  /* descriptores de sockets */
int                  addrlen;        /* longitud msgecciones */
struct sockaddr_in   sind, pin;      /* msgecciones sockets cliente u servidor */


/*  procedimiento de aborte del servidor, si llega una senal SIGINT */
/* ( <ctrl> <c> ) se cierra el socket y se aborta el programa       */
void aborta_handler(int sig){
   printf("....abortando el proceso servidor %d\n",sig);
   close(sd);  
   close(sd_actual); 
   exit(1);
}


void nJsonFile(char *json,char* inst, int id, int fila, int col){
	char ejemplo[1000]="";
	char nstr[10];
	strcpy(ejemplo,"{\"inst\":\"");
	strcat(ejemplo,inst);
	strcat(ejemplo,"\",\"id\": ");
	sprintf(nstr, "%d", id);
	strcat(ejemplo,nstr);
	strcat(ejemplo," ,\"fila\": ");
	sprintf(nstr, "%d", fila);
	strcat(ejemplo,nstr);
	strcat(ejemplo,",\"columna\": ");
	sprintf(nstr, "%d", col);
	strcat(ejemplo,nstr);
	strcat(ejemplo,"}");
	strcpy(json,ejemplo);
}


void nJsonTurno(char *json,int id){
	char ejemplo[1000]="";
	char nstr[10];
	strcpy(ejemplo,"{\"inst\":\"");
	strcat(ejemplo,"TURNO");
	strcat(ejemplo,"\",\"id\": ");
	sprintf(nstr, "%d", id);
	strcat(ejemplo,nstr);
	strcat(ejemplo,"}");
	strcpy(json,ejemplo);
}

int isNumberl(const char *str) {
    if (!str || *str == '\0') // Null or empty string
        return 0;

    char *endptr;
    errno = 0; // To distinguish success/failure after call
    strtol(str, &endptr, 10);

    // Check for various possible errors
    if ((errno == ERANGE && (strtol(str, NULL, 10) == LONG_MAX || strtol(str, NULL, 10) == LONG_MIN)) || (errno != 0 && strtol(str, NULL, 10) == 0)) {
        return 0; // Not a valid number
    }
    // If there are any non-numeric characters in the string, it's not a number
    while (*endptr != '\0') {
        if (!isspace((unsigned char)*endptr) && !isdigit((unsigned char)*endptr)) {
            	return 0; // Not a valid number
        }
        endptr++;
    }
    return 1; // String can be converted to a number
}

void readJsonFile(char *archivo, char *inst, int *id, int *fila, int *col){
	/*
	char izq[100], der[100];
	strcpy(izq,"");
	strcpy(der,"");
	int it=0,ap=0, lookl=0;
	*id=0;
	*fila=0;
	*col=0;
	while(archivo[it]!='\0'){
		if(archivo[it]=='\"'){
			if(ap==0){
				ap=1;
				if(lookl==0){
					strcpy(izq,"");
				} else{
					strcpy(der,"");
				}
			} else{
				ap=0;
			}
		}
		if(ap==1){
			
				if(lookl==0){
					strcat(izq,&archivo[it]);
				} else{
					strcat(der,&archivo[it]);
				}
		}else{
			if(archivo[it]==':'){
				lookl=1;
			}else{
				if(archivo[it]!=','){
					if(strcmp(izq,"inst")==0)strcpy(inst,der);
					if(strcmp(izq,"id")==0&&isNumber(der))*id=atoi(der);
					if(strcmp(izq,"fila")==0&&isNumber(der))*fila=atoi(der);
					if(strcmp(izq,"columna")==0&&isNumber(der))*col=atoi(der);
					lookl=0;
					strcpy(der,"");
					strcpy(izq,"");
				}
			}
		}
		it++;
	}*/
	cJSON* json = cJSON_Parse(archivo);
	printf("Archivo Parseado\n");
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error parsing JSON: %s\n", error_ptr);
        }
        return;
    }

    // Get the value of the "inst" parameter
    cJSON* instJ = cJSON_GetObjectItemCaseSensitive(json, "inst");
    if (instJ == NULL) {
        fprintf(stderr, "JSON does not contain 'inst' parameter\n");
        return ;
    } else{
	
	    // Extract the value as a string
	    char* instValue = strdup(instJ->valuestring);
	    
	    char* idValue;
	    
	    char* filaValue;
	    
	    char* columnaValue;
	    
	    cJSON *id_obj;
	    id_obj = cJSON_GetObjectItemCaseSensitive(json, "id");
	    if (id_obj == NULL ) {
	        printf("Error: 'id' not found or not a number.\n");
	    }else{
			idValue=strdup(id_obj->valuestring);
	    	if(isNumberl(idValue)==1)
	    		*id = atoi(idValue);
		}
	    
	    cJSON *fila_obj;
	    fila_obj = cJSON_GetObjectItemCaseSensitive(json, "fila");
	    if (fila_obj == NULL ) {
	        printf("Error: 'fila' not found or not a number.\n");
	    }else{
			filaValue=strdup(fila_obj->valuestring);
	    	if(isNumberl(filaValue)==1)
	    		*fila = atoi(filaValue);
		}
	    
	    cJSON *columna_obj;
	    columna_obj = cJSON_GetObjectItemCaseSensitive(json, "columna");
	    if (columna_obj == NULL ) {
	        printf("Error: 'columna' not found or not a number.\n");
	    }else{
			columnaValue=strdup(columna_obj->valuestring);
	    	if(isNumberl(columnaValue)==1)
	    	*col = atoi(columnaValue);
		}
		
		
		strcpy(inst,instValue);
	}

    // Clean up
    cJSON_Delete(json);
	printf("todo salio bien");
	
}

  	
int won(int tablero[7][7]){
	for( int i=0; i<7; i++){
		for( int k=0; k<7; k++){
			printf("%d ", tablero[i][k]);
		}
			printf("\n");
	}
	for( int i=0; i<7; i++){
		for( int k=0; k<4; k++){
			if(tablero[i][k]!=0)
			if(tablero[i][k]==tablero[i][k+1]&&tablero[i][k]==tablero[i][k+2]&&tablero[i][k]==tablero[i][k+3]&&tablero[i][k]!=0)return 1;
		}
	}
	printf("No hay fila iguales\n");
	for( int i=0; i<7; i++){
		for( int k=0; k+3<7; k++){
			if(tablero[k][i]!=0)
			if(tablero[k][i]==tablero[k+1][i]&&tablero[k][i]==tablero[k+2][i]&&tablero[k][i]==tablero[k+3][i]&&tablero[k][i]!=0)return 1;
		}
	}
	printf("No hay columnas\n");

	for( int i=0; i+3<7; i++){
		for( int k=0; k+3<7; k++){
			if(tablero[i][k]!=0)
			if(tablero[i][k]==tablero[i+1][k+1]&&tablero[i][k]==tablero[i+2][k+2]&&tablero[i][k]==tablero[i+3][k+3])return 1;
		}
	}
		
	for( int i=6; i-3>=0; i--){
		for( int k=0; k+3<7; k++){
			if(tablero[i][k]!=0)
			if(tablero[i][k]==tablero[i-1][k+1]&&tablero[i][k]==tablero[i-2][k+2]&&tablero[i][k]==tablero[i-3][k+3])return 1;
		}
	}
	for( int i=6; i-3>=0; i--){
		for( int k=6; k-3>=0; k--){
			if(tablero[i][k]!=0)
			if(tablero[i][k]==tablero[i-1][k-1]&&tablero[i][k]==tablero[i-2][k-2]&&tablero[i][k]==tablero[i-3][k-3])return 1;
		}
	}
	for( int i=0; i+3<7; i++){
		for( int k=6; k-3>=0; k--){
			if(tablero[i][k]!=0)
			if(tablero[i][k]==tablero[i+1][k-1]&&tablero[i][k]==tablero[i+2][k-2]&&tablero[i][k]==tablero[i+3][k-3])return 1;
		}
	}
	printf("No hay diagonal2 \n");
	return 0;
}

/*
	Nota Isaac, es muy importante que modifiques la IP y el puerto para los del servidor UDP
*/

int udp(char *message, int intencion)
{    
    char buffer[1000]; 
    char ip[16]="10.7.52.136";
    int sockfd; 
    int port=5001;
    struct sockaddr_in servaddr; 
      
    // clear servaddr 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_addr.s_addr = inet_addr(ip); 
    servaddr.sin_port = htons(port); 
    servaddr.sin_family = AF_INET; 
      
    // create datagram socket 
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
    if(sockfd==-1){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //it is not required to establish a connection
    //directly sending a message
    printf("Mensaje enviado\n");
	int r = sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
    if(r==-1){
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }
      
    // waiting for response 
    int len = sizeof(servaddr);
    int ans=-1;
    int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr*)&servaddr, &len);
    if(n < 0) {
        perror("recvfrom failed");
        exit(EXIT_FAILURE);
        if(intencion==0)return 2;
        if(intencion==1)return 4;
    }else{
        buffer[n] = '\0'; 
        printf("\nHe recibido del server: ");
        printf("%s\n",buffer);
        ans= atoi(buffer);
    }
    // close the descriptor 
    close(sockfd); 
    return ans;
} 



void descifrar( char* mensaje){
    while (*str != '\0') {
        *str = *str - 1;
        str++;
    }
}
void cifrar( char* mensaje){
    while (*str != '\0') {
        *str = *str + 1;
        str++;
    }
}


int main(){
	char  msg[msgSIZE];	     /* parametro entrada y salida */
	char  json[jsonSIZE];	     /* parametro entrada y salida */
	int tablero[7][7];
	/*
	When the user presses <Ctrl + C>, the aborta_handler function will be called, 
	and such a message will be printed. 
	Note that the signal function returns SIG_ERR if it is unable to set the 
	signal handler, executing line 54.
	*/	
   if(signal(SIGINT, aborta_handler) == SIG_ERR){
   	perror("Could not set signal handler");
      return 1;
   }
       //signal(SIGINT, aborta);      /* activando la senal SIGINT */

/* obtencion de un socket tipo internet */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

/* asignar msgecciones en la estructura de msgecciones */
	sind.sin_family = AF_INET;
	sind.sin_addr.s_addr = INADDR_ANY;   /* INADDR_ANY=0x000000 = yo mismo */
	sind.sin_port = htons(PUERTO);       /*  convirtiendo a formato red */

/* asociando el socket al numero de puerto */
	if (bind(sd, (struct sockaddr *)&sind, sizeof(sind)) == -1) {
		perror("bind");
		exit(1);
	}

/* ponerse a escuchar a traves del socket */
	if (listen(sd, 5) == -1) {
		perror("listen");
		exit(1);
	}
	
	printf("Esperando Conexiones\n"); 
	/* esperando que un cliente solicite un servicio */
	while(true){
		
		if ((sd_actual = accept(sd, (struct sockaddr *)&pin, &addrlen)) == -1) {
			perror("accept");
			exit(1);
		}
		printf("Conexion 1 establecida\n");
		if ((sd_actual2 = accept(sd, (struct sockaddr *)&pin, &addrlen)) == -1) {
			perror("accept");
			exit(1);
		}
		printf("Conexiones establecidas\n");
		pid_t child_pid;
		child_pid=fork();
		
		if(child_pid!=0){
			for( int i=0; i<7;i++){
				for( int j=0; j<7; j++){
					tablero[i][j]=0;
				}
			}
			printf("sd_Actual: %d\n",(int)sd_actual );
			char sigue='S';
			char msgReceived[1000];
			char inst[100];
			int turno=0;
			int notsendanything=0;
			int idc, fil, col;
			strcpy(json,"");
			int logina=0, loginb=0;
			while(sigue=='S'){	
				idc=0;
				fil=0; 
				col=0;			
				/* tomar un mensaje del cliente */
				int n;
				if(turno ==0){			
					n = recv(sd_actual, msg, sizeof(msg), 0);
					if(logina==1)turno=1;
				}
				else{			
					n = recv(sd_actual2, msg, sizeof(msg), 0);
					if(loginb==1)turno=0;
				}
				if (n == -1) {
					perror("recv");
					exit(1);
				}		
				msg[n] = '\0';
				descifrar(msg);		
				printf("Client %d sent: %s\n", 1-turno, msg);
				strcpy(json,msg);
				readJsonFile(json, inst, &idc, &fil, &col);
				printf("Instruccion recibida: %s\n", inst);
				if((strcmp(inst,"close")==0)){ //it means that the conversation must be closed
					sigue='N';
					nJsonFile(json,inst,idc,fil,col);
				}else{
					if(strcmp(inst,"LOGIN")==0||strcmp(inst,"REGISTER")==0){
						printf("Enviando una request de %s\n",inst);
						int res;
						if(strcmp(inst,"LOGIN")==0)
							res=udp(json,0);
						else
							res=udp(json,1);
						if(res==1){
							if(logina==1){
								loginb=1;
								nJsonFile(json,"LOGIN_SUCCESSFULL",2,0,0);
								if (send(sd_actual2, json, strlen(json), 0) == -1) {
									perror("send");
									exit(1);
								}
								notsendanything=1;
								turno=0;
							}
							else{
								logina=1;
								nJsonFile(json,"LOGIN_SUCCESSFULL",1,0,0);
								if (send(sd_actual, json, strlen(json), 0) == -1) {
									perror("send");
									exit(1);
								}
								notsendanything=1;
								turno=1;
							}
						} else{
							if(res==2)
								nJsonFile(json,"USER_NOT_FOUND",0,0,0);
							if(res==3)
								nJsonFile(json,"REGISTER_SUCCESSFULL",0,0,0);
							if(res==4)
								nJsonFile(json,"USER_ALREADY_EXISTS",0,0,0);
						}
						printf("Codigo de respuesta recibido %d\n", res);
					} else{		
						if(strcmp(inst,"ADD_FICHA")==0){
							int nfil=6;
							while(nfil>0&&tablero[col][nfil]!=0)
								nfil--;
							fil=nfil;
							printf("Nueva fila: %d\n",nfil);
							tablero[col][fil]=idc;
							if(won(tablero)==1)
								nJsonFile(json,"GANA",idc,fil,col);
							else
								nJsonFile(json,"ADD_FICHA",idc,fil,col);
						}
					}
				}		
				/* enviando la respuesta del servicio */
				int sent;
				if(notsendanything==0){
					cifrar(json);
					if(turno ==0){
						if ( (sent = send(sd_actual, json, strlen(json), 0)) == -1) {
							perror("send");
							exit(1);
						}
						printf("mensaje enviado a: %s\n", json);
						if(strcmp(inst,"ADD_FICHA")==0||strcmp(inst,"GANA")==0){
								
							if ( (sent = send(sd_actual2, json, strlen(json), 0)) == -1) {
								perror("send");
								exit(1);
							}
							if(strcmp(inst,"ADD_FICHA")==0){	
								int temn = recv(sd_actual2, msg, sizeof(msg), 0);
								temn = recv(sd_actual, msg, sizeof(msg), 0);
							}
						printf("mensaje enviado b: %s\n", json);
						}
					} else{
						if ( (sent = send(sd_actual2, json, strlen(json), 0)) == -1) {
							perror("send");
							exit(1);
						}
						printf("mensaje enviado a: %s\n", json);
						
						if(strcmp(inst,"ADD_FICHA")==0||strcmp(inst,"GANA")==0){
								
							if ( (sent = send(sd_actual, json, strlen(json), 0)) == -1) {
								perror("send");
								exit(1);
							}	
							if(strcmp(inst,"ADD_FICHA")==0){		
								int temn = recv(sd_actual2, msg, sizeof(msg), 0);
								temn = recv(sd_actual, msg, sizeof(msg), 0);
							}
							printf("mensaje enviado b: %s\n", json);
						}
					}
				}
				notsendanything=0;
			}

				/* cerrar los dos sockets */
					close(sd_actual);  
				close(sd);
				printf("Conexion cerrada\n");
				break;
		}
	}
	return 0;
}
