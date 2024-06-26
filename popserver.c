#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_LEN 100
#define CRLF "\r\n"
char MESSAGE[100];
int lastread = 0;
char sendBUFF[100];
char recvBUFF[100];
int total_recv = 0;
int total_read = 0;
int ptr = 0;
const char *domain_name = "<iitkgp.edu>";

int EDIT(char *username, int numMails, int *deleted)
{
    char *path1 = (char *)malloc(100 * sizeof(char));
    strcpy(path1, username);
    strcat(path1, "/mymailbox");
    char *path2 = (char *)malloc(100 * sizeof(char));
    strcpy(path2, username);
    strcat(path2, "/temp");
    FILE *file2 = fopen(path2, "w");
    FILE *file1 = fopen(path1, "r");
    if (file2 == NULL)
    {
        perror("Error opening file temp");
        return 1;
    }
    if (file1 == NULL)
    {
        perror("Error opening file mailbox");
        return 1;
    }
    int currentMail = 0;
    char line[100];

    // Read each line from the input file
    while (fgets(line, sizeof(line), file1) != NULL)
    {
        if (line[0] == '.' && line[1] == '\n')
        {
            // End of a mail
            if (currentMail < numMails && deleted[currentMail] == 0)
            {
                // Write the mail to the output file if it should not be deleted
                fputs(".\n", file2);
            }
            currentMail++;
        }
        else if (currentMail < numMails && deleted[currentMail] == 0)
        {
            // Write the line to the output file if it should not be deleted
            fputs(line, file2);
        }
    }
    fclose(file2);
    fclose(file1);
    remove(path1);
    int flag = rename(path2, path1);
    return flag;
}

bool authenticateUser(const char *username, const char *password)
{
    // Open the user.txt file for reading
    FILE *file = fopen("user.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return false;
    }

    char line[MAX_LEN];
    // printf("%s %s\n", username, password);
    char token[MAX_LEN];
    strcpy(token, username);
    strcat(token, " ");
    strcat(token, password);
    // printf("%s", token);
    // Read each line from the file
    while (fgets(line, MAX_LEN, file) != NULL)
    {
        // printf("line : %s", line);
        // printf("%d %d %d %c\n", strcmp(line, token), strlen(line), strlen(token), line[strlen(line) - 1]);
        line[strlen(line) - 2] = '\0';
        if (strcmp(line, token) == 0)
        {
            fclose(file);
            return true;
        }
    }
    // Close the file if username and password are not found
    fclose(file);
    return false;
}

int sendMSG(int socket_fd, char *buffer, size_t size)
{
    int total_size = size + strlen(CRLF);
    char *complete_buffer = (char *)malloc(total_size + 1);
    if (complete_buffer == NULL)
    {
        perror("Memory allocation failed");
        return -1;
    }
    strcpy(complete_buffer, "");
    strcat(complete_buffer, buffer);
    strcat(complete_buffer + size, CRLF);
    printf("S : %s\n", complete_buffer);
    int bytes_sent = send(socket_fd, complete_buffer, total_size, 0);
    if (bytes_sent < 0)
    {
        perror("Error in sending buffer\n");
        free(complete_buffer);
        return -1;
    }
    free(complete_buffer);
    return bytes_sent;
}

int recvMSG(int socket_fd, char *buffer, char *MESSAGE)
{
    int msgptr = 0;
    int maystop = 0;
    while (1)
    {
        while (total_read < total_recv)
        {
            if (buffer[ptr] == '\r' && maystop == 0)
            {
                maystop = 1;
                ptr++;
                total_read++;
            }
            else if (maystop == 1 && buffer[ptr] == '\n')
            {
                total_read++;
                ptr++;
                MESSAGE[msgptr] = '\0';
                printf("C : %s\n", MESSAGE);
                return msgptr;
            }
            else
            {
                if (maystop == 0)
                {
                    MESSAGE[msgptr] = buffer[ptr];
                    msgptr++;
                    total_read++;
                    ptr++;
                }
                else
                {
                    MESSAGE[msgptr] = '\r';
                    msgptr++;
                    maystop = 0;
                }
            }
        }
        total_read = 0;
        total_recv = recv(socket_fd, buffer, 100, 0);
        ptr = 0;
        if (total_recv < 0)
        {
            printf("Message recieving error\n");
            return -1;
        }
    }
}

