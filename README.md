---------------------------- Emulating End-to-End Reliable Flow Control over Unreliable Communication Channels ------------------------------------

Assignment - 5

Submission by :-
• Soumojit Chatterjee	(Roll No.- 21CS30062)
• Swapnil Ghosh 	(Roll No.- 21CS10086)

-------------------------------------------------------------------- Data structures used in msocket.c ------------------------------------------------------------------

♦ Struct 'msg' 

The struct msg represents a message to be transmitted, stored in send or receive buffers. It encapsulates essential information such as the sequence number, last transmission time, and data.

Components :-

• int seq_no			Sequence number of the message.
• time_t last_time		Timestamp indicating the last time the message was sent across the network. It is -2 if the slot is empty and -1 if the data was never sent.
• char data[1024]		Data part of the message, with a size of 1024 bytes.


♦ Struct 'window' 

The struct window represents the sender window in the context of communication. It is implemented as a circular queue to manage message transmission efficiently. The structure maintains essential information about the sender buffer, including marker indices and entry positions.

Components :-

•int first_idx: 	Index of the first end marker in the sender buffer.
•int last_idx: 		Index of the last end marker in the sender buffer.
•int first_msg: 	Sequence number of the first message in the window.
•int last_msg: 	Sequence number of the last message in the window.
•int entry: 		Position where data will be inserted during a m_sendto call.


♦ Struct 'mtp_sock' 

The struct mtp_sock represents the basic entry of a socket in the context of My Transport Protocol (MTP). It encapsulates essential attributes and buffers necessary for reliable message transmission and reception over UDP sockets.

Components :-

•int free: 										Flag indicating the availability of the socket entry. A value of 1 signifies that the entry is free, while 0 indicates that the entry is in use.
•pid_t process_id: 								Process ID associated with the socket.
•int udp_socket: 									File descriptor for the underlying UDP socket.
•struct sockaddr_in dest_addr:	 					Destination address for communication.
•struct msg send_buff[SEND_BUFFER_SIZE]: 		Array of message structures representing the send buffer.
•struct msg recv_buff[RECV_BUFFER_SIZE]: 		Array of message structures representing the receive buffer.
•struct window swnd: 								Sender window data structure.
•int rwnd[RECV_BUFFER_SIZE]: 					Array representing the receive window for flow control.


♦ Struct 'SOCK_INFO' 

The struct SOCK_INFO facilitates communication between the initialization process and a user process within the context of My Transport Protocol (MTP). It serves as a means for exchanging essential socket-related information between the processes, particularly during socket initialization (m_socket) and binding (m_bind) operations.

Components :-

•int status: 						Indicates the status of the socket. A value of 0 represents a socket() call and a value of 1 represents a bind() call.
•int mtp_id: 						Table index associated with the MTP socket.
•int internetFamily: 				Specifies the address family (e.g., IPv4, IPv6) required to pass in socket() call.
•int flag: 						Flag variable used for additional control or status information.
•struct sockaddr_in src_addr: 		Source address information associated with the socket required to bind() socket.
•int errnum: 						The return value of the underlying system calls.
•int err_no: 						errno global variable.



-------------------------------------------------------------------- Data structures used in initmsocket.c ------------------------------------------------------------------

♦ Last_inorder		Stores the sequence number of the last message which is received in-order by the receiver buffer
♦ Last_ack			Stores the sequence number of the Last message which is acknowledged by the receiver side
♦ Last_seq_no_send	Stores the last sequence number which is inserted into the send buffer by the m_sendto() call
♦ Last_seq_no_recv	Stores the last sequence number which is retrieved from the recv buffer by the m_recvfrom() call
♦ no_space			A flag to indicate if space is available in the recv buffer or not. A value of 1 indicates no space in recv buffer.
♦ ack_timer			A timer set for the special ACK sent when the value of no_space transits to 0 from 1. (This ACK is sent repetitively if the timer expires and is switched off when an appropriate data frame reaches the receiver side)

♦ Table
The Table is a data structure used to manage multiple MTP (My Transport Protocol) sockets in the system. It serves as a centralized repository for storing socket-specific information, allowing efficient access and manipulation of socket-related data.

