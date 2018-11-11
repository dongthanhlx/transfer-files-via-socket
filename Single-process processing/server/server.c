
/*
    Ho va ten  : Dong Van Thanh
    MSSV: 16021647
    Lop: K61N
    Mo ta: Lap trinh may chu xu li don tien trinh
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
#include<pthread.h>
#include<error.h>
#include<sys/select.h>
int counter = 0;
const int bufferSize = 1024;
const int QLEN = 5;

int main(){
    int server_sockfd, client_sockfd, maxfd, maxi, sock;
    int server_len, client[FD_SETSIZE], nready;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int port = 1235;
    int max_client = 10;
    int opt = 1,i;
    char *quit = "QUIT";
    fd_set rset, allset;

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
    maxfd = server_sockfd;
    maxi = -1;
    for(i=0; i<FD_SETSIZE; i++) client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(server_sockfd, &allset);
    unsigned int client_len;
    while(1){
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if(FD_ISSET(server_sockfd, &rset)){
            client_len = sizeof(client_address);
            client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);
            if(client_sockfd < 0){
                perror("Accept Error");
                exit(1);
            }
            for(i=0; i<FD_SETSIZE; i++){
                if(client[i] < 0){
                    client[i] = client_sockfd;
                    break;
                }
            }
            if(i == FD_SETSIZE){
                perror("Too many client");
                exit(1);
            }
            FD_SET(client_sockfd, &allset);
            if(client_sockfd > maxfd)   maxfd = client_sockfd;
            if(i > maxi)    maxi = i;
            if(--nready <= 0)   continue;
        }
            for(i = 0; i<= maxi; i++){
                char fileName[bufferSize];
                char buffer[bufferSize];
                if((sock = client[i]) < 0)  continue;
                if(FD_ISSET(sock, &rset)){
                    if((read(sock, fileName,sizeof(buffer))) == 0){
                        close(sock);
                        FD_CLR(sock, &allset);
                        client[i] = -1;
                    }
                    else {
                        if(strcmp(fileName, quit) == 0){
                            printf("Disconnected from client \n");
                            break;
                        }
                        int fileSize = 0;
                        if(access(fileName, 0) == -1){
                            write(sock, &fileSize, sizeof(int));
                            continue;
                        }
                        FILE *f = fopen(fileName, "rb");
                        fseek(f, 0, SEEK_END);
                        fileSize = ftell(f);
                        rewind(f);
                        write(sock, &fileSize,sizeof(int));
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
                            write(sock, buffer, sizeRead);
                            bzero(buffer, sizeof(buffer));
                        }
                        fclose(f);
                        printf("Send file %s (%d byte) completed\n",fileName,fileSize);
                        counter++;
                        printf("Total files sent to client: %d\n",counter);
                    }
                }
            }
        
    }

    close(client_sockfd);
    return 0;
}

