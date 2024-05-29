/*
  Cliente del programa numero 1, de la materia CÃ³mputo Distribuido
  

   Lectura remota de un directorio usando sockets pertenecientes
   a la familia TCP, en modo conexion.
   Codigo del cliente.

   Nombre Archivo: tcpclient.c
   Archivos relacionados: tcpserver.c num_vocales.h
   Fecha: Febrero 2023

   Compilacion: cc tcpclient.c cJSON.c -lnsl -o tcpclient

   Ejecucion: ./tcpclient 172.18.2.2    
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
#define HASHKEY 1

int                  sd, sd_actual;  /* descriptores de sockets */
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

void nJsonFile(char *json,char* inst, int id, int fila, char* msg){
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
	strcat(ejemplo,",\"msg\": ");
	strcat(ejemplo,msg);
	strcat(ejemplo,"}");
	strcpy(json,ejemplo);
}
void nJsonFile2(char *json,char* inst,char* usr,char* email,char* pass){
	char ejemplo[1000]="";
	char nstr[10];
	strcpy(ejemplo,"{\"inst\":\"");
	strcat(ejemplo,inst);
	strcat(ejemplo,"\",\"user\": \"");
	strcat(ejemplo,usr);
	strcat(ejemplo,"\" ,\"email\": \"");
	strcat(ejemplo,email);
	strcat(ejemplo,"\",\"password\": \"");
	strcat(ejemplo,pass);
	strcat(ejemplo,"\"}");
	strcpy(json,ejemplo);
}


int isNumberl(const char *str) {
	printf("Checking if %s is a number\n", str);
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

void readJsonFile(char *archivo, char *inst, int *id, int *fila, char *col){
	
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
		
	    char* idValue;
	    
	    char* filaValue;
	    
	    char* columnaValue;
	    
	    cJSON *id_obj;
	    id_obj = cJSON_GetObjectItemCaseSensitive(json, "id");
	    if (id_obj == NULL ) {
	        printf("Error: 'id' not found or not a number.\n");
	    }else{
	    	printf("Instruccion obtenida correctamente\n");
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
    	columna_obj = cJSON_GetObjectItemCaseSensitive(json, "msg");
	    if (columna_obj == NULL ) {
	        printf("Error: 'columna' not found or not a number.\n");
	    }else{
		    char* colValue = strdup(instJ->valuestring);
			strcpy(col,colValue);
		}
		
	}

    // Clean up
    cJSON_Delete(json);
	printf("todo salio bien\n");
	
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

int main(argc, argv)
   int    argc;  
   char  *argv[];
{
	char                dir[msgSIZE];  /* parametro entrada y salida */
	int                 sd;		   /* descriptores de sockets    */
	struct hostent 	   *hp;		   /* estructura del host        */
	struct sockaddr_in sin, pin; 	   /* direcciones socket        */
    int                *status;        /* regreso llamada sistema */
    char               *host;          /* nombre del host */
	char  msg[msgSIZE];	     /* parametro entrada y salida */
	char  json[jsonSIZE];	     /* parametro entrada y salida */

/* verificando el paso de parametros */

        if ( argc != 2) {
           fprintf(stderr,"Error uso: %s <host> \n",argv[0]);
           exit(1);
        } 
        host = argv[1];

/* encontrando todo lo referente acerca de la maquina host */

	if ( (hp = gethostbyname(host)) == 0) {
		perror("gethosbyname");
		exit(1);
	}
		
/* llenar la estructura de direcciones con la informacion del host */
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
	pin.sin_port = htons(PUERTO);                    

/* obtencion de un socket tipo internet */
	if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

/* conectandose al PUERTO en el HOST  */
	if ( connect(sd, (struct sockaddr *)&pin, sizeof(pin)) == -1) {
		perror("connect");
		exit(1);
	}
	
	char sigue='S';
	char login='N';
	strcpy(json," ");
	while(sigue=='S'){
		if(login=='S'){
			char expresion[100];
			scanf("%s",expresion);
		/* enviar mensaje al PUERTO del  servidor en la maquina HOST */
		    strcpy(dir,expresion);
			if ( send(sd, dir, sizeof(dir), 0) == -1 ) {
				perror("send");
				exit(1);
			}
		
		/* esperar por la respuesta */
			if ( recv(sd, dir, sizeof(dir), 0) == -1 ) {
				perror("recv");
				exit(1);
			}
		
		/* imprimimos el resultado y cerramos la conexion del socket */
			printf("%s", dir);
		} else{
			char inst[100];
			char ans[100];
			
			printf("Quiere registarr un usuario?(Y/N)\n");
			scanf("%s",ans);
			if(strcmp(ans,"Y")==0)
				strcpy(inst,"REGISTER");
			else
				strcpy(inst,"LOGIN");
			printf("%s", ans);
			char usr[100];
			printf("Ingrese el nombre de usuariuo:\n");
			scanf("%s",usr);
			char email[100];
			printf("Ingrese el correo usuariuo:\n");
			scanf("%s",email);
			char pass[100];
			printf("Ingrese la contraseña de usuario:\n");
			scanf("%s",pass);
			char expresion[1000];
			nJsonFile2(expresion, inst, usr, email, pass);
			cifrar(expresion);
			strcpy(dir,expresion);
			printf("mensaje cifrado enviado: %s\n",dir);
			if ( send(sd, dir, sizeof(dir), 0) == -1 ) {
				perror("send");
				exit(1);
			}
			int n;
			n=recv(sd, dir, sizeof(dir), 0);
		/* esperar por la respuesta */
			if (  n== -1 ) {
				perror("recv");
				exit(1);
			}
			dir[n]='\0';
			int idc,fil;
		/* imprimimos el resultado y cerramos la conexion del socket */
			printf("respuesta cifrada: %s\n", dir);
			descifrar(dir);
			printf("respuesta descifrada: %s\n",dir);
			readJsonFile(dir, inst, &idc, &fil, usr);
			printf("instruccion: %s\n",inst);
			if(strcmp(inst, "LOGIN_SUCCESSFULL")==0){
				printf("Login exitoso\n");
				login='S';
			}
		}
	}
	
	
	close(sd);
	return 0;
}
