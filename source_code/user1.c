#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "msocket.h"
#define FILE_NAME "sample134.txt"
#define SRC_IP "127.0.0.1"
#define DEST_IP "127.0.0.1"
#define SRC_PORT 2000
#define DEST_PORT 3000

int main(int argc, char *argv[])
{
    int sockfd = m_socket(AF_INET, SOCK_MTP, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(0);
    }
    struct sockaddr_in user1addr;
    struct sockaddr_in user2addr;
    user1addr.sin_family = AF_INET;
    user2addr.sin_family = AF_INET;
    user1addr.sin_addr.s_addr = inet_addr(SRC_IP);
    user2addr.sin_addr.s_addr = inet_addr(DEST_IP);
    user1addr.sin_port = htons(SRC_PORT);
    user2addr.sin_port = htons(DEST_PORT);
    if (m_bind(sockfd, (struct sockaddr *)&user1addr, (struct sockaddr *)&user2addr) < 0)
    {
        perror("Socket Binding failed");
        exit(0);
    }
    int fd = open(FILE_NAME, O_RDONLY);
    char data[1024];
    int cnt = 0;
    while (1)
    {
        for (int i = 0; i < 1024; i++)
        {
            data[i] = '\0';
        }
        int bytes_recv = read(fd, data, 1024);
        int n = 0;
        while (n <= 0)
        {
            n = m_sendto(sockfd, data, 1024, 0, (struct sockaddr *)&user2addr, sizeof(user2addr));
        }
        printf("Data chunk - %d sent successfully\n", ++cnt);
        if (bytes_recv < 1024)
            break;
    }
    close(fd);
}