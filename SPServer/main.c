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
#include <sys/ioctl.h>

#define PORT 2020
#define UPLOADPORT 8080
#define USERS 10
#define CONVERSATIONS 50

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
struct sockaddr_in upload_addr;
struct Conversation conversations[CONVERSATIONS];
struct User users[USERS];

void checkHostName(int hostname);
void checkHostEntry(struct hostent * hostentry);
void checkIPbuffer(char *IPbuffer);
void* connection(void* sock);
void setup(void);
int updateUserBySockId(int fd, char * name);
void* uploadTask(void* sock);
void* uploadThread();

int main (){
    
//    char hostbuffer[256];
//    char *IPbuffer;
//    struct hostent *host_entry;
//    int hostname;
    
    int listenfd = 0;
    char sendBuff[1025];
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
    
    //Hostname
    // To retrieve hostname
//    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
//    checkHostName(hostname);
//
//    // To retrieve host information
//    host_entry = gethostbyname(hostbuffer);
//    checkHostEntry(host_entry);
//
//    // To convert an Internet network
//    // address into ASCII string
//    IPbuffer = inet_ntoa(*((struct in_addr*)
//                           host_entry->h_addr_list[0]));
//
//    printf("Hostname: %s\n", hostbuffer);
//    printf("Host IP: %s\n", IPbuffer);
//    printf("Port %d\n", PORT);
    
    int enable = 1;
    int sock1 = setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &enable, 4);
    if(sock1 == -1){
        perror("setSockOpt");
    }
    
    int b1 = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (b1 == -1){
        perror("bind");
    }
    int l1 = listen(listenfd, 10);
    if (l1 == -1){
        perror("listen");
    }
    
    pthread_t uploadFile;
    int pThreadFd = pthread_create(&uploadFile, NULL, uploadThread, NULL);
    if (pThreadFd == -1){
        perror("upload thread error");
    }
    
    setup();
    
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
        int pThreadFd = pthread_create(&sTod, NULL, connection, (void*) &connfd);
        if (pThreadFd == -1){
            perror("pthread error");
        }
        
    }
    
    return 0;
}

void* uploadThread(){
    
    int listenfd = 0;
    char sendBuff[1025];
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&upload_addr, '0', sizeof(upload_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    upload_addr.sin_family = AF_INET;
    upload_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    upload_addr.sin_port = htons(UPLOADPORT);
    int enable = 1;
    int sock1 = setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &enable, 4);
    if(sock1 == -1){
        perror("setSockOpt");
    }
    int b1 = bind(listenfd, (struct sockaddr*)&upload_addr, sizeof(upload_addr));
    if (b1 == -1){
        perror("bind");
    }
    int l1 = listen(listenfd, 10);
    if (l1 == -1){
        perror("listen");
    }
    while(1){
        struct sockaddr_in clients;
        int c = sizeof(struct sockaddr_in);
        int connfd = accept(listenfd, (struct sockaddr*) &clients, (socklen_t *) &c );
        if(connfd == -1){
            perror("connectfd");
            continue;
        }
        printf("\nClient is Connected on socket id %d for uploading\n", connfd);
        
        pthread_t sTod;
        int pThreadFd = pthread_create(&sTod, NULL, uploadTask, (void*) &connfd);
        if (pThreadFd == -1){
            perror("pthread error");
        }
    }
}