Structure:
Type: Pointer to an array of structures of type mtp_sock.
Elements: Each element of the array represents a distinct MTP socket and contains detailed information about its state, configuration, and communication parameters.

---------------------------------------------------------------- Functions used in the program -------------------------------------------------------------------------------------------------

♦ dropMessage(float prob): 
This function simulates dropping a message based on a given probability prob. It generates a random number between 0 and 1 using rand() function and compares it with the probability. If the random number is less than the probability, the function returns 1 indicating that the message should be dropped, otherwise it returns 0 indicating that the message should not be dropped.

♦cmp(int a, int b): 
This function compares two sequence numbers a and b and returns:

0 if the difference between a and b is exactly 7,
1 if a is greater than b, and
-1 if a is less than b.
It's used to determine the order of sequence numbers accounting for the wrap-around condition in a sliding window protocol.

♦calEmptySpace(int idx): 
This function calculates the number of empty slots in the receive buffer of the MTP socket indexed by idx. It iterates through the receive buffer and counts the slots where the last_time field of the message is set to -2, indicating an empty slot.

♦firstEmptySlot(int idx): 
This function finds the index of the first empty slot in the receive buffer of the MTP socket indexed by idx. It iterates through the receive buffer and returns the index of the first slot where the last_time field of the message is set to -2, indicating an empty slot. If no empty slot is found, it returns -1.

♦RECV(int idx): 
This function is responsible for receiving messages from the UDP socket associated with the MTP socket indexed by idx. Upon receiving a message, it processes the message to determine whether it is an ACK or a data message. If it's an ACK message, it updates the sender window based on the ACK sequence numbers. If it's a data message, it constructs a struct msg containing the received data and returns it to the calling function. This function also simulates message loss based on a predefined probability.

♦sendACK(int i, int x, int y): 
This function is used to send acknowledgment (ACK) messages from the MTP socket indexed by i. It constructs an ACK message with sequence numbers ranging from x to y and sends it to the destination specified by the MTP socket. It also simulates message loss based on a predefined probability.

♦sendDATA(int i, int j): 
This function is responsible for sending data messages from the MTP socket indexed by i. It constructs a data message containing the data from the send buffer at index j and sends it to the destination specified by the MTP socket. It also simulates message loss based on a predefined probability and updates the last transmission time of the message if it was successfully sent.

♦createTable(): 
This function creates a shared memory segment for storing an array of struct mtp_sock instances. It initializes each mtp_sock instance as free.

♦createSHM(): 
This function creates shared memory segments for various shared variables used in the MTP implementation, such as SOCK_INFO_info, last_inorder, last_ack, last_seq_no_send, last_seq_no_recv, and no_space. It initializes these shared variables and attaches them to the current process.

♦initSemaphore(int *semid, int key, int initValue): 
This function initializes a semaphore with the specified key and initial value. If the semaphore does not exist, it creates a new semaphore with the initial value. If the semaphore already exists, it retrieves the existing semaphore.

♦destroySemaphore(int *semid): 
This function destroys the semaphore associated with the provided semaphore ID (semid).

♦ATTACH(): 
This function initializes and attaches shared memory segments and semaphores required for the MTP implementation. It calls createTable(), createSHM(), and initSemaphore() functions.

♦DETACH(): 
This function detaches shared memory segments and destroys semaphores associated with the MTP implementation. It is called during cleanup and deallocates resources allocated during initialization.



----------------------------------------------------------------- Average number of transmissions VS failure probability (p) ----------------------------------------------

| Failure Probability (p) 	| Number of Transmissions 	| Number of Transmissions per Message 

| 0                        			| 134                     				| 1.0000                               
| 0.05                     			| 138                     				| 1.0299                               
| 0.1                      			| 151                     				| 1.1269                               
| 0.15                     			| 168                     				| 1.2537                               
| 0.2                      			| 199                     				| 1.4851                               
| 0.25                     			| 218                     				| 1.6269                               
| 0.3                      			| 246                     				| 1.8358                               
| 0.35                     			| 269                     				| 2.0075                               
| 0.4                      			| 290                     				| 2.1642                               
| 0.45                     			| 316                     				| 2.3582                               
| 0.5                      			| 352                     				| 2.6269                               

