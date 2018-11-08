
/*
    Ho va ten  : Dong Van Thanh
    MSSV: 16021647
    Lop: K61N
    Mo ta: Lap trinh may chu xu li da luong
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
int counter = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
void *connection_handler(void *);
int main(){
    int server_sockfd, client_sockfd;
    int server_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int port = 1235;
    int max_client = 10;
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

    while(1){
    //accept
        unsigned int client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);
        if(client_sockfd < 0){
            perror("Accept Error");
            exit(1);
        }
        pthread_t thread_id;
        if(pthread_create(&thread_id, NULL, connection_handler, (void *)&client_sockfd) < 0){
            perror("could not create thread");
            return 1;
        }
        // pthread_join(thread_id, NULL);
    }
    close(client_sockfd);
    return 0;
}

void *connection_handler(void *server_sockfd){
    int sock = *(int *)server_sockfd;
    char quit[] = "QUIT";
    int bufferSize = 1024;
    int nbytes;
    pthread_detach(pthread_self());
    while(1){
        char buffer[bufferSize];
        char fileName[bufferSize];
        printf("Server waiting...\n");
        if((nbytes = read(sock, fileName,sizeof(buffer))) <= 0){
            break;
        }
        if(strcmp(fileName, quit) == 0){
            printf("Disconnected from client \n");
            close(sock);
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
        printf("Send file %s completed\n",fileName);
        pthread_mutex_lock(&counter_mutex);
        counter++;
        printf("Total files sent to client: %d\n",counter);
        pthread_mutex_unlock(&counter_mutex);
    }
    close(sock);
    return NULL;
}
