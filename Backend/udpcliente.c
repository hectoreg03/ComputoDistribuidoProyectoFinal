/*

   Conexion UDP
   Codigo del cliente

   Nombre Archivo: udpcliente.c   
   Fecha: Febrero 2023

   Compilacion: cc udpcliente.c -lnsl -o udpcliente
   Ejecución: ./udpcliente
*/
// udp client driver program 
#include <stdio.h> 
#include <string.h> 
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <stdlib.h> 
  
//#define PORT 15000 
#define MAXLINE 1000 
  
// Driver code 
int main() 
{    
    char buffer[MAXLINE]; 
    char ip[]="172.18.2.2";
    char *message = "{   \"inst\":\"REGISTER\",   \"user\":\"usuario1\",   \"email\":\"jhgasdjgf@gmail.com\",   \"password\":\"contra\" }"; 
    //char *message = "{   \"inst\":\"REGISTER\" }"; 
    int sockfd; 
    int port=5001;
    struct sockaddr_in servaddr; 
      
    // clear servaddr 
    bzero(&servaddr, sizeof(servaddr)); 
    /*
    printf("Escribe la ip del server: ");
    scanf("%s",ip);
    printf("Escribe el puerto del server: ");
    scanf("%d",&port);*/
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
    }else{
        buffer[n] = '\0'; 
        printf("\nHe recibido del server: ");
        printf("%s\n",buffer);
    }
    // close the descriptor 
    close(sockfd); 
    printf("Conexion cerrada\n");
} 