void countMailsAndSizes(int *numMails, int mailSizes[], char *username)
{
    char *path = (char *)malloc(100 * sizeof(char));
    strcpy(path, username);
    strcat(path, "/mymailbox");
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }
    int numMailsCount = 0;
    int mailSize = 0;

    char line[MAX_LEN];

    while (fgets(line, MAX_LEN, file) != NULL)
    {
        if (line[0] == '.' && line[1] == '\n')
        {
            if (mailSize > 0)
            {
                mailSizes[numMailsCount] = mailSize;
                mailSize = 0;
                numMailsCount++;
            }
        }
        else
        {
            mailSize += strlen(line);
        }
    }

    fclose(file);
    *numMails = numMailsCount;
}

void RETRSEND(int newserver_socket, int num, char *username)
{
    char *path = (char *)malloc(100 * sizeof(char));
    strcpy(path, username);
    strcat(path, "/mymailbox");
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }
    int currentMail = 0;
    char line[100];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (line[0] == '.' && line[1] == '\n' && currentMail == num)
        {
            break;
        }
        else if (currentMail == num)
        {
            line[strlen(line) - 1] = '\0';
            strcpy(sendBUFF, line);
            // printf("line : %s\n", sendBUFF);
            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
        }
        else if (line[0] == '.' && line[1] == '\n')
        {
            currentMail++;
        }
    }
    fclose(file);
}

