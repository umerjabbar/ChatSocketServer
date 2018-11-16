//
//  Server.c
//  SPMileStone2
//
//  Created by Umer Jabbar on 27/04/2018.
//  Copyright © 2018 ZotionApps. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/types.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define PORT 20012

struct User {
    int id;
    char name[30];
    char image[150];
};

struct Message {
    int id;
    char body[10000];
    int senderId;
    char senderName[30];
    char senderImage[150];
    char time[50];
    int type;
};

struct Conversation{
    int id;
    int user1Id;
    int user2Id;
    struct Message messages[500];
    char time[50];
};

struct sockaddr_in serv_addr;
struct Conversation conversations[500];
struct User users[100];

void* connection(void* sock);

int main (){
    
    int listenfd = 0;
    char sendBuff[1025];
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
    
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);
    
    printf("Connect to server on port %d, %u ", PORT, INADDR_ANY);
    
    while(1){
        
        struct sockaddr_in clients;
        int c = sizeof(struct sockaddr_in);
        int connfd = accept(listenfd, (struct sockaddr*) &clients, (socklen_t *) &c );
        if(connfd == -1){
            perror("connectfd");
        }
        char hostname[1024];
        
        if(getnameinfo((struct sockaddr*)&serv_addr, sizeof(serv_addr), hostname, 1024, NULL, 0, 0) == -1){
            perror("getnameInfo");
        }else{
            
        }
        
        printf("\nClient is Connected on socket id %d\n", connfd);
        
        pthread_t sTod;
        pthread_create(&sTod, NULL, connection, (void*) &connfd);
        
    }
    
    return 0;
}


void* connection(void* sock){
    int sockfd = *(int*) sock;
    
    while(1){
        char buff[10000];
        ssize_t r1 = recv(sockfd, buff, 10000, 0);
        if (r1 == -1){
            perror("error in read socket");
            pthread_exit(NULL);
        } if(r1 == 0){
            perror("error in read socket");
            pthread_exit(NULL);
        }
        buff[r1-1] = '\0';
        printf("~ Client %d,  %s \n", sockfd, buff);
        
        char * token;
        token = strtok(buff, "$\n");
        if(token != NULL){
            if(strcmp(token, "add") == 0){
                
            }
        }
        
        
        
    }
//    pthread_exit(NULL);
}











