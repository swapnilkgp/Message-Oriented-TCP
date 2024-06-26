#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>   
#include <netdb.h>       
#include <unistd.h>      
#include <errno.h>       
#include <string.h>      
#include <fcntl.h>       
#include <sys/types.h>   
#include <netinet/tcp.h> 
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/select.h>
#include <signal.h>

#define T 5
#define G_T 500
#define N 25
#define SEND_BUFFER_SIZE 10
#define RECV_BUFFER_SIZE 5
#define p 0.05
#define Table_KEY ftok(".", 10)
#define last_inorder_KEY ftok(".", 11)
#define last_ack_KEY ftok(".", 12)
#define last_seq_no_send_KEY ftok(".", 13)
#define last_seq_no_recv_KEY ftok(".", 14)
#define no_space_KEY ftok(".", 15)
#define SOCK_INFO_info_KEY ftok(".", 16)
#define sem_socket_KEY ftok(".", 17)
#define over_socket_KEY ftok(".", 18)
#define mutex_KEY ftok(".", 19)

struct msg
{
    int seq_no;
    time_t last_time;
    char data[1024];
};

struct window
{
    int first_idx, last_idx;
    int first_msg, last_msg;
    int entry;
};
struct mtp_sock
{
    int free;
    pid_t process_id;
    int udp_socket;
    struct sockaddr_in dest_addr;
    struct msg send_buff[SEND_BUFFER_SIZE];
    struct msg recv_buff[RECV_BUFFER_SIZE];
    struct window swnd;
    int rwnd[RECV_BUFFER_SIZE];
};

struct SOCK_INFO
{
    int status;
    int mtp_id;
    int internetFamily;
    int flag;
    struct sockaddr_in src_addr;
    int errnum;
    int err_no;
};