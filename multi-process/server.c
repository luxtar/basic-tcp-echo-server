#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MAX_CLIENT 5
#define BUF_SIZE 1024

int client_index = 0;

int main(int argc, char **argv)
{
        if (argc != 2)
        {
                printf("Usage : %s [port]\n", argv[0]);
                exit(0);
        }

        int pid;
        int server_sock, client_sock;

        if ((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        {
                perror("socket error : ");
                exit(0);
        }

        int on = 1;
        if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
                perror("socket option set error : ");
                exit(0);
        }

        struct sockaddr_in server_addr, client_addr;
        int client_addr_size = sizeof(client_addr);
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(atoi(argv[1]));

        if(bind(server_sock , (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
        {
                perror("bind error : ");
                exit(0);
        }

        if(listen(server_sock, 5) < 0 )
        {
                perror("listen error : ");
                exit(0);
        }

        signal(SIGCHLD, SIG_IGN);

        char buf[BUF_SIZE];
        while(1)
        {
                printf("accept...\n");

                client_sock = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
                if (client_sock < 0)
                {
                        perror("Accept error : ");
                        continue;
                }

                 if(client_index == MAX_CLIENT) {
                        printf("client accept full(max client count : %d)\n", MAX_CLIENT);
                        close(client_sock);
                        continue;
                }

                client_index++;

                printf("client accepted(Addr: %s, Port: %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                pid = fork();
                if (pid == 0)
                {
                        printf("pid:%d\n", pid);

                        while(1)
                        {
                                memset(buf, 0x00, BUF_SIZE);
                                if (read(client_sock, buf, sizeof(buf)) <= 0)
                                {
                                        printf("Client %d close\n", client_sock, buf);
                                        client_index--;
                                        close(client_sock);
                                        break;
                                }

                                printf("read : %s\n", buf);

                                if(write(client_sock, buf, sizeof(buf)) <= 0) {
                                        printf("Client %d close\n", client_sock, buf);
                                        client_index--;
                                        close(client_sock);
                                        break;
                                }

                                printf("write : %s\n", buf);
                        }
                }
                if (pid == -1)
                {
                        close(client_sock);
                        perror("fork error : ");
                        continue;
                }
        }

        close(server_sock);

        return 0;
}

