#include "header.h"
int count = 0;
struct mtp_sock *Table;
int *last_inorder, *last_ack, *last_seq_no_send, *last_seq_no_recv;
struct SOCK_INFO *SOCK_INFO_info;
int sem_socket, over_socket, mutex;
struct sockaddr *src_addr;
int *no_space;
struct sembuf pop, vop;
#define sem_wait(s) semop(s, &pop, 1)
#define sem_signal(s) semop(s, &vop, 1)

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

void createTable()
{
    int shmid_Table;
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
}

void createSHM()
{
    int shmid_last_inorder, shmid_last_ack, shmid_last_seq_no_send, shmid_last_seq_no_recv, shmid_SOCK_INFO_info, shmid_no_space;
    shmid_SOCK_INFO_info = shmget(SOCK_INFO_info_KEY, sizeof(struct SOCK_INFO), IPC_CREAT | 0666);
    if (shmid_SOCK_INFO_info < 0)
    {
        perror("shmget for SOCK_INFO_info failed");
        exit(EXIT_FAILURE);
    }
    SOCK_INFO_info = (struct SOCK_INFO *)shmat(shmid_SOCK_INFO_info, NULL, 0);
    if (SOCK_INFO_info == (struct SOCK_INFO *)(-1))
    {
        perror("shmat for last_inorder failed");
        exit(EXIT_FAILURE);
    }

    shmid_last_inorder = shmget(last_inorder_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    if (shmid_last_inorder < 0)
    {
        perror("shmget for last_inorder failed");
        exit(EXIT_FAILURE);
    }
    last_inorder = (int *)shmat(shmid_last_inorder, NULL, 0);
    if (last_inorder == (int *)(-1))
    {
        perror("shmat for last_inorder failed");
        exit(EXIT_FAILURE);
    }

    shmid_last_ack = shmget(last_ack_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    if (shmid_last_ack < 0)
    {
        perror("shmget for last_ack failed");
        exit(EXIT_FAILURE);
    }
    last_ack = (int *)shmat(shmid_last_ack, NULL, 0);
    if (last_ack == (int *)(-1))
    {
        perror("shmat for last_ack failed");
        exit(EXIT_FAILURE);
    }

    shmid_last_seq_no_send = shmget(last_seq_no_send_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    if (shmid_last_seq_no_send < 0)
    {
        perror("shmget for last_seq_no_send failed");
        exit(EXIT_FAILURE);
    }
    last_seq_no_send = (int *)shmat(shmid_last_seq_no_send, NULL, 0);
    if (last_seq_no_send == (int *)(-1))
    {
        perror("shmat for last_seq_no_send failed");
        exit(EXIT_FAILURE);
    }

    shmid_last_seq_no_recv = shmget(last_seq_no_recv_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    if (shmid_last_seq_no_recv < 0)
    {
        perror("shmget for last_seq_no_recv failed");
        exit(EXIT_FAILURE);
    }
    last_seq_no_recv = (int *)shmat(shmid_last_seq_no_recv, NULL, 0);
    if (last_seq_no_recv == (int *)(-1))
    {
        perror("shmat for last_seq_no_recv failed");
        exit(EXIT_FAILURE);
    }
    shmid_no_space = shmget(no_space_KEY, sizeof(int) * N, IPC_CREAT | 0666);
    no_space = (int *)shmat(shmid_no_space, NULL, 0);
    if (no_space == (int *)(-1))
    {
        perror("shmat for no_space failed");
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
    if (semctl(*semid, 0, IPC_RMID, 0) == -1)
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}
void sigint_handler(int);
void ATTACH()
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        perror("Unable to register signal handler for SIGINT");
        exit(EXIT_FAILURE);
    }
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1;
    vop.sem_op = 1;
    createTable();
    createSHM();
    initSemaphore(&sem_socket, sem_socket_KEY, 0);
    initSemaphore(&over_socket, over_socket_KEY, 0);
    initSemaphore(&mutex, mutex_KEY, 1);
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
}

void sigint_handler(int signum)
{
    DETACH();
    exit(EXIT_SUCCESS);
}

int m_socket(int internetFamily, int type, int flag)
{
    ATTACH();
    sem_wait(mutex);
    SOCK_INFO_info->status = 0;
    SOCK_INFO_info->internetFamily = internetFamily;
    SOCK_INFO_info->flag = flag;
    int i = -1;
    for (i = 0; i < N; i++)
    {
        if (Table[i].free == 1)
        {
            Table[i].free = 0;
            SOCK_INFO_info->mtp_id = i;
            break;
        }
    }
    if (i == -1)
    {
        errno = ENOBUFS;
        sem_signal(mutex);
        DETACH();
        return -1;
    }
    sem_signal(sem_socket);
    sem_wait(over_socket);

    if (SOCK_INFO_info->errnum < 0)
    {
        Table[i].free = 1;
        errno = SOCK_INFO_info->err_no;
        int val = SOCK_INFO_info->errnum;
        sem_signal(mutex);
        DETACH();
        return val;
    }
    Table[i].process_id = getpid();
    for (int k = 0; k < SEND_BUFFER_SIZE; k++)
    {
        Table[i].send_buff[k].last_time = -2;
    }
    for (int k = 0; k < RECV_BUFFER_SIZE; k++)
    {
        Table[i].recv_buff[k].last_time = -2;
    }
    last_inorder[i] = 16;
    last_ack[i] = 16;
    last_seq_no_recv[i] = 16;
    last_seq_no_send[i] = 16;
    Table[i].swnd.first_idx = 0;
    Table[i].swnd.last_idx = RECV_BUFFER_SIZE - 1;
    Table[i].swnd.entry = 0;
    Table[i].swnd.first_msg = 1;
    Table[i].swnd.last_msg = RECV_BUFFER_SIZE;
    for (int k = 0; k < RECV_BUFFER_SIZE; k++)
    {
        Table[i].rwnd[k] = k + 1;
    }
    sem_signal(mutex);
    DETACH();
    return i;
}

int m_bind(int i, struct sockaddr *src_addr, struct sockaddr *dest_addr)
{
    ATTACH();
    sem_wait(mutex);
    SOCK_INFO_info->status = 1;
    SOCK_INFO_info->mtp_id = i;
    struct sockaddr_in *src = (struct sockaddr_in *)src_addr;
    struct sockaddr_in *dest = (struct sockaddr_in *)dest_addr;
    SOCK_INFO_info->src_addr = *(src);
    sem_signal(sem_socket);
    sem_wait(over_socket);
    if (SOCK_INFO_info->errnum < 0)
    {
        errno = SOCK_INFO_info->err_no;
        int val = SOCK_INFO_info->errnum;
        sem_signal(mutex);
        DETACH();
        return val;
    }
    Table[i].dest_addr = *(dest);
    sem_signal(mutex);
    DETACH();
    return 0;
}

void m_close(int i)
{
    sem_wait(mutex);
    ATTACH();
    Table[i].free = 1;
    sem_signal(mutex);
    DETACH();
}

int m_sendto(int i, char *buffer, int msg_len, int flag, struct sockaddr *dest_addr1, int addr_len)
{
    ATTACH();
    sem_wait(mutex);
    int entry = Table[i].swnd.entry;
    if (entry == Table[i].swnd.first_idx && Table[i].send_buff[entry].last_time != -2)
    {
        errno = ENOBUFS;
        sem_signal(mutex);
        DETACH();
        return (-1);
    }
    else
    {
        struct sockaddr_in *dest_addr = (struct sockaddr_in *)dest_addr1;
        if (dest_addr->sin_port != Table[i].dest_addr.sin_port || dest_addr->sin_addr.s_addr != Table[i].dest_addr.sin_addr.s_addr)
        {
            errno = ENOTCONN;
            sem_signal(mutex);
            DETACH();
            return (-1);
        }
        else
        {
            struct msg message;
            last_seq_no_send[i] = (last_seq_no_send[i] % 16) + 1;
            message.seq_no = last_seq_no_send[i];
            message.last_time = -1;
            if (msg_len > 1024)
            {
                msg_len = 1024;
            }
            for (int i = 0; i < msg_len; i++)
            {
                message.data[i] = buffer[i];
            }
            Table[i].send_buff[entry] = message;
            entry = (entry + 1) % SEND_BUFFER_SIZE;
            Table[i].swnd.entry = entry;
            sem_signal(mutex);
            DETACH();
            return msg_len;
        }
    }
}

int m_recvfrom(int i, char *buffer, int msg_len, int flag, struct sockaddr *dest_addr1, int *addr_len)
{
    ATTACH();
    sem_wait(mutex);
    if (last_inorder[i] == last_seq_no_recv[i])
    {
        errno = ENOMSG;
        sem_signal(mutex);
        DETACH();
        return -1;
    }
    else
    {
        last_seq_no_recv[i] = (last_seq_no_recv[i] % 16) + 1;

        for (int j = 0; j < RECV_BUFFER_SIZE; j++)
        {
            if (Table[i].recv_buff[j].seq_no == last_seq_no_recv[i] && Table[i].recv_buff[j].last_time == -1)
            {
                if (msg_len > 1024)
                {
                    msg_len = 1024;
                }
                for (int k = 0; k < msg_len; k++)
                {
                    buffer[k] = Table[i].recv_buff[j].data[k];
                }
                Table[i].recv_buff[j].last_time = -2;
                break;
            }
        }
        struct sockaddr_in *dest_addr2 = (struct sockaddr_in *)dest_addr1;
        *dest_addr2 = Table[i].dest_addr;
        *addr_len = sizeof(Table[i].dest_addr);
        sem_signal(mutex);
        DETACH();
        return msg_len;
    }
}