#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

char SERVER_IP[20];
int PORT1;
int PORT2;
#define MAX_BUFFER_SIZE 100
#define LINE_NUM 50
int total_recv = 0;
int total_read = 0;
int ptr = 0;
char buffer1[MAX_BUFFER_SIZE];
char buffer2[MAX_BUFFER_SIZE];
char buffer[MAX_BUFFER_SIZE];
char username[MAX_BUFFER_SIZE];
char password[MAX_BUFFER_SIZE];
const char delim[] = " ";
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

int Rcv(char *buffer, char *MESSAGE, int socket_fd)
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

void Snd(char buffer2[], int clientSocket)
{
    // printf("C : %s\n", buffer2);
    strcat(buffer2, "\r\n");
    send(clientSocket, buffer2, strlen(buffer2), 0);
}

char *extractInfo(char **message)
{
    char *result = (char *)malloc(4 * MAX_BUFFER_SIZE * sizeof(char));

    char senderEmail[MAX_BUFFER_SIZE];
    char receivedTimestamp[MAX_BUFFER_SIZE];
    char subject[MAX_BUFFER_SIZE];

    // Parse the message to extract sender's email, received timestamp, and subject
    sscanf(message[0], "From: %s", senderEmail);
    sscanf(message[1], "To: %*s"); // Ignore "To:" field
    sscanf(message[2], "Subject: %[^\n]", subject);
    sscanf(message[3], "Received: %[^\n]", receivedTimestamp);

    // Concatenate sender's email, received timestamp, and subject
    snprintf(result, 4 * MAX_BUFFER_SIZE, "%s   %s   %s", senderEmail, receivedTimestamp, subject);

    return result;
}

void USER(int clientSocket)
{
    strcpy(buffer2, "USER ");
    strcat(buffer2, username);
    Snd(buffer2, clientSocket);
    Rcv(buffer, buffer1, clientSocket);
}

int PASS(int clientSocket)
{
    strcpy(buffer2, "PASS ");
    strcat(buffer2, password);
    Snd(buffer2, clientSocket);
    Rcv(buffer, buffer1, clientSocket);
    if (buffer1[0] == '+')
        return (1);
    else
        return (0);
}

void QUIT(int clientSocket)
{
    strcpy(buffer2, "QUIT");
    Snd(buffer2, clientSocket);
    Rcv(buffer, buffer1, clientSocket);
}

char *STAT(int clientSocket)
{
    strcpy(buffer2, "STAT");
    Snd(buffer2, clientSocket);
    Rcv(buffer, buffer1, clientSocket);
    char *buf = (char *)malloc(100 * sizeof(char));
    strcpy(buf, buffer1);
    if (buf[0] == '-')
        return (NULL);
    else
        return (buf);
}

char **LIST(int clientSocket)
{
    strcpy(buffer2, "LIST");
    Snd(buffer2, clientSocket);
    Rcv(buffer, buffer1, clientSocket);
    if (buffer1[0] == '-')
        return (NULL);
    else
    {
        char **msg = (char **)malloc(100 * sizeof(char *));
        int i = 0;
        while (1)
        {
            Rcv(buffer, buffer1, clientSocket);
            msg[i] = (char *)malloc(100 * sizeof(char));
            strcpy(msg[i], buffer1);
            i++;
            if (strcmp(buffer1, ".") == 0)
                break;
        }
        return (msg);
    }
}

char **RETR(int i, int clientSocket)
{
    strcpy(buffer2, "RETR ");
    char idx[10];
    sprintf(idx, "%d", i);
    strcat(buffer2, idx);
    Snd(buffer2, clientSocket);
    Rcv(buffer, buffer1, clientSocket);
    if (buffer1[0] == '-')
        return (NULL);
    else
    {
        char **msg = (char **)malloc(50 * sizeof(char *));
        int i = 0;
        while (1)
        {
            Rcv(buffer, buffer1, clientSocket);
            msg[i] = (char *)malloc(100 * sizeof(char));
            strcpy(msg[i], buffer1);
            i++;
            if (strcmp(buffer1, ".") == 0)
                break;
        }
        return (msg);
    }
}

int DELE(int i, int clientSocket)
{
    strcpy(buffer2, "DELE ");
    char idx[10];
    sprintf(idx, "%d", i);
    strcat(buffer2, idx);
    Snd(buffer2, clientSocket);
    Rcv(buffer, buffer1, clientSocket);
    if (buffer1[0] == '+')
        return (1);
    else
        return (0);
}

