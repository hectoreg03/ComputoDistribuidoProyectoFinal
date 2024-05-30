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
#include <stdbool.h>
#include "cJSON.h"
#include "SHA256.h"

#define PORT 5000 
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

// Assuming these structures and functions are defined as per the previous example
typedef struct {
    char user[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int isActive;
    int msgSent;
} User;

User users[MAX_USERS];

typedef struct {
    User* admin;
    char* name;
    User** users;
    size_t userCount;
} ChatRoom;

typedef struct {
    char **messages;  // Pointer to an array of strings
    int size;         // Current number of messages
    int capacity;     // Current capacity of the array
} Messages;


User* create_user(const char* user, const char* password, int isActive);
ChatRoom* create_chatroom(User* admin, const char* name, User** users, size_t userCount);
void free_user(User* user);
void free_chatroom(ChatRoom* chatRoom);
char* json_stringify_chatroom(ChatRoom* chatRoom);
char* json_stringify_chatrooms(ChatRoom** chatRooms, size_t chatRoomCount);
void handle_new_chatroom(ChatRoom* newChatRoom);
void send_message(const char* message,const char* user) ;
void broadcast(const char* message, Messages *array);

// Function prototypes
void registerUser(char* ans, char* user,  char * password,  int *userCount);
void loginUser(char* ans, char* user,  char * password, int *userCount);

// Example list of chat rooms
ChatRoom** chatRooms = NULL;
size_t chatRoomCount = 0;
  
bool room_exists(const char* roomName) {
    for (size_t i = 0; i < chatRoomCount; i++) {
        if (strcmp(chatRooms[i]->name, roomName) == 0) {
            return true;
        }
    }
    return false;
}

// Function to initialize the dynamic array
void initMessages(Messages *array, int initialCapacity) {
    array->messages = malloc(initialCapacity * sizeof(char *));
    array->size = 0;
    array->capacity = initialCapacity;
}

// Function to add a message to the dynamic array
void addMessage(Messages *array, const char *message) {
    // Check if we need to resize the array
    if (array->size == array->capacity) {
        // Double the capacity
        array->capacity *= 2;
        array->messages = realloc(array->messages, array->capacity * sizeof(char *));
    }
    // Allocate memory for the new message and copy it
    array->messages[array->size] = malloc(strlen(message) + 1);
    strcpy(array->messages[array->size], message);
    array->size++;
}

// Function to free the memory allocated for the array
void freeMessages(Messages *array) {
    for (int i = 0; i < array->size; i++) {
        free(array->messages[i]);
    }
    free(array->messages);
    array->messages = NULL;
    array->size = 0;
    array->capacity = 0;
}

// Function to print all messages in the dynamic array
void printMessages(Messages *array) {
    for (int i = 0; i < array->size; i++) {
        printf("Message %d: %s\n", i + 1, array->messages[i]);
    }
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


void readCrendentialsJson(const char* jsonString, char* user, char* password) {
    // Parse the JSON string
    cJSON* json = cJSON_Parse(jsonString);
    if (!json) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }
    
    char tUser[MAX_USERNAME_LENGTH];
    char tPassword[MAX_PASSWORD_LENGTH];

    // Get the "user" and "password" fields from the JSON object
    cJSON* userJson = cJSON_GetObjectItem(json, "user");
    cJSON* passwordJson = cJSON_GetObjectItem(json, "password");

    if (cJSON_IsString(userJson) && cJSON_IsString(passwordJson)) {
        // Copy the values to the UserCredentials structure
        //tUser = strdup(userJson->valuestring);
        //tPassword = strdup(passwordJson->valuestring);
        strcpy(user,strdup(userJson->valuestring));
        strcpy(password,strdup(passwordJson->valuestring));
    } else {
        fprintf(stderr, "Invalid JSON format\n");
        cJSON_Delete(json);
        return ;
    }

    // Clean up the cJSON object
    cJSON_Delete(json);
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

void write_chatrooms_to_file(const char* filename) {
    char* jsonString = json_stringify_chatrooms(chatRooms, chatRoomCount);
    if (jsonString) {
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%s", jsonString);
            fclose(file);
            printf("[CHATROOMS SAVED]\n");
        } else {
            perror("Error writing to file");
        }
        free(jsonString);
    }
}

void handle_new_chatroom(ChatRoom* newChatRoom) {
	/*
    if (room_exists(newChatRoom->name)) {
        send_message("{\"inst\": \"ROOM_ALREADY_EXISTS\"}");
        return;
    }

    // Append the new chat room to the list
    chatRooms = realloc(chatRooms, sizeof(ChatRoom*) * (chatRoomCount + 1));
    chatRooms[chatRoomCount] = newChatRoom;
    chatRoomCount++;

    send_message("{\"inst\": \"ROOM_CREATED_SUCCESSFULLY\"}");

    write_chatrooms_to_file("chatRooms.json");

    // Broadcast updated chat rooms
    char* jsonString = json_stringify_chatrooms(chatRooms, chatRoomCount);
    if (jsonString) {
        broadcast(jsonString);
        free(jsonString);
    }*/
}
  
  
  
void send_message(const char* message,const char* user) {
    // Placeholder for sending message to client
    printf("Sending message to client: %s\n", message);
}

void broadcast(const char* message, Messages *array) {
    // Placeholder for broadcasting message to all clients
    printf("Broadcasting message to all clients: %s\n", message);
    addMessage(array,message);
}
  
// Driver code 
int main(){   
	Messages allMessages;
    initMessages(&allMessages, 2); // Initial capacity of 2
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
    	int isActive=0;
        readInstruction(buffer,inst);
        if(strcmp(inst,"LOGIN")==0){
        	readCrendentialsJson(buffer, username, password);
			printf("intentando Login: %s\n",username);
        	ans=1;
			loginUser(message, username,password, &userCount);
		}
	    if(strcmp(inst,"REGISTER")==0){
        	readCrendentialsJson(buffer, username, password);
			printf("Intentando Register: %s\n",username);
        	ans=1;
			registerUser(message,username,password, &userCount);
		}
	    if(strcmp(inst,"DISCONNECT")==0){
			printf("Desconectando\n");
        	ans=1;
			//disconnectUser(message,username,password, &userCount);
		}
		
	    if(strcmp(inst,"MESSAGE")==0){
	    	broadcast(buffer,&allMessages);
			printf("Intentando Register\n");
        	ans=1;
			registerUser(message,username,password, &userCount);
		}
	    if(strcmp(inst,"CREATE_ROOM")==0){
		printf("Intentando Register\n");
        	ans=1;
			registerUser(message,username,password, &userCount);
		}
		/*
	    if(strcmp(inst,"REGISTER")==0){
		printf("Intentando Register\n");
        	ans=1;
			registerUser(message,username,password, &userCount);
		}
	    if(strcmp(inst,"REGISTER")==0){
		printf("Intentando Register\n");
        	ans=1;
			registerUser(message,username,password, &userCount);
		}*/
		if(ans==0){
        	createInstJson(message,"Unknown_Instruction");
		}
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
void registerUser(char* ans, char* user, char * password, int *userCount) {
    if (*userCount >= MAX_USERS) {
        printf("Cannot register more users. Maximum user limit reached.\n");
        	createInstJson(ans,"USER_ALREADY_EXISTS");
        return;
    }

    printf("Registering User\n");
    strcpy(users[*userCount].user,user);
    users[*userCount].isActive=0;
    users[*userCount].msgSent=0;
    strcpy( users[*userCount].password,password);

    printf("User registered successfully.\n");
    (*userCount)++;
    
    createInstJson(ans,"REGISTER_SUCCESSFULL");
}

// Function to login a user
void loginUser(char* ans, char* user,  char * password, int *userCount) {
	
    for (int i = 0; i < *userCount; i++) {
        if (strcmp(users[i].user, user) == 0 && strcmp(users[i].password, password) == 0) {
        	users[i].isActive=1;
        	createInstJson(ans,"LOGIN_SUCCESSFULL");
            return;
        }
    }
    createInstJson(ans,"USER_NOT_FOUND");
}


// Implementations for the JSON conversion functions
char* json_stringify_user(User* user) {
    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "user", user->user);
    cJSON_AddStringToObject(json, "password", user->password);
    cJSON_AddBoolToObject(json, "isActive", user->isActive);
    char* jsonString = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return jsonString;
}

char* json_stringify_chatroom(ChatRoom* chatRoom) {
    cJSON* json = cJSON_CreateObject();
    cJSON* adminJson = cJSON_CreateObject();
    cJSON_AddStringToObject(adminJson, "user", chatRoom->admin->user);
    cJSON_AddStringToObject(adminJson, "password", chatRoom->admin->password);
    cJSON_AddBoolToObject(adminJson, "isActive", chatRoom->admin->isActive);
    cJSON_AddItemToObject(json, "admin", adminJson);
    cJSON_AddStringToObject(json, "name", chatRoom->name);
    cJSON* usersJson = cJSON_CreateArray();
    for (size_t i = 0; i < chatRoom->userCount; i++) {
        cJSON* userJson = cJSON_CreateObject();
        cJSON_AddStringToObject(userJson, "user", chatRoom->users[i]->user);
        cJSON_AddStringToObject(userJson, "password", chatRoom->users[i]->password);
        cJSON_AddBoolToObject(userJson, "isActive", chatRoom->users[i]->isActive);
        cJSON_AddItemToArray(usersJson, userJson);
    }
    cJSON_AddItemToObject(json, "users", usersJson);
    char* jsonString = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return jsonString;
}

char* json_stringify_chatrooms(ChatRoom** chatRooms, size_t chatRoomCount) {
    cJSON* jsonArray = cJSON_CreateArray();
    for (size_t i = 0; i < chatRoomCount; i++) {
        cJSON* chatRoomJson = cJSON_CreateObject();
        cJSON* adminJson = cJSON_CreateObject();
        cJSON_AddStringToObject(adminJson, "user", chatRooms[i]->admin->user);
        cJSON_AddStringToObject(adminJson, "password", chatRooms[i]->admin->password);
        cJSON_AddBoolToObject(adminJson, "isActive", chatRooms[i]->admin->isActive);
        cJSON_AddItemToObject(chatRoomJson, "admin", adminJson);
        cJSON_AddStringToObject(chatRoomJson, "name", chatRooms[i]->name);
        cJSON* usersJson = cJSON_CreateArray();
        for (size_t j = 0; j < chatRooms[i]->userCount; j++) {
            cJSON* userJson = cJSON_CreateObject();
            cJSON_AddStringToObject(userJson, "user", chatRooms[i]->users[j]->user);
            cJSON_AddStringToObject(userJson, "password", chatRooms[i]->users[j]->password);
            cJSON_AddBoolToObject(userJson, "isActive", chatRooms[i]->users[j]->isActive);
            cJSON_AddItemToArray(usersJson, userJson);
        }
        cJSON_AddItemToObject(chatRoomJson, "users", usersJson);
        cJSON_AddItemToArray(jsonArray, chatRoomJson);
    }
    char* jsonString = cJSON_PrintUnformatted(jsonArray);
    cJSON_Delete(jsonArray);
    return jsonString;
}
