int m_socket(int, int, int);
int m_bind(int, struct sockaddr *, struct sockaddr *);
void m_close(int);
int m_sendto(int, char *, int, int, struct sockaddr *, int);
int m_recvfrom(int, char *, int, int, struct sockaddr *, int*);
int dropMessage(float prob);
#define SOCK_MTP 1000