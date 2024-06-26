#include "header.h"
struct sembuf pop, vop;
#define sem_wait(s) semop(s, &pop, 1)
#define sem_signal(s) semop(s, &vop, 1)
int sem_socket, over_socket, mutex;
struct mtp_sock *Table;
int *last_inorder, *last_ack, *last_seq_no_send, *last_seq_no_recv;
struct SOCK_INFO *SOCK_INFO_info;
int *no_space;
int shmid_Table;
int shmid_last_inorder, shmid_last_ack, shmid_last_seq_no_send, shmid_last_seq_no_recv, shmid_SOCK_INFO_info, shmid_no_space;
int ack_timer[N];

int dropMessage(float prob)
{
    float randomNum = (float)rand() / RAND_MAX;

    if (randomNum < prob)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
int cmp(int a, int b)
{
    if (a > b)
    {
        int diff = a - b - 1;
        if (diff == 7)
        {
            return 0;
        }
        else if (diff > 7)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else if (a < b)
    {
        int diff = b - a - 1;
        if (diff == 7)
        {
            return 0;
        }
        else if (diff > 7)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    else
        return 0;
}

int calEmptySpace(int idx)
{
    int cnt = 0;
    for (int i = 0; i < RECV_BUFFER_SIZE; i++)
    {
        cnt += (Table[idx].recv_buff[i].last_time == -2);
    }
    return cnt;
}

int firstEmptySlot(int idx)
{
    int cnt = -1;
    for (int i = 0; i < RECV_BUFFER_SIZE; i++)
    {
        if (Table[idx].recv_buff[i].last_time == -2)
        {
            return i;
        }
    }
    return cnt;
}

void createTable()
{

    int i;

    shmid_Table = shmget(Table_KEY, sizeof(struct mtp_sock) * N, IPC_CREAT | 0666);
    if (shmid_Table < 0)
    {
        perror("shmget error while creating table");
        exit(EXIT_FAILURE);
    }

    Table = (struct mtp_sock *)shmat(shmid_Table, NULL, 0);
    if (Table == (struct mtp_sock *)(-1))
    {
        perror("shmat error while creating table");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++)
    {
        Table[i].free = 1;
    }
}

void createSHM()
{

    shmid_SOCK_INFO_info = shmget(SOCK_INFO_info_KEY, sizeof(struct SOCK_INFO), IPC_CREAT | 0666);
    if (shmid_SOCK_INFO_info < 0)
    {
        perror("shmget for SOCK_INFO_info ped");
        exit(EXIT_FAILURE);
    }
    SOCK_INFO_info = (struct SOCK_INFO *)shmat(shmid_SOCK_INFO_info, NULL, 0);
    if (SOCK_INFO_info == (struct SOCK_INFO *)(-1))
    {
        perror("shmat for last_inorder ped");
        exit(EXIT_FAILURE);
    }

    shmid_last_inorder = shmget(last_inorder_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    if (shmid_last_inorder < 0)
    {
        perror("shmget for last_inorder ped");
        exit(EXIT_FAILURE);
    }
    last_inorder = (int *)shmat(shmid_last_inorder, NULL, 0);
    if (last_inorder == (int *)(-1))
    {
        perror("shmat for last_inorder ped");
        exit(EXIT_FAILURE);
    }

    shmid_last_ack = shmget(last_ack_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    if (shmid_last_ack < 0)
    {
        perror("shmget for last_ack ped");
        exit(EXIT_FAILURE);
    }
    last_ack = (int *)shmat(shmid_last_ack, NULL, 0);
    if (last_ack == (int *)(-1))
    {
        perror("shmat for last_ack ped");
        exit(EXIT_FAILURE);
    }

    shmid_last_seq_no_send = shmget(last_seq_no_send_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    if (shmid_last_seq_no_send < 0)
    {
        perror("shmget for last_seq_no_send ped");
        exit(EXIT_FAILURE);
    }
    last_seq_no_send = (int *)shmat(shmid_last_seq_no_send, NULL, 0);
    if (last_seq_no_send == (int *)(-1))
    {
        perror("shmat for last_seq_no_send ped");
        exit(EXIT_FAILURE);
    }

    shmid_last_seq_no_recv = shmget(last_seq_no_recv_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    if (shmid_last_seq_no_recv < 0)
    {
        perror("shmget for last_seq_no_recv ped");
        exit(EXIT_FAILURE);
    }
    last_seq_no_recv = (int *)shmat(shmid_last_seq_no_recv, NULL, 0);
    if (last_seq_no_recv == (int *)(-1))
    {
        perror("shmat for last_seq_no_recv ped");
        exit(EXIT_FAILURE);
    }
    shmid_no_space = shmget(no_space_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    no_space = (int *)shmat(shmid_no_space, NULL, 0);
    if (no_space == (int *)(-1))
    {
        perror("shmat for no_space ped");
        exit(EXIT_FAILURE);
    }
}

void initSemaphore(int *semid, int key, int initValue)
{

    if ((*semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666)) != -1)
    {

        int arg = initValue;
        if (semctl(*semid, 0, SETVAL, arg) == -1)
        {
            perror("semctl");
            exit(EXIT_FAILURE);
        }
    }
    else
    {

        if ((*semid = semget(key, 1, 0)) == -1)
        {
            perror("semget");
            exit(EXIT_FAILURE);
        }
    }
    return;
}

void destroySemaphore(int *semid)
{
    if (semctl(*semid, 0, IPC_RMID) == -1)
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

void ATTACH()
{
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;
    createTable();
    createSHM();
    initSemaphore(&sem_socket, sem_socket_KEY, 0);
    initSemaphore(&over_socket, over_socket_KEY, 0);
    initSemaphore(&mutex, mutex_KEY, 1);
    for (int i = 0; i < N; i++)
    {
        ack_timer[i] = -1;
    }
}
void DETACH()
{
    if (shmdt(Table) == -1)
    {
        perror("shmdt error while detaching table");
        exit(EXIT_FAILURE);
    }
    if (shmdt(last_inorder) == -1)
    {
        perror("shmdt error while detaching last_inorder");
        exit(EXIT_FAILURE);
    }
    if (shmdt(last_ack) == -1)
    {
        perror("shmdt error while detaching last_ack");
        exit(EXIT_FAILURE);
    }
    if (shmdt(last_seq_no_recv) == -1)
    {
        perror("shmdt error while detaching last_seq_no_recv");
        exit(EXIT_FAILURE);
    }
    if (shmdt(last_seq_no_send) == -1)
    {
        perror("shmdt error while detaching last_seq_no_send");
        exit(EXIT_FAILURE);
    }
    if (shmdt(SOCK_INFO_info) == -1)
    {
        perror("shmdt error while detaching SOCK_INFO_info");
        exit(EXIT_FAILURE);
    }
    if (shmdt(no_space) == -1)
    {
        perror("shmdt error while detaching no_space");
        exit(EXIT_FAILURE);
    }
    shmctl(shmid_Table, IPC_RMID, 0);
    shmctl(shmid_last_ack, IPC_RMID, 0);
    shmctl(shmid_last_inorder, IPC_RMID, 0);
    shmctl(shmid_last_seq_no_recv, IPC_RMID, 0);
    shmctl(shmid_last_seq_no_send, IPC_RMID, 0);
    shmctl(shmid_no_space, IPC_RMID, 0);
    shmctl(shmid_SOCK_INFO_info, IPC_RMID, 0);

    destroySemaphore(&sem_socket);
    destroySemaphore(&over_socket);
    destroySemaphore(&mutex);
}

void sigint_handler(int signum)
{

    DETACH();

    exit(EXIT_SUCCESS);
}

struct msg *RECV(int idx)
{
    printf("Some msg RECV for i : %d\n", idx);
    struct sockaddr_in client_addr;
    int client_addr_len;
    int sockfd = Table[idx].udp_socket;
    char recvBUFF[1026];
    int bytes_recv = recvfrom(sockfd, recvBUFF, 1026, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (recvBUFF[0] == 'A')
    {
        printf("ACK %d %d\n", recvBUFF[1] - 'A', recvBUFF[2] - 'A');
    }
    else
    {
        printf("DATA %d\n", recvBUFF[1] - 'A');
    }
    if (bytes_recv <= 0)
    {
        return NULL;
    }
    else
    {
        if (recvBUFF[0] == 'A')
        {
            int x = recvBUFF[1] - 'A';
            int y = recvBUFF[2] - 'A';
            printf("ACK recvd : %d,%d\n", x, y);
            if (cmp(last_ack[idx], x) == 1)
            {
                return (NULL);
            }
            else
            {
                int first_idx = Table[idx].swnd.first_idx;
                int last_idx = Table[idx].swnd.last_idx;
                printf("first_idx : %d\n", first_idx);
                printf("last_idx : %d\n", last_idx);
                int flg = 0;
                for (int j = first_idx; j != (last_idx + 5) % SEND_BUFFER_SIZE; j = (j + 1) % SEND_BUFFER_SIZE)
                {
                    if (Table[idx].send_buff[j].last_time == -2)
                    {
                        flg = 1;
                        Table[idx].swnd.first_idx = j;
                        Table[idx].swnd.last_idx = (j + y - 1) % SEND_BUFFER_SIZE;
                        if (Table[idx].swnd.last_idx < 0)
                        {
                            Table[idx].swnd.last_idx += SEND_BUFFER_SIZE;
                        }
                        Table[idx].swnd.first_msg = (x) % 16 + 1;
                        Table[idx].swnd.last_msg = (x + y - 1) % 16 + 1;
                        last_ack[idx] = x;
                        printf("swnd updated to %d %d\n", j, (j + y - 1) % SEND_BUFFER_SIZE);
                        break;
                    }

                    if (Table[idx].send_buff[j].seq_no != (x % 16) + 1)
                    {
                        printf("%d %d freed\n", j, Table[idx].send_buff[j].seq_no);
                        Table[idx].send_buff[j].last_time = -2;
                    }
                    else
                    {
                        Table[idx].swnd.first_idx = j;
                        Table[idx].swnd.last_idx = (j + y - 1) % SEND_BUFFER_SIZE;
                        if (Table[idx].swnd.last_idx < 0)
                        {
                            Table[idx].swnd.last_idx += SEND_BUFFER_SIZE;
                        }
                        Table[idx].swnd.first_msg = (x) % 16 + 1;
                        Table[idx].swnd.last_msg = (x + y - 1) % 16 + 1;
                        last_ack[idx] = x;
                        printf("swnd updated to %d %d\n", j, (j + y - 1) % SEND_BUFFER_SIZE);
                        flg = 1;
                        break;
                    }
                }
                if (!flg)
                {
                    int j = (last_idx + 5) % SEND_BUFFER_SIZE;
                    Table[idx].swnd.first_idx = j;
                    Table[idx].swnd.last_idx = (j + y - 1) % SEND_BUFFER_SIZE;
                    if (Table[idx].swnd.last_idx < 0)
                    {
                        Table[idx].swnd.last_idx += SEND_BUFFER_SIZE;
                    }
                    Table[idx].swnd.first_msg = (x) % 16 + 1;
                    Table[idx].swnd.last_msg = (x + y - 1) % 16 + 1;
                    last_ack[idx] = x;
                    printf("swnd updated to %d %d\n", j, (j + y - 1) % SEND_BUFFER_SIZE);
                }
            }

            return (NULL);
        }
        else if (recvBUFF[0] == 'D')
        {
            int seq_no = recvBUFF[1] - 'A';
            struct msg *message = (struct msg *)malloc(sizeof(struct msg));
            message->seq_no = seq_no;
            message->last_time = -1;
            for (int i = 0; i < 1024; i++)
            {
                message->data[i] = recvBUFF[i + 2];
            }
            return message;
        }
    }
}

int sendACK(int i, int x, int y)
{
    int drop = dropMessage(p);
    if (drop == 1)
    {
        printf("ACK sent by socket %d lost: %d %d\n", i, x, y);
        return 0;
    }
    char buffer[3];
    buffer[0] = 'A';
    buffer[1] = 'A' + x;
    buffer[2] = 'A' + y;
    printf("ACK sent by socket %d successfully: %d %d\n", i, x, y);
    int ret = sendto(Table[i].udp_socket, buffer, 3, 0, (struct sockaddr *)&Table[i].dest_addr, sizeof(Table[i].dest_addr));
    return ret;
}

int sendDATA(int i, int j)
{
    int drop = dropMessage(p);
    if (drop == 1)
    {
        printf("DATA for socket : %d, idx : %d, seq_no : %d lost\n", i, j, Table[i].send_buff[j].seq_no);
        return 0;
    }
    char buffer[1026];
    buffer[0] = 'D';
    buffer[1] = 'A' + Table[i].send_buff[j].seq_no;
    for (int k = 0; k < 1024; k++)
    {
        buffer[k + 2] = Table[i].send_buff[j].data[k];
    }
    int ret = sendto(Table[i].udp_socket, buffer, 1026, 0, (struct sockaddr *)&Table[i].dest_addr, sizeof(Table[i].dest_addr));
    time_t now;
    now = time(NULL);
    if (ret == 1026)
    {
        Table[i].send_buff[j].last_time = now;
    }
    printf("DATA for socket : %d, idx : %d, seq_no : %d sent\n", i, j, Table[i].send_buff[j].seq_no);
    return ret;
}

void *R(void *arg)
{
    fd_set read_fd;
    int max_fd = 0, activity;
    while (1)
    {
        sem_wait(mutex);
        FD_ZERO(&read_fd);
        for (int i = 0; i < N; i++)
        {
            if (Table[i].free)
                continue;
            int y = calEmptySpace(i);
            if (y == 0)
            {
                no_space[i] = 1;
            }
            if (ack_timer[i] != -1)
            {
                time_t curr_time = time(NULL);
                double time_elapsed = difftime(curr_time, ack_timer[i]);
                if (time_elapsed >= T)
                {
                    sendACK(i, last_inorder[i], y);
                    ack_timer[i] = curr_time;
                }
            }
            FD_SET(Table[i].udp_socket, &read_fd);
            if (max_fd < Table[i].udp_socket)
                max_fd = Table[i].udp_socket;
            if (no_space[i] == 1)
            {
                int y = calEmptySpace(i);
                if (y > 0)
                {
                    no_space[i] = 0;
                    time_t curr_time = time(NULL);
                    ack_timer[i] = curr_time;
                    sendACK(i, last_inorder[i], y);
                    for (int j = 0; j < RECV_BUFFER_SIZE; j++)
                    {
                        Table[i].rwnd[j] = -1;
                    }
                    int cnt = 0;
                    int msg_seq = last_inorder[i] % 16 + 1;
                    int ptr = 0;
                    while (cnt < y)
                    {
                        int flag = 0;
                        for (int k = 0; k < RECV_BUFFER_SIZE; k++)
                        {
                            if (Table[i].recv_buff[k].seq_no == msg_seq)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        if (flag == 0)
                        {
                            cnt++;
                            Table[i].rwnd[ptr++] = msg_seq;
                        }
                        msg_seq = (msg_seq % 16) + 1;
                    }
                    printf("RWND : \n");
                    for (int j = 0; j < RECV_BUFFER_SIZE; j++)
                    {
                        if (Table[i].rwnd[j] != -1)
                        {
                            printf("%d ", Table[i].rwnd[j]);
                        }
                    }
                    printf("\n");
                }
            }
        }
        sem_signal(mutex);
        struct timeval timeout;
        timeout.tv_sec = T;
        timeout.tv_usec = 0;
        int activity = select(max_fd + 1, &read_fd, NULL, NULL, &timeout);
        if (activity == 0)
        {
            continue;
        }
        else
        {
            sem_wait(mutex);

            for (int i = 0; i < N; i++)
            {
                if (Table[i].free)
                    continue;
                if (FD_ISSET(Table[i].udp_socket, &read_fd))
                {
                    printf("inside FD_ISSET : %d\n", i);
                    struct msg *message = RECV(i);
                    if (message != NULL)
                    {
                        int flag = 0;
                        printf("rwnd for socket %d : \n", i);
                        for (int j = 0; j < RECV_BUFFER_SIZE; j++)
                        {
                            printf("%d ", Table[i].rwnd[j]);
                            if (Table[i].rwnd[j] == message->seq_no)
                            {
                                flag = 1;
                                break;
                            }
                        }
                        printf("\n");
                        if (flag == 0)
                        {
                            printf("flag = 0\n");
                            int y = calEmptySpace(i);
                            if (y == 0)
                            {
                                no_space[i] = 1;
                            }
                            sendACK(i, last_inorder[i], y);
                        }
                        else
                        {
                            printf("flag = 1\n");
                            int j = firstEmptySlot(i);
                            if (j != -1)
                            {
                                Table[i].recv_buff[j] = *message;
                                ack_timer[i] = -1;
                            }
                            if (j != -1 && message->seq_no == (last_inorder[i] % 16) + 1)
                            {
                                for (int j = 0; j < RECV_BUFFER_SIZE; j++)
                                {
                                    if (Table[i].recv_buff[j].last_time == -1)
                                    {
                                        printf("%d ", Table[i].recv_buff[j].seq_no);
                                    }
                                }
                                while (1)
                                {
                                    int flag = 0;
                                    for (int j = 0; j < RECV_BUFFER_SIZE; j++)
                                    {
                                        if (Table[i].recv_buff[j].seq_no == last_inorder[i] % 16 + 1 && Table[i].recv_buff[j].last_time == -1)
                                        {
                                            flag = 1;
                                        }
                                    }
                                    if (flag)
                                    {
                                        last_inorder[i] = last_inorder[i] % 16 + 1;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }
                            int y = calEmptySpace(i);
                            if (y == 0)
                            {
                                no_space[i] = 1;
                            }
                            sendACK(i, last_inorder[i], y);
                            for (j = 0; j < RECV_BUFFER_SIZE; j++)
                            {
                                Table[i].rwnd[j] = -1;
                            }
                            int cnt = 0;
                            int msg_seq = last_inorder[i] % 16 + 1;
                            int ptr = 0;
                            while (cnt < y)
                            {
                                int flag = 0;
                                for (int k = 0; k < RECV_BUFFER_SIZE; k++)
                                {
                                    if (Table[i].recv_buff[k].seq_no == msg_seq && Table[i].recv_buff[k].last_time == -1)
                                    {
                                        flag = 1;
                                        break;
                                    }
                                }
                                if (flag == 0)
                                {
                                    cnt++;
                                    Table[i].rwnd[ptr++] = msg_seq;
                                }
                                msg_seq = (msg_seq % 16) + 1;
                            }
                            printf("RWND for socket %d : \n", i);
                            for (int j = 0; j < RECV_BUFFER_SIZE; j++)
                            {
                                if (Table[i].rwnd[j] != -1)
                                {
                                    printf("%d ", Table[i].rwnd[j]);
                                }
                            }
                            printf("\n");
                        }

                        free(message);
                    }
                }
            }
            sem_signal(mutex);
        }
    }
}

void *G(void *arg)
{
    while (1)
    {
        sleep(G_T);
        sem_wait(mutex);
        printf("Garbage collector is active\n");
        for (int i = 0; i < N; i++)
        {
            if (Table[i].free)
            {
                continue;
            }
            int dead = 0;
            if (kill(Table[i].process_id, 0) && errno == ESRCH)
            {
                dead = 1;
            }
            if (dead)
            {
                Table[i].free = 1;
                printf("Closing socket %d\n", i);
                close(Table[i].udp_socket);
            }
        }
        sem_signal(mutex);
    }
}

void *S(void *arg)
{
    while (1)
    {
        sleep(T / 2);
        sem_wait(mutex);
        // printf("S thread wakes up\n");
        for (int i = 0; i < N; i++)
        {
            int time_expired = 0;
            if (Table[i].free)
                continue;
            int first_idx = Table[i].swnd.first_idx;
            int last_idx = Table[i].swnd.last_idx;
            // printf("Data sending for socket - %d\n", i);
            for (int j = first_idx; j != (last_idx + 1) % SEND_BUFFER_SIZE; j = (j + 1) % SEND_BUFFER_SIZE)
            {
                if (Table[i].send_buff[j].last_time == -2)
                    continue;
                else if (Table[i].send_buff[j].last_time == -1)
                {
                    sendDATA(i, j);
                }
                else
                {
                    time_t curr_time = time(NULL);
                    double time_elapsed = difftime(curr_time, Table[i].send_buff[j].last_time);
                    if (time_elapsed >= T || time_expired == 1)
                    {
                        time_expired = 1;
                        sendDATA(i, j);
                        Table[i].send_buff[j].last_time = curr_time;
                    }
                }
            }
        }
        sem_signal(mutex);
    }
}

int main()
{
    ATTACH();
    pthread_t R_thread_id;
    pthread_t S_thread_id;
    pthread_t G_thread_id;
    pthread_create(&R_thread_id, NULL, R, NULL);
    pthread_create(&S_thread_id, NULL, S, NULL);
    pthread_create(&G_thread_id, NULL, G, NULL);
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        perror("Unable to register signal handler for SIGINT");
        return EXIT_FAILURE;
    }
    while (1)
    {
        sem_wait(sem_socket);
        if (SOCK_INFO_info->status == 0)
        {
            int sockfd = socket(SOCK_INFO_info->internetFamily, SOCK_DGRAM, SOCK_INFO_info->flag);
            printf("Socket creation for %d\n", SOCK_INFO_info->mtp_id);
            perror("status");
            if (sockfd < 0)
            {
                SOCK_INFO_info->errnum = sockfd;
                SOCK_INFO_info->err_no = errno;
            }
            else
            {
                Table[SOCK_INFO_info->mtp_id].udp_socket = sockfd;
                SOCK_INFO_info->errnum = sockfd;
            }
        }
        else if (SOCK_INFO_info->status == 1)
        {
            int ret = bind(Table[SOCK_INFO_info->mtp_id].udp_socket, (struct sockaddr *)&(SOCK_INFO_info->src_addr), sizeof(SOCK_INFO_info->src_addr));
            printf("Socket binding for %d\n", SOCK_INFO_info->mtp_id);
            perror("status");
            if (ret < 0)
            {
                SOCK_INFO_info->errnum = ret;
                SOCK_INFO_info->err_no = errno;
            }
            else
            {
                SOCK_INFO_info->errnum = 0;
            }
        }

        SOCK_INFO_info->status = -1;
        sem_signal(over_socket);
    }
    DETACH();
    return 0;
}