void* uploadTask(void* sock){
    int sockfd = *(int*) sock;
    
    //Read Picture Byte Array
    printf("Reading Picture Byte Array\n");
    char p_array[10270];
    int image = open("cs1.png", O_CREAT | O_RDWR | S_IRUSR | S_IWUSR);
    ssize_t nb = read(sockfd, p_array, 10270);
    while (nb > 0) {
        write(image, p_array, nb);
        printf(" will send ");
        int count;
        ioctl(sockfd, FIONREAD,&count);
        if (count <= 0){
            break;
        }
        nb = recv(sockfd, p_array, nb, 0);
        if (nb <= 0){
            perror("\nrecv file");
            break;
        }
        printf(" still running ");
    }
    printf(" stopped running ");
    close(image);
    
    write(sockfd, "/Users/umer/Library/Developer/Xcode/DerivedData/SPServer-eqhyfarpzcbgiahaqfpsytudoiaj/Build/Products/Debug/c1.png", sizeof("/Users/umer/Library/Developer/Xcode/DerivedData/SPServer-eqhyfarpzcbgiahaqfpsytudoiaj/Build/Products/Debug/cs1.png"));
    close(sockfd);
    
    pthread_exit(NULL);
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
            if(strcmp(token, "addUser") == 0){
                token = strtok(NULL, "$\n");
                if(token != NULL){
                    updateUserBySockId(sockfd, token);
                }
            }else if(strcmp(token, "send") == 0){
                token = strtok(NULL, "$\n");
                if(token != NULL){
                    int fd2 = atoi(token);
                    token = strtok(NULL, "$\n");
                    char buff2[1000];
                    int size = sprintf(buff2, "receive$%d$%s", sockfd, token);
                    ssize_t w1 = write(fd2, buff2, size);
                    if (w1 == -1){
                        perror("write error -1");
                    }else if (w1 == 0){
                        perror("write error 0");
                    }
                }
            }else if(strcmp(token, "showUsers") == 0){
                char buff2[1000];
                int size = sprintf(buff2, "userlist${ \"users\" : [");
                buff2[size] = '\0';
                for (int i = 0; i < USERS; i++) {
                    if(users[i].id == 0){
                        break;
                    }
                    char buff3[500];
                    int qw = sprintf(buff3, "{\"id\" : %d, \"name\" : \"%s\"},", users[i].id, users[i].name);
                    buff3[qw] = '\0';
                    strcat(buff2, buff3);
                }
                char buff4[1000];
                int s = sprintf(buff4, "%s]}", buff2);
                buff2[s] = '\0';
                ssize_t w1 = write(sockfd, buff4, s);
                if (w1 == -1){
                    perror("write error -1");
                }else if (w1 == 0){
                    perror("write error 0");
                }
            }
        }
    }
    //        pthread_exit(NULL);
}


void setup(){
    
    for (int i = 0; i < USERS; i++) {
        users[i] = *(struct User*) malloc(sizeof(struct User));
        users[i].id = 0;
    }
    for (int i = 0; i < CONVERSATIONS; i++) {
        conversations[i] = *(struct Conversation*) malloc(sizeof(struct Conversation));
        conversations[i].id = 0;
    }
}

int userArrayIndex(){
    
    for (int i = 0; i < USERS; i++) {
        if(users[i].id == 0){
            return i;
        }
    }
    return -1;
}

int conversationArrayIndex(){
    
    for (int i = 0; i < USERS; i++) {
        if(conversations[i].id == 0){
            return i;
        }
    }
    return -1;
}

int searchUserBySockId(int fd){
    
    for (int i = 0; i < USERS; i++) {
        if(users[i].id == 0){
            return -1;
        }
        if (users[i].id == fd) {
            return i;
        }
    }
    return -1;
}

int searchConversationsBySockId(int fd){
    
    for (int i = 0; i < CONVERSATIONS; i++) {
        if(conversations[i].id == 0){
            return -1;
        }
        if (conversations[i].id == fd) {
            return i;
        }
    }
    return -1;
}

int makeUserBySockId(int fd){
    
    int index = searchUserBySockId(fd);
    if (index == -1) {
        struct User *user = (struct User*) malloc(sizeof(struct User));
        user -> id = fd;
        int i = userArrayIndex();
        users[i] = *user;
        return 0;
    }
    return -1;
}

int updateUserBySockId(int fd, char * name){
    
    int index = searchUserBySockId(fd);
    if (index == -1) {
        struct User *user = (struct User*) malloc(sizeof(struct User));
        user -> id = fd;
        strcpy(user -> name, name);
        int i = userArrayIndex();
        users[i] = *user;
    }else{
        strcpy(users[index].name, name);
    }
    return 0;
}


void checkHostName(int hostname)
{
    if (hostname == -1)
    {
        perror("gethostname");
        exit(1);
    }
}

// Returns host information corresponding to host name
void checkHostEntry(struct hostent * hostentry)
{
    if (hostentry == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
}

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer)
{
    if (NULL == IPbuffer)
    {
        perror("inet_ntoa");
        exit(1);
    }
}