void NOOP(int clientSocket)
{
    strcpy(buffer2, "NOOP");
    Snd(buffer2, clientSocket);
    Rcv(buffer, buffer1, clientSocket);
}

int RSET(int clientSocket)
{
    strcpy(buffer2, "RSET");
    Snd(buffer2, clientSocket);
    Rcv(buffer, buffer1, clientSocket);
    if (buffer1[0] == '+')
        return (1);
    else
        return (0);
}

int main(int argc, char const *argv[])
{
    if (argc < 4)
    {
        printf("Enter proper command line arguments\n");
        return 0;
    }

    strcpy(SERVER_IP, argv[1]);
    PORT1 = atoi(argv[2]);
    PORT2 = atoi(argv[3]);

    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    while (1)
    {
        printf("\n\n");
        printf("Press 1 to Manage Mail\n");
        printf("Press 2 to Send Mail\n");
        printf("Press 3 to QUIT\n");
        printf("Enter input : ");
        int input;
        scanf("%d", &input);
        getchar();
        if (input == 1)
        {
            struct sockaddr_in serverAddress;
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
            serverAddress.sin_port = htons(PORT2);
            int clientSocket = 0;
            clientSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (clientSocket == -1)
            {
                perror("Error creating socket");
                exit(EXIT_FAILURE);
            }
            if ((connect(clientSocket, (struct sockaddr *)&serverAddress,
                         sizeof(serverAddress))) < 0)
            {
                perror("Unable to connect to server\n");
                exit(0);
            }
            Rcv(buffer, buffer1, clientSocket);
            USER(clientSocket);
            int pass = PASS(clientSocket);
            if (pass != 1)
            {
                printf("Authentication failed\n");
                continue;
            }
            while (1)
            {
                printf("\n");
                int i = 0;
                char **msg2 = LIST(clientSocket);
                while (strcmp(msg2[i], ".") != 0)
                {
                    char *firstword = strtok(msg2[i], delim);
                    int n = atoi(firstword);
                    char **msg1 = RETR(n, clientSocket);
                    char *result = extractInfo(msg1);
                    printf("%d  %s\n", n, result);
                    i++;
                }
                // free(firstword);
                // free(secword);
                printf("Enter mail no. to see: ");
                int idx;
                scanf("%d", &idx);
                if (idx == -1)
                    break;
                char **msg = RETR(idx, clientSocket);
                while (msg == NULL)
                {
                    printf("Mail no.out of range, give again: ");
                    scanf("%d", &idx);
                    if (idx == -1)
                        break;
                    msg = RETR(idx, clientSocket);
                }
                if (idx == -1)
                    break;
                i = 0;
                while (strcmp(msg[i], ".") != 0)
                {
                    printf("%s\n", msg[i]);
                    // free(msg[i]);
                    i++;
                }
                // free(msg[i]);
                char c;
                getchar();
                printf("Enter d to delete : (any character otherwise) : ");
                c = getchar();
                if (c == 'd')
                    DELE(idx, clientSocket);
            }
            QUIT(clientSocket);
            close(clientSocket);
            continue;
        }
        else if (input == 2)
        {
            struct sockaddr_in serverAddress;
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
            serverAddress.sin_port = htons(PORT1);
            char *line[51];
            char sender[MAX_BUFFER_SIZE];
            char receiver[MAX_BUFFER_SIZE];
            const char delim[] = " ";
            int l = 0;
            printf("Enter the mail:\n");
            while (1)
            {
                fgets(buffer1, sizeof(buffer1), stdin);
                line[l] = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
                strcpy(line[l], buffer1);
                l++;
                if (strcmp(buffer1, ".\n") == 0)
                    break;
            }
            if (l < 4)
            {
                printf("Invalid format, please try again.\n");
                continue;
            }
            int j1 = 0, flag = 0, sd = 0;
            while (line[0][j1] != '\n')
            {
                if (line[0][j1] == ' ')
                {
                    flag = 1;
                    buffer1[j1] = '\0';
                    j1++;
                    printf("Invalid format, please try again.\n");

                    continue;
                }
                if (flag == 0)
                    buffer1[j1] = line[0][j1];
                if (flag == 1)
                    sender[sd++] = line[0][j1];
                j1++;
            }
            sender[sd] = '\0';
            if (strcmp(buffer1, "From:") != 0)
            {
                printf("Invalid format, please try again.\n");

                continue;
            }
            j1 = 0, flag = 0;
            int rv = 0;
            while (line[1][j1] != '\n')
            {
                if (line[1][j1] == ' ')
                {
                    flag = 1;
                    buffer1[j1] = '\0';
                    j1++;
                    printf("Invalid format, please try again.\n");

                    continue;
                }
                if (flag == 0)
                    buffer1[j1] = line[1][j1];
                if (flag == 1)
                    receiver[rv++] = line[1][j1];
                j1++;
            }
            receiver[rv] = '\0';
            if (strcmp(buffer1, "To:") != 0)
            {
                printf("Invalid format, please try again.\n");
                continue;
            }
            j1 = 0;
            while (line[2][j1] != '\n')
            {
                if (line[2][j1] == ' ')
                {
                    buffer1[j1] = '\0';
                    break;
                }
                buffer1[j1] = line[2][j1];
                j1++;
            }
            if (strcmp(buffer1, "Subject:") != 0)
            {
                printf("Invalid format, please try again.\n");

                continue;
            }
            int c = 0, id;
            flag = 0;
            for (int i = 0; i < sd; i++)
            {
                if (sender[i] == '@')
                {
                    c++;
                    id = i;
                }
            }
            if (c != 1)
                flag = 1;

            if (id == 0 || id == sd - 1)
                flag = 1;
            c = 0;
            for (int i = 0; i < rv; i++)
            {
                if (receiver[i] == '@')
                {
                    c++;
                    id = i;
                }
            }
            if (c != 1)
                flag = 1;
            if (id == 0 || id == rv - 1)
                flag = 1;

            if (flag)
            {
                printf("Incorrect Format\n");
                continue;
            }
            int clientSocket = 0;
            clientSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (clientSocket == -1)
            {
                perror("Error creating socket");
                exit(EXIT_FAILURE);
            }
            if ((connect(clientSocket, (struct sockaddr *)&serverAddress,
                         sizeof(serverAddress))) < 0)
            {
                perror("Unable to connect to server\n");
                exit(0);
            }
            while (1)
            {
                Rcv(buffer, buffer1, clientSocket);
                char *domain = extractDomain(buffer1);
                char *firstword = strtok(buffer1, delim);
                if (strcmp(firstword, "220") == 0)
                {
                    strcpy(buffer2, "HELLO ");
                    strcat(buffer2, domain);
                    Snd(buffer2, clientSocket);
                }
                else if (strcmp(firstword, "250") == 0)
                {
                    firstword = strtok(NULL, delim);
                    if (strcmp(firstword, "OK") == 0)
                    {
                        firstword = strtok(NULL, delim);
                        if (strcmp(firstword, "HELLO") == 0)
                        {
                            strcpy(buffer2, "MAIL FROM: <");
                            strcat(buffer2, sender);
                            strcat(buffer2, ">");
                            Snd(buffer2, clientSocket);
                            Rcv(buffer, buffer1, clientSocket);
                            firstword = strtok(buffer1, delim);
                            if (strcmp(firstword, "550") == 0)
                            {
                                printf("Incorrect username: connection broken\n");
                                break;
                            }
                            else
                            {
                                strcpy(buffer2, "RCPT TO: <");
                                strcat(buffer2, receiver);
                                strcat(buffer2, ">");
                                Snd(buffer2, clientSocket);
                                Rcv(buffer, buffer1, clientSocket);
                                strcpy(buffer2, "DATA");
                                Snd(buffer2, clientSocket);
                                Rcv(buffer, buffer1, clientSocket);
                                char MAIL[100];
                                for (int i = 0; i < l; i++)
                                {
                                    strcpy(MAIL, line[i]);
                                    MAIL[strlen(MAIL) - 1] = '\0';
                                    Snd(MAIL, clientSocket);
                                }
                            }
                        }
                        else
                        {
                            strcpy(buffer2, "QUIT");
                            Snd(buffer2, clientSocket);
                        }
                    }
                }
                else if (strcmp(firstword, "221") == 0)
                {
                    break;
                }
            }
            close(clientSocket);
        }
        else if (input == 3)
        {
            printf("Quitting the program\n");
            break;
        }
        else
        {
            continue;
        }
    }
}