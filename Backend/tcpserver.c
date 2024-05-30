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
#define  PUERTO    5001	     /* numero puerto arbitrario */
#define  PUERTOUDP    5000	     /* numero puerto arbitrario */
#define MAXLINE 1000 
#define HASHKEY 1

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


void createInstJson(char* ans, char* inst) {
    // Create a new cJSON object
    cJSON *json = cJSON_CreateObject();
    
    // Add the "inst" field with the value "ROOM_ALREADY_EXISTS"
    cJSON_AddStringToObject(json, "inst", inst);

    // Convert the cJSON object to a JSON string
    char *jsonString = cJSON_PrintUnformatted(json);
	strcpy(ans,jsonString);
    // Clean up the cJSON object
    cJSON_Delete(json);
}


void readInstruction(char *archivo, char *inst){
	
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
		strcpy(inst,instValue);
	}

    // Clean up
    cJSON_Delete(json);
	printf("todo salio \n");
	
}


void readCrendentialsJson(const char* jsonString, char* user) {
    // Parse the JSON string
    cJSON* json = cJSON_Parse(jsonString);
    if (!json) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    // Get the "user" and "password" fields from the JSON object
    cJSON* userJson = cJSON_GetObjectItem(json, "user");
    cJSON* passwordJson = cJSON_GetObjectItem(json, "password");

    if (cJSON_IsString(userJson) && cJSON_IsString(passwordJson)) {
        // Copy the values to the UserCredentials structure
        strcpy(user,strdup(userJson->valuestring));
    } else {
        fprintf(stderr, "Invalid JSON format\n");
        cJSON_Delete(json);
        return ;
    }

    // Clean up the cJSON object
    cJSON_Delete(json);
}


/*
	Nota Isaac, es muy importante que modifiques la IP y el puerto para los del servidor UDP
*/

void udp(char *message, char* ans)
{    
    char buffer[1000]; 
    char ip[16]="172.18.2.4";
    int sockfd; 
    int port=PUERTOUDP;
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
    int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr*)&servaddr, &len);
    if(n < 0) {
        perror("recvfrom failed");
        exit(EXIT_FAILURE);
        createInstJson(ans,"LOST_CONNECTION_WITH_SERVER");
    }else{
        buffer[n] = '\0'; 
        strcpy(ans,buffer);
    }
    // close the descriptor 
    close(sockfd); 
} 

void descifrar( char* mensaje){
    while (*mensaje != '\0') {
        *mensaje = *mensaje - HASHKEY;
        mensaje++;
    }
}
void cifrar( char* mensaje){
    while (*mensaje != '\0') {
        *mensaje = *mensaje + HASHKEY;
        mensaje++;
    }
}


int main(){
	char  msg[msgSIZE];	     /* parametro entrada y salida */
	char  json[jsonSIZE];	     /* parametro entrada y salida */
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
	int seguirperman=1;
	while(seguirperman){
		
		if ((sd_actual = accept(sd, (struct sockaddr *)&pin, &addrlen)) == -1) {
			perror("accept");
			exit(1);
		}
		printf("Conexiones establecidas\n");
		pid_t child_pid =fork();
		printf("Conexion numero: %d\n", child_pid);
		if(child_pid==0){
			printf("sd_Actual: %d\n",(int)sd_actual );
			char sigue='S';
			char msgReceived[1000];
			char inst[100];
			char msgCnt[1000];
			char loggedUser[1000];
			strcpy(json,"");
			
			while(sigue=='S'){			
				/* tomar un mensaje del cliente */
				int n;		
				n = recv(sd_actual, msg, sizeof(msg), 0);
				if (n == -1) {
					perror("recv");
					exit(1);
				}		
				msg[n] = '\0';
				descifrar(msg);		
				strcpy(json,msg);
				readInstruction(json, inst);
				printf("Instruccion recibida: %s\n", inst);
				if((strcmp(inst,"close")==0)){ //it means that the conversation must be closed
					sigue='N';
					createInstJson(json,"close");
				}else{
					if(strcmp(inst,"LOGIN")==0){
						printf("Enviando una request de %s\n",inst);
						char res[1000];
						udp(json,res);
						strcpy(json,res);
						readInstruction(json, inst);
						if(strcmp(inst,"LOGIN_SUCCESSFULL")==0){
							readCrendentialsJson(json, loggedUser);
							printf("Login en la conexion actual: %s\n",loggedUser);
						}
						
						printf("JSON generado: %s\n", json);
					} else{	
						printf("Enviando una request de %s\n",inst);
						char res[1000];
						udp(json,res);
						strcpy(json,res);
						printf("JSON generado: %s\n", json);
					}
				}		
				/* enviando la respuesta del servicio */
				int sent;
				printf("mensaje previo a cifrar: %s\n", json);
				cifrar(json);
				printf("mensaje enviado a: %s\n", json);
				if ( (sent = send(sd_actual, json, strlen(json), 0)) == -1) {
					perror("send");
					exit(1);
				}
			}

				/* cerrar los dos sockets */
				close(sd_actual);  
				close(sd);
				printf("Conexion cerrada\n");
				seguirperman=0;
				break;
		} else{
			printf("Puerto Cerrado\n");
			close(sd_actual);  
		}
	}
				close(sd);
	return 0;
}
