#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    int root = 0;
    if(getuid() != 0){
        send(sock, &root, sizeof(root), 0);
        char akun[1024] = {0};
        char auth_response[1024] = {0};
        sprintf(akun, "%s,%s", argv[2], argv[4]);
        // printf("%s\n", akun);
        send(sock , akun, strlen(akun) , 0 );
        valread = read(sock, auth_response, 1024);
        printf("%s", auth_response);
        
    }
    else {
        root=1;
        send(sock, &root, sizeof(root), 0);
    }
    
    while(1){
        char request[1024] = {0};
        char response[1024] = {0};
        fgets(request, sizeof(request), stdin);
        send(sock , request, strlen(request) , 0 );
        valread = read(sock, response, 1024);
        printf("%s", response);
        char *exit = strstr(request, "exit");
        if(exit)
            break;
    }
    // send(sock , hello , strlen(hello) , 0 );
    // printf("Hello message sent\n");
    // valread = read( sock , buffer, 1024);
    // printf("%s\n",buffer );
    
    return 0;
}