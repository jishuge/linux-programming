#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


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

        int acceptFd = accept(link_socket, (struct sockaddr *) &cliaddr, &client_address_length);
        if (acceptFd == -1) {
            perror("Accept error");
            break;
        }

        int pid = fork();
        if (pid == -1) {
            perror("fork error");
            break;
        } else if (pid == 0) {
            char ip[16];
            memset(ip, 0, 16);

            inet_ntop(AF_INET, &cliaddr.sin_addr, ip, sizeof(ip));

            printf("client %s:%d connect\n", ip, ntohs(cliaddr.sin_port));
            while (1) {
                close(link_socket);
                socklen_t n = read(acceptFd, buff, sizeof(buff) - 1);
                if (n == 0) {
                    printf("client %s:%d close\n", ip, ntohs(cliaddr.sin_port));
                    close(acceptFd);
                    exit(-1);
                }
                if (buff[n - 1] == '\n') {
                    buff[n - 1] = '\0';
                }
                printf("[%s:%d] %s\n", ip, ntohs(cliaddr.sin_port), buff);
            }
        } else {
            close(acceptFd);
        }
    }


    return 0;
}
