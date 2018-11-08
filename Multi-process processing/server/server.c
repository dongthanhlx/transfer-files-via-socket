
/*
    Ho va ten  : Dong Van Thanh
    MSSV: 16021647
    Lop: K61N
    Mo ta: Lap trinh may chu xu li da tien trinh
    File : server
*/
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/un.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<time.h>

void sig_chld(int signo){
    pid_t pid;
    int status;
    while ((pid = waitpid (-1, &status, WNOHANG) )> 0)
    /* empty */;
    return;
}

int main(){
    int server_sockfd, client_sockfd, nbytes, bufferSize = 1024;
    int server_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    char quit[] = "QUIT";
    int port = 1236;
    int max_client = 10;
    pid_t childpid;
    int opt = 1;
    bzero(&server_address, sizeof(server_address));
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_address.sin_port = htons(port);
    server_len = sizeof(server_address);
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    //bind

    if(bind(server_sockfd, (struct sockaddr*)&server_address, server_len) < 0){
        printf("Error in binding \n");
        exit(1);
    }

    //listen
    if(-1 == listen(server_sockfd, max_client)){
        perror("Listen Error");
    }
    printf("Server listen at port %d\n",port);

    signal(SIGCHLD, sig_chld);
    while(1){
    //accept
        unsigned int client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);
        if(client_sockfd < 0){
            perror("Accept Error");
            exit(1);
        }
        if((childpid = fork()) == 0){
            close(server_sockfd);
            while(1){
                char buffer[bufferSize];
                printf("Server waiting...\n");
                if((nbytes = read(client_sockfd, buffer,sizeof(buffer))) <= 0){
                    break;
                }
                if(strcmp(buffer, quit) == 0){
                    printf("Disconnected from client\n");
                    close(client_sockfd);
                    break;
                }
                int fileSize = 0;
                if(access(buffer, 0) == -1){
                    write(client_sockfd, &fileSize, sizeof(int));
                    continue;
                }
                FILE *f = fopen(buffer, "rb");
                fseek(f, 0, SEEK_END);
                fileSize = ftell(f);
                rewind(f);
                write(client_sockfd, &fileSize,sizeof(int));
                if(f == NULL){
                    printf("File not found");
                    break;
                }
                bzero(buffer, sizeof(buffer));
                while(!feof(f)){
                    int sizeRead = fread(buffer, 1, sizeof(buffer), f);
                    if(sizeRead < 0){
                        printf("Error read");
                        exit(1);
                    }
                    write(client_sockfd, buffer, sizeRead);
                    bzero(buffer, sizeof(buffer));
                }
                printf("Send file completed\n");
                fclose(f);
            }
        }
    }
    close(client_sockfd);
    return 0;
}