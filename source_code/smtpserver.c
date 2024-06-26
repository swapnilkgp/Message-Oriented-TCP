#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_LEN 5000
#define CRLF "\r\n"
char MESSAGE[100];
int lastread = 0;
char sendBUFF[100];
char recvBUFF[100];
int total_recv = 0;
int total_read = 0;
int ptr = 0;
const char *domain_name = "<iitkgp.edu>";

char *extractFirstWord(const char *str)
{
    int i = 0;
    char *result = (char *)malloc(5 * sizeof(char));
    // Skip leading spaces and tabs
    while (str[i] == ' ' || str[i] == '\t')
    {
        i++;
    }

    // Copy characters into result until a space, tab, or null terminator is encountered
    int j = 0;
    while (str[i] != ' ' && str[i] != '\t' && str[i] != '\0')
    {
        result[j] = str[i];
        i++;
        j++;
    }

    // Null-terminate the result string
    result[j] = '\0';
    return result;
}

int isFolderExists(const char *folderPath)
{
    struct stat st;
    if (stat(folderPath, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            // The path exists and is a directory
            return 1;
        }
        else
        {
            // The path exists but is not a directory
            return 0;
        }
    }
    else
    {
        // The path does not exist
        return 0;
    }
}

char *extractDomain(const char *input)
{
    const char *start = strchr(input, '<'); // Find the first occurrence of '<'
    if (start == NULL)
    {
        return NULL; // If '<' not found, return NULL
    }

    start++; // Move the pointer to the character after '<'

    const char *end = strchr(start, '>'); // Find the first occurrence of '>'
    if (end == NULL)
    {
        return NULL; // If '>' not found, return NULL
    }

    size_t length = end - start; // Calculate the length of the substring

    char *result = (char *)malloc(length + 1); // Allocate memory for the substring (+1 for null terminator)
    if (result == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    strncpy(result, start, length); // Copy the substring to the result buffer
    result[length] = '\0';          // Null-terminate the string

    return result;
}

char *extractTime()
{
    char *timestr = (char *)malloc(30 * sizeof(char));
    time_t current_time;
    struct tm *local_time;

    // Get the current time
    current_time = time(NULL);

    // Convert the current time to local time
    local_time = localtime(&current_time);

    // Extract date, hour, and minute from the local_time structure
    int year = local_time->tm_year + 1900; // Years since 1900
    int month = local_time->tm_mon + 1;    // Months since January (0-11)
    int day = local_time->tm_mday;         // Day of the month (1-31)
    int hour = local_time->tm_hour;        // Hours since midnight (0-23)
    int minute = local_time->tm_min;       // Minutes after the hour (0-59)

    // Print the date, hour, and minute
    sprintf(timestr, "Received: %04d-%02d-%02d : %02d:%02d\n", year, month, day, hour, minute);
    return timestr;
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

int manageMAILBOX(int socket_fd, char *buffer, FILE *file, char *timestr)
{
    // int msgptr = 0;
    // int maystop = 0;
    // int cnt = 0;
    // int isTime = 0;
    // while (1)
    // {
    //     while (total_read < total_recv)
    //     {
    //         if (cnt == 3 && isTime == 0)
    //         {
    //             isTime = 1;
    //             fputs(timestr, file);
    //         }
    //         if (buffer[ptr] == '\r' && maystop == 0)
    //         {
    //             maystop = 1;
    //             ptr++;
    //             total_read++;
    //         }
    //         else if (maystop == 1 && buffer[ptr] == '\n')
    //         {
    //             total_read++;
    //             ptr++;
    //             return msgptr;
    //         }
    //         else
    //         {
    //             if (maystop == 0)
    //             {
    //                 fputc(buffer[ptr], file);
    //                 if (buffer[ptr] == '\n')
    //                 {
    //                     cnt++;
    //                 }
    //                 msgptr++;
    //                 total_read++;
    //                 ptr++;
    //             }
    //             else
    //             {
    //                 fputc('\r', file);
    //                 msgptr++;s
    //                 maystop = 0;
    //             }
    //         }
    //     }
    //     total_read = 0;
    //     total_recv = recv(socket_fd, buffer, 100, 0);
    //     ptr = 0;
    //     if (total_recv < 0)
    //     {
    //         printf("Message recieving error\n");
    //         return -1;
    //     }
    // }
    int cnt = 0;
    while (1)
    {
        recvMSG(socket_fd, buffer, MESSAGE);
        if (strcmp(MESSAGE, ".") == 0)
        {
            fputs(MESSAGE, file);
            fputc('\n', file);
            break;
        }
        else
        {
            fputs(MESSAGE, file);
            fputc('\n', file);
            cnt++;
            if (cnt == 3)
            {
                fputs(timestr, file);
            }
        }
    }
    return 0;
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
            //  220 <iitkgp.edu> Service ready
            int len = 0;
            strcpy(sendBUFF, "220 ");
            strcat(sendBUFF, domain_name);
            strcat(sendBUFF, " Service ready");
            len += strlen("220 ") + strlen(domain_name) + strlen(" Service ready");
            sendMSG(newserver_socket, sendBUFF, len);

            while (1)
            {
                recvMSG(newserver_socket, recvBUFF, MESSAGE);
                char *firstword = extractFirstWord(MESSAGE);
                char *user;
                if (strcmp(firstword, "HELLO") == 0)
                {
                    //  250 OK Hello iitkgp.edu
                    len = 0;
                    strcpy(sendBUFF, "250 OK HELLO ");
                    strcat(sendBUFF, domain_name);
                    len += strlen("250 OK HELLO ") + strlen(domain_name);
                    sendMSG(newserver_socket, sendBUFF, len);
                }

                else if (strcmp(firstword, "MAIL") == 0)
                {
                    //  250 <ag@iitkgp.edu>... Sender ok
                    char *sender = extractDomain(MESSAGE);
                    // printf("%s\n",sender);
                    strcpy(sendBUFF, "250 < ");
                    strcat(sendBUFF, sender);
                    strcat(sendBUFF, ">... Sender OK");
                    // printf("IN MAIL %s\n",sendBUFF);
                    sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                }

                else if (strcmp(firstword, "RCPT") == 0)
                {
                    // 250 root... Recipient ok
                    user = extractDomain(MESSAGE);
                    int isPresent = isFolderExists(user);
                    if (isPresent == 0)
                    {
                        strcpy(sendBUFF, "550 No such user");
                        sendMSG(newserver_socket, sendBUFF, strlen(sendBUFF));
                        printf("No such username : Breaking connection\n");
                        break;
                    }
                    strcpy(sendBUFF, "250 root... Recipient ok");
                    len = strlen("250 root... Recipient ok");
                    sendMSG(newserver_socket, sendBUFF, len);
                }

                else if (strcmp(firstword, "DATA") == 0)
                {
                    // 354 Enter mail, end with "." on a line by itself
                    strcpy(sendBUFF, "354 Enter mail, end with \".\" on a line by itself");
                    len = strlen("354 Enter mail, end with \".\" on a line by itself");
                    sendMSG(newserver_socket, sendBUFF, len);

                    // Processing the Email
                    strcat(user, "/mymailbox");
                    FILE *file = fopen(user, "a");
                    if (file == NULL)
                    {
                        perror("Error opening file");
                        return 1;
                    }
                    char *timestr = extractTime();
                    manageMAILBOX(newserver_socket, recvBUFF, file, timestr);
                    fclose(file);
                    free(timestr);
                    free(user);

                    // 250 OK Message accepted for delivery
                    strcpy(sendBUFF, "250 OK Message accepted for delivery");
                    len = strlen("250 OK Message accepted for delivery");
                    sendMSG(newserver_socket, sendBUFF, len);
                }

                else if (strcmp(firstword, "QUIT") == 0)
                {
                    // 221 iitkgp.edu closing connection
                    strcpy(sendBUFF, "221 ");
                    strcat(sendBUFF, domain_name);
                    strcat(sendBUFF, " closing connection");
                    len = strlen(domain_name) + strlen("221 ") + strlen(" closing connection");
                    sendMSG(newserver_socket, sendBUFF, len);
                    break;
                }
                else
                {
                }
            }
            close(newserver_socket);
            exit(0);
        }
        else
        {
            close(newserver_socket);
        }
    }
    close(server_socket);
}