int main(int argc, char const *argv[])
{
    int server_socket, newserver_socket;
    struct sockaddr_in server_address, client_address;
    int client_len;
    int my_port = atoi(argv[1]);
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in server socket creation !!\n");
        exit(0);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(my_port);
    if ((bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address))) < 0)
    {
        perror("Unable to bind local address to server socket !!\n");
        exit(0);
    }

    listen(server_socket, 5);
    printf("Server listening on Port %d..\n", my_port);
    // LOOP
    while (1)
    {
        client_len = sizeof(client_address);
        if ((newserver_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len)) < 0)
        {
            perror("Accept Error !!\n");
            exit(0);
        }
        int child = fork();
        if (child == 0)
        {
            close(server_socket);
            //  +OK POP3 server ready
            strcpy(sendBUFF, "+OK POP3 server ready");
            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
            int isAuth = 0;
            const char delim[] = " ";
            char *username = (char *)malloc(100 * sizeof(char));
            char *password = (char *)malloc(100 * sizeof(char));
            int numMails;
            int *numptr = (int *)malloc(sizeof(int));
            int mailSizes[100];
            int *deleted = (int *)calloc(100, sizeof(int));
            while (1)
            {
                recvMSG(newserver_socket, recvBUFF, MESSAGE);
                char *firstword = strtok(MESSAGE, delim);
                if (strcmp(firstword, "USER") == 0)
                {
                    if (isAuth == 1)
                    {
                        strcpy(sendBUFF, "-ERR Authentication already done");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                    else
                    {
                        char *USERNAME = strtok(NULL, delim);
                        strcpy(username, USERNAME);
                        // printf("%s\n", username);
                        strcpy(sendBUFF, "+OK Please enter a password");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                }

                else if (strcmp(firstword, "PASS") == 0)
                {
                    if (isAuth)
                    {
                        strcpy(sendBUFF, "-ERR Authentication already done");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                    else
                    {
                        char *PASSWORD = strtok(NULL, delim);
                        strcpy(password, PASSWORD);
                        // printf("%s %s---\n", username, password);
                        bool flag = authenticateUser(username, password);
                        if (flag == true)
                        {
                            isAuth = 1;
                            countMailsAndSizes(numptr, mailSizes, username);
                            strcpy(sendBUFF, "+OK valid logon");
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                            numMails = *numptr;
                        }
                        else
                        {
                            strcpy(sendBUFF, "-ERR Authentication failed");
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                        }
                    }
                }

                else if (strcmp(firstword, "STAT") == 0)
                {
                    if (isAuth)
                    {
                        int totsize = 0;
                        int totmails = 0;
                        for (int i = 0; i < numMails; i++)
                        {
                            if (deleted[i] == 0)
                            {
                                totmails++;
                                totsize += mailSizes[i];
                            }
                        }
                        strcpy(sendBUFF, "");
                        sprintf(sendBUFF, "+OK %d %d", totmails, totsize);
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                    else
                    {
                        strcpy(sendBUFF, "-ERR NO authentication done");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                }

                else if (strcmp(firstword, "DELE") == 0)
                {
                    if (isAuth)
                    {
                        bool isdel = false;
                        char *delchar = strtok(NULL, delim);
                        int delIDX = atoi(delchar);
                        if (delIDX <= 0 || delIDX > numMails)
                        {
                            strcpy(sendBUFF, "-ERR invalid index");
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                            continue;
                        }
                        delIDX--;
                        if (deleted[delIDX] == 1)
                        {
                            strcpy(sendBUFF, "-ERR message already deleted");
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                        }
                        else
                        {
                            deleted[delIDX] = 1;
                            strcpy(sendBUFF, "+OK message deleted");
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                        }
                    }
                    else
                    {
                        strcpy(sendBUFF, "-ERR NO authentication done");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                }

                else if (strcmp(firstword, "RSET") == 0)
                {
                    if (isAuth)
                    {
                        int totnum = 0;
                        int totsize = 0;
                        for (int i = 0; i < numMails; i++)
                        {
                            if (deleted[i] == 1)
                            {
                                deleted[i] = 0;
                                totnum++;
                                totsize += mailSizes[i];
                            }
                        }
                        sprintf(sendBUFF, "+OK maildrop has %d messages (%d octets)", totnum, totsize);
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                    else
                    {
                        strcpy(sendBUFF, "-ERR NO authentication done");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                }

                else if (strcmp(firstword, "NOOP") == 0)
                {
                    strcpy(sendBUFF, "+OK");
                    sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                }

                else if (strcmp(firstword, "LIST") == 0)
                {
                    if (isAuth)
                    {
                        char *next = strtok(NULL, delim);
                        if (next == NULL)
                        {
                            int totsize = 0;
                            int totmails = 0;
                            for (int i = 0; i < numMails; i++)
                            {
                                if (deleted[i] == 0)
                                {
                                    totmails++;
                                    totsize += mailSizes[i];
                                }
                            }
                            sprintf(sendBUFF, "+OK %d messages (%d octets)", totmails, totsize);
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));

                            int num = 0;
                            for (int i = 0; i < numMails; i++)
                            {
                                if (deleted[i] == 0)
                                {
                                    sprintf(sendBUFF, "%d %d", i + 1, mailSizes[i]);
                                    sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                                }
                            }

                            strcpy(sendBUFF, ".");
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                        }
                        else
                        {
                            int num = atoi(next);
                            if (num > numMails || num <= 0)
                            {
                                strcpy(sendBUFF, "-ERR invalid index");
                                sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                                continue;
                            }
                            num--;
                            if (deleted[num] == 1)
                            {
                                strcpy(sendBUFF, "-ERR message already deleted");
                                sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                            }
                            else
                            {
                                sprintf(sendBUFF, "+OK %d %d", num + 1, mailSizes[num]);
                                sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                            }
                        }
                    }
                    else
                    {
                        strcpy(sendBUFF, "-ERR NO authentication done");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                }

                else if (strcmp(firstword, "RETR") == 0)
                {
                    if (isAuth)
                    {
                        char *numchar = strtok(NULL, delim);
                        int num = atoi(numchar);
                        if (num > numMails || num <= 0)
                        {
                            strcpy(sendBUFF, "-ERR invalid index");
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                            continue;
                        }
                        num--;
                        if (deleted[num] == 1)
                        {
                            strcpy(sendBUFF, "-ERR message already deleted");
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                        }
                        else
                        {
                            sprintf(sendBUFF, "+OK %d octets follow", mailSizes[num]);
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                            RETRSEND(newserver_socket, num, username);
                            strcpy(sendBUFF, ".");
                            sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                        }
                    }
                    else
                    {
                        strcpy(sendBUFF, "-ERR NO authentication done");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                }

                else if (strcmp(firstword, "QUIT") == 0)
                {
                    int flag = 1;
                    if (isAuth)
                    {
                        flag = EDIT(username, numMails, deleted);
                    }
                    if (flag == 0 || isAuth == 0)
                    {
                        strcpy(sendBUFF, "+OK Bye-bye");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                    else
                    {
                        strcpy(sendBUFF, "-ERR no deletion before session ends");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                    }
                    break;
                }

                else
                {
                    strcpy(sendBUFF, "-ERR invalid command");
                    sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                }
            }
            close(newserver_socket);
            free(username);
            free(password);
            free(numptr);
            free(deleted);
            exit(0);
        }
        else
        {
            close(newserver_socket);
        }
    }
    close(server_socket);
}