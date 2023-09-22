#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

typedef struct socketPthreadInfo {
    int act;
    struct sockaddr_in cliaddr;
} CFINFO;

void *threadFunction(void *args);

int main() {

    struct sockaddr_in servaddr, cliaddr;
    socklen_t client_address_length = sizeof(cliaddr);
    int link_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (link_socket == -1) {
        perror("Service Socket error");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int bindFD = bind(link_socket, (struct sockaddr *) &servaddr, sizeof(servaddr));

    if (bindFD == -1) {
        perror("Bind error");
        return -1;
    }

    listen(link_socket, 128);

    char buff[2048];
    memset(buff, 0, 2048);

    while (1) {

        pthread_t pid;

        int acceptFd = accept(link_socket, (struct sockaddr *) &cliaddr, &client_address_length);
        if (acceptFd == -1) {
            perror("Accept error");
            break;
        }

        CFINFO cfinfo;
        cfinfo.act = acceptFd;
        cfinfo.cliaddr = cliaddr;
        pthread_create(&pid, NULL, threadFunction, (void *) &cfinfo);

    }

    return 0;
}


void *threadFunction(void *args) {
    CFINFO *cfinfo = (CFINFO *) args;
    int accepfd = cfinfo->act;
    struct sockaddr_in cliaddr = cfinfo->cliaddr;


    char ip[16];
    size_t port;
    inet_ntop(AF_INET, &cliaddr.sin_addr, ip, sizeof(ip));
    port = ntohs(cliaddr.sin_port);



    char buff[1024];
    memset(buff, 0, 1024);
    while (1) {
        unsigned int n = read(accepfd, buff, 1023);
        if (n == 0) {
            close(accepfd);
            printf("[%s:%zu]client close .....\n",ip,port);
            break;
        }
        write(STDOUT_FILENO, buff, n);
    }
    return NULL;
}