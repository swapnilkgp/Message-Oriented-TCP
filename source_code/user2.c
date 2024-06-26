#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include "msocket.h"
#define FILE_NAME "user2.txt"
#define SRC_IP "127.0.0.1"
#define DEST_IP "127.0.0.1"
#define SRC_PORT 3000
#define DEST_PORT 2000

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
    int ret = m_bind(sockfd, (struct sockaddr *)&user1addr, (struct sockaddr *)&user2addr);
    if (ret < 0)
    {
        perror("Socket Binding failed");
        exit(0);
    }
    int file;
    char buffer[1024];
    ssize_t bytes_written;
    int cnt = 0;
    file = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    while (1)
    {
        if (file == -1)
        {
            printf("Error opening file!\n");
            return 1;
        }
        int len = 0;
        while (len <= 0)
        {
            int n;
            len = m_recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&user1addr, &n);
        }
        printf("Data chunk - %d received successfully\n", ++cnt);
        int i;
        for (i = 0; i < len; i++)
        {
            if (buffer[i] == '\0')
            {
                break;
            }
        }

        bytes_written = write(file, buffer, i);
        if (bytes_written == -1)
        {
            printf("Error writing to file!\n");
            close(file);
            return 1;
        }

        if (i < sizeof(buffer))
        {
            break;
        }
    }
    close(file);
}