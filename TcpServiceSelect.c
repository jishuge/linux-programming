#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>


/**
 * select 原理：
 * 先把要监听的文件描述符添加到读的集合（readSet）中去，然后交给select，
 * 如果有文件描述符发生了变化，那么就readSet这个集合里面其他没有发生变化的文件描述符就被清空，保留发生变化的文件描述符
 * 所以在添加之前要备份readSet。
 *
 */

#define  PORT 8080




int main() {

    int linkFd = socket(AF_INET, SOCK_STREAM, 0);
    if (linkFd == -1) {
        perror("socket error");
        return -1;
    }

    struct sockaddr_in serviceAddres;
    serviceAddres.sin_port = htons(PORT);
    serviceAddres.sin_family = AF_INET;
    serviceAddres.sin_addr.s_addr = htonl(INADDR_ANY);


    /**
     * IP端口复用
     */
    int opt = 1;
    setsockopt(linkFd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof(opt));


    /**
     * 进行帐号哦啊绑定
     */
    unsigned bindFd = bind(linkFd, (struct sockaddr *) &serviceAddres, sizeof(serviceAddres));


    if (bindFd == -1) {
        perror("bind error");
        return -1;
    }

    listen(linkFd, 128);


    int maxFd = linkFd;
    fd_set readOldSet, readNewSet;

    /**
     * 清空集合
     */
    FD_ZERO(&readOldSet);
    FD_ZERO(&readNewSet);

    printf("把连接监听的文件字符串添加到集合中去：%d\n",linkFd);
    FD_SET(linkFd, &readOldSet);


    while (1) {

        readNewSet = readOldSet;
        int n = select(maxFd + 1, &readNewSet, NULL, NULL, NULL);

        if (n < 0) {
            break;
        } else if (n == 0) {
            continue;
        } else {

            /**
             * 监听到了文件描述符的变化，
             * 1，linkFd监听到了
             * 2,客户端fd监听到了。
             */
            if (FD_ISSET(linkFd, &readNewSet)) {


                struct sockaddr_in clientAddress;
                socklen_t len = sizeof(clientAddress);
                char ip[16] = "";
                /**
                 * linkFd，有的新的客户端链接上了。
                 */
                int cfd = accept(linkFd, (struct sockaddr *) &clientAddress, &len);
                printf("[%s:%d] client connnect....\n", inet_ntop(AF_INET, &clientAddress.sin_addr.s_addr, ip, 16),
                       ntohs(clientAddress.sin_port));

                /**
                 * 更新监听集合。
                 */
                printf("把客户端添加到集合中去：%d\n", cfd);
                FD_SET(cfd, &readOldSet);

                /**
                 * 更新max最大的文件描述符
                 */
                if (cfd > maxFd) {
                    maxFd = cfd;
                }

                printf("现在最大的文件描述符：%d\n", maxFd);

                /**
                 * 如果只有一个的，那么就是linkFd变化,没有客户端，重新开始监听。
                 */
                if (--n == 0) {
                    continue;
                }

            }

            /**
             * 非linkfd变化，对 其进行读取。
             */

            for (int i = linkFd + 1; i <= maxFd; ++i) {

                if (FD_ISSET(i, &readNewSet)) {
                    char buff[1500] = "";
                    long ret = read(i, buff, sizeof(buff));
                    if (ret <= 0) {
                        close(i);
                        FD_CLR(i, &readOldSet);
                        printf("关闭了链接\n");
                    } else {
                        //输出到屏幕
                        write(STDOUT_FILENO,buff,ret);
                        //printf("message:%s\n", buff);
                    }
                }
            }

        }


    }


}
