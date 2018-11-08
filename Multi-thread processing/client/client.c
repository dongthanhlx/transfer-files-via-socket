/*
    Ho va ten  : Dong Van Thanh
    MSSV: 16021647
    Lop: K61N
    Mo ta: Lap trinh may chu xu li da luong
    File :client
*/
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/un.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<time.h>
int main(){
    int sockfd,bufferSize;
    int len,port;
    struct sockaddr_in address;
    int result;
    char ip[100], quit[] = "QUIT";
    //socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("Error in connection\n");
        exit(1);
    }
    printf("IP: ");
    scanf("%s",ip);
    printf("Port: ");
    scanf("%d",&port);
    printf("Size of buffer: ");
    scanf("%d",&bufferSize);
    getchar();
    char buffer[bufferSize];
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF,&bufferSize, sizeof(bufferSize));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);
    len = sizeof(address);

    //connect

    result = connect(sockfd, (struct sockaddr*)&address, len);
    if(result < 0){
        perror("Opps: client problem");
        exit(1);
    }

    while(1){
        char fileName[bufferSize];
        printf("Enter fileName: ");
        fgets(fileName,sizeof(fileName),stdin);
        fileName[strlen(fileName) -1] = '\0';
        //send fileName
        
        if((strcmp(fileName,quit) == 0)){
            printf("Disconnected \n");
            write(sockfd, fileName,strlen(fileName)+1);
            exit(1);
        }
        write(sockfd, fileName,strlen(fileName)+1);
        int fileSize = 0;
        read(sockfd, &fileSize, sizeof(fileSize));
        if(fileSize <= 0){
            printf("File not found\n");
            continue;
        }
        FILE *f = fopen(fileName, "arb");
        if(f == NULL){
            perror("Error file");
        } 
        int bytesReceived = 0;
        bzero(buffer, bufferSize);
        clock_t start_counter = clock();
        while(bytesReceived < fileSize){
            int current = read(sockfd, buffer, bufferSize);
            fwrite(buffer, 1, current, f);
            bytesReceived += current;
            bzero(buffer, bufferSize);
        }
        clock_t end_counter = clock();  
        printf("Download file completed with time %f s\n",(double)(end_counter - start_counter)/CLOCKS_PER_SEC);
        fclose(f);
    }

    close(sockfd);
    return 0;
}
