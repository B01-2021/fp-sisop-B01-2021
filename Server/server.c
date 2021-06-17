#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#define PORT 8080

int cek_data(char data[], char nama_file[]){
    char path_file[1024];
    sprintf(path_file, "databases/client_database/%s", nama_file);

    FILE *fakun;
    fakun= fopen(path_file,"r");
    if (fakun == NULL) {
        perror("fopen()");
        return EXIT_FAILURE;
    }
    char line[1024];
    while (fgets(line , sizeof(line) , fakun)!= NULL)
    {   
        if (strstr(line , data)!= NULL){
            return 1;
        }
    }
    return 0;
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    char *path="/home/bunga/FP/Server";
    int root;
    char auth_akun[1024] = {0};

    recv(new_socket, &root, sizeof(root), 0);
    if(!root){
        char auth_response[1024] = {0};
        valread = read( new_socket , auth_akun, 1024);
        // printf("auth akun : %s\n", auth_akun);
        int cek = cek_data(auth_akun, "client_account.txt");
        // printf("cek = %d\n", cek);
        if(cek){
            sprintf(auth_response, "Auth berhasil\n");
        }
        else{
            sprintf(auth_response, "Akunmu tidak terdaftar\n");
        }
        send(new_socket, auth_response, sizeof(auth_response), 0);
    }

    char database_used[1024];
    database_used[0]='\0';
    
    while(1){
        char request[1024] = {0};
        char response[1024] = {0};
        valread = read( new_socket , request, 1024);
        printf("%s", request);

        char *create_user = strstr(request, "CREATE USER");
        char *create_database = strstr(request, "CREATE DATABASE");
        char *use= strstr(request, "USE");
        if (create_user){
            if(root){
                char akun[20] = {0};
                int u=0;
                char password[20] = {0};
                int p=0;
                for(int i=12; i<strlen(request); i++){
                    if(request[i] == 32){
                        while(1){
                            i++;
                            if(request[i] == 32 && request[i-1] == 'Y' && request[i-2] == 'B'){
                                i++;
                                akun[u] = ',';
                                u++;
                                break;
                            }
                        }
                    }
                    akun[u] = request[i];
                    u++;
                }
                char path_file_akun[100];
                sprintf(path_file_akun, "databases/client_database/client_account.txt");
                // printf("%s\n", path_file_akun);

                FILE *fakun;
                fakun= fopen(path_file_akun,"a");
                if (fakun == NULL) {
                    perror("fopen()");
                    return EXIT_FAILURE;
                }
                fputs(akun, fakun);
                fputs("\n", fakun);
                fflush(fakun);
                fclose(fakun);
                sprintf(response, "CREATE USER SUCCESS\n");
            }
            else{
                sprintf(response, "ACCESS DENIED!!!\n");
            }
            send(new_socket, response, sizeof(response), 0);
        }

        if (create_database){
            char nama_database[1024]={0};
            char path_database_baru[1024]={0};
            int ii, n=0;
            for(ii=strlen(request); ii>0; ii--){
                if(request[ii] == 32)
                    break;
            }
            for(int i=ii+1; i<strlen(request)-2; i++){
                nama_database[n]=request[i];
                n++;
            }
            
            printf("%s\n", nama_database);
            sprintf(path_database_baru, "databases/%s", nama_database);
            int result = mkdir(path_database_baru, 0777);
            if(!result){
                printf("mkdir gagal");
            }

            if(!root){
                char access_database[1024]={0};
                sprintf(access_database, "%s,%s", nama_database, auth_akun);

                FILE *faccess;
                faccess= fopen("databases/client_database/access_account.txt","a");
                if (faccess == NULL) {
                    perror("fopen()");
                    return EXIT_FAILURE;
                }
                fputs(access_database, faccess);
                fputs("\n", faccess);
                fflush(faccess);
                fclose(faccess);
            }
            database_used[0]='\0';
            strcpy(database_used, nama_database);
            sprintf(response, "CREATE DATABASE SUCCESS\n");
            send(new_socket, response, sizeof(response), 0);
        }   
        
        if (use){
            database_used[0]='\0';
            int ii, n=0;
            for(ii=strlen(request); ii>0; ii--){
                if(request[ii] == 32)
                    break;
            }
            for(int i=ii+1; i<strlen(request)-2; i++){
                database_used[n]=request[i];
                n++;
            }
            char akses[1024];
            sprintf(akses, "%s,%s", database_used, auth_akun);
            int cek = cek_data(akses, "access_account.txt");
            if(cek || root){
                sprintf(response, "%s USED\n", database_used);
            }
            else{
                sprintf(response, "ACCESS DENIED!!!\n");
            }
            
            send(new_socket, response, sizeof(response), 0);

        }

        char *exit = strstr(request, "exit");
        if(exit)
            break;
    }
   
    // printf("%s\n",buffer );
    // send(new_socket , hello , strlen(hello) , 0 );
    // printf("Hello message sent\n");
    return 0;
}