/*

   Conexion UDP
   Codigo del servidor

   Nombre Archivo: udpserver.c   
   Fecha: Febrero 2023

   Compilacion: cc udpserver.c cJSON.c SHA256.c -lnsl -o udpserver
   Ejecución: ./udpserver

*/

// server program for udp connection 
#include <stdio.h> 
#include <string.h> 
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <stdlib.h>
#include "cJSON.h"
#include "SHA256.h"

#define PORT 5001 
#define MAXLINE 1000 
#define Key 1

#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50
#define MAX_EMAIL_LENGTH 100
#define MAX_USERS 100

//Instruccion Invalida -1
//Login exitoso 1
// Login fallo 2
//Registrar exitoso 3
// Registrar fallo 4
//

// Structure to represent a user
struct User {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char email[MAX_EMAIL_LENGTH]; 
};

// Function prototypes
int registerUser(char* user, char* email, char * password, struct User *users, int *userCount);
int loginUser(char* user, char* email, char * password,struct User *users, int *userCount);

void readJsonFile(char *archivo, char *inst,char *user,char *email,char *password){
	
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
	    
	    char* userValue;
	    
	    char* emailValue;
	    
	    char* passwordValue;
	    
	    cJSON* userJ = cJSON_GetObjectItemCaseSensitive(json, "user");
	    if (userJ == NULL) {
	        fprintf(stderr, "JSON does not contain 'user' parameter\n");
	    } else{
	
	    // Extract the value as a string
	    userValue = strdup(userJ->valuestring);
		}
		
	    cJSON* emailJ = cJSON_GetObjectItemCaseSensitive(json, "user");
	    if (emailJ == NULL) {
	        fprintf(stderr, "JSON does not contain 'email' parameter\n");
	    } else{
			emailValue = strdup(emailJ->valuestring);
		}
		
	    cJSON* passwordJ = cJSON_GetObjectItemCaseSensitive(json, "user");
	    if (passwordJ == NULL) {
	        fprintf(stderr, "JSON does not contain 'password' parameter\n");
	    } else{
			passwordValue = strdup(passwordJ->valuestring);
		}
		
		
		strcpy(inst,instValue);
		strcpy(user,userValue);
		strcpy(email,emailValue);
		strcpy(password,passwordValue);
	}

    // Clean up
    cJSON_Delete(json);
	printf("todo salio \n");
	
	
	
}
char *SHA256( char* message){
	BYTE buf[SHA256_BLOCK_SIZE];
	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, message, strlen(message));
	sha256_final(&ctx, buf);
	char sal[SHA256_BLOCK_SIZE];
	for( int i=0; i<SHA256_BLOCK_SIZE; i++){
		sal[i]=buf[i];
	}
}
  
// Driver code 
int main(){    
	struct User users[MAX_USERS];
	int userCount = 0;
    char buffer[MAXLINE]; 
    char message[500] = "Hello Client"; 
    int listenfd, len; 
    struct sockaddr_in servaddr, cliaddr; 
    int empezar;
    printf("Listening in port number: %d\n", PORT);
    //printf("\nPresione cualquier tecla para empezar \n");
    //scanf("%d",&empezar);

    //bzero(&servaddr, sizeof(servaddr)); 
  
    
    while(1){
		// Create a UDP Socket 
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(listenfd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }         

    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_family = AF_INET;  

    // bind server address to socket descriptor 
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
    //receive the datagram 
    len = sizeof(cliaddr); 
    int n = recvfrom(listenfd, buffer, MAXLINE, 
            0, (struct sockaddr*)&cliaddr,&len); //receive message from server 
    if(n < 0) {
        perror("recvfrom failed");
        exit(EXIT_FAILURE);
    }else{
        buffer[n] = '\0'; 
		printf("mensaje recibido %s\n", buffer);
        char inst[20];
        int ans=-1;
    	char username[MAX_USERNAME_LENGTH];
    	char password[MAX_PASSWORD_LENGTH];
    	char email[MAX_EMAIL_LENGTH];
        readJsonFile(buffer,inst,username,email,password);
        if(strcmp(inst,"LOGIN")==0){
		printf("intentando Login\n");
        	ans=loginUser(username,email,password,users, &userCount);
		}
        if(strcmp(inst,"REGISTER")==0){
		printf("Intentando Register\n");
        	ans=registerUser(username,email,password, users, &userCount);
		}
		sprintf(message, "%d", ans);
		printf("message proccessed\n");
    }       
    // send the response 
    sendto(listenfd, message, strlen(message), 0, 
          (struct sockaddr*)&cliaddr, sizeof(cliaddr)); 
    close(listenfd);
    printf("Conexion cerrada\n");
	}
    return 0;
} 

// Function to register a new user
int registerUser(char* user, char* email, char * password, struct User *users, int *userCount) {
    if (*userCount >= MAX_USERS) {
        printf("Cannot register more users. Maximum user limit reached.\n");
        return 4;
    }

    printf("Registering User\n");
    strcpy(users[*userCount].username,user);
    strcpy(users[*userCount].email,email);
    char *hashpass=  SHA256(password);
    strcpy( users[*userCount].password,password);

    printf("User registered successfully.\n");
    (*userCount)++;
    return 3;
}

// Function to login a user
int loginUser(char* user, char* email, char * password, struct User *users, int *userCount) {
    char *hashpass=  SHA256(password);
    for (int i = 0; i < *userCount; i++) {
        if (strcmp(users[i].username, user) == 0 && strcmp(users[i].password, hashpass) == 0) {
            return 1;
        }
    }
    
    return 2;
}
