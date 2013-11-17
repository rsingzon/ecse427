#include "common/dfs_common.h"
#include <pthread.h>
/**
 * create a thread and activate it
 * entry_point - the function exeucted by the thread
 * args - argument of the function
 * return the handler of the thread
 */
inline pthread_t * create_thread(void * (*entry_point)(void*), void *args)
{
	//TODO: create the thread and run it
	pthread_t * thread;
	thread = malloc(sizeof(pthread_t));

	//pthread_create(thread,attr,start_routine,arg);
	pthread_create(thread, NULL, entry_point, args);

	return thread;
}

/**
 * create a socket and return
 */
int create_tcp_socket()
{
	int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	//TODO:create the socket and return the file descriptor 
	return socket_fd;
}

/**
 * create the socket and connect it to the destination address
 * return the socket fd
 */
int create_client_tcp_socket(char* address, int port)
{
	assert(port >= 0 && port < 65536);
	int socket = create_tcp_socket();
	if (socket == INVALID_SOCKET) return 1;

	//Define variables
	struct sockaddr_in serv_addr;
	in_addr_t address_int;
	char *ip_string;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	address_int = inet_addr(address);
	serv_addr.sin_addr.s_addr = address_int;

	//TODO: connect it to the destination port
	printf("Client: Attempting to connect\n");
	if ( connect(socket, &serv_addr, sizeof(serv_addr)) < 0 ){
		perror("Client failed to connect!\n");
		exit(1);
	}


	return socket;
}

/**
 * create a socket listening on the certain local port and return
 */
int create_server_tcp_socket(int port)
{
	assert(port >= 0 && port < 65536);
	int socket = create_tcp_socket();
	if (socket == INVALID_SOCKET) return 1;

	//Bind socket to local IP address
	struct sockaddr_in name_addr;
	in_addr_t address_int;
	name_addr.sin_family = AF_INET;
	name_addr.sin_port = htons(port);
	address_int = inet_addr("127.0.0.1");
	if(address_int < 0){
		printf("Error converting IP address\n");
	}
	name_addr.sin_addr.s_addr = address_int;

	int isBound;
	isBound = bind(socket, (struct sockaddr *) &name_addr, sizeof(name_addr));
	if(isBound < 0) {
		perror("Error on binding!\n");
		//exit(1);
	}

	//TODO: listen on local port
	listen(socket, 5);
	printf("Bound and listening!\n");

	return socket;
}

/**
 * socket - connecting socket
 * data - the buffer containing the data
 * size - the size of buffer, in byte
 */
void send_data(int socket, void* data, int size)
{
	assert(data != NULL);
	assert(size >= 0);
	if (socket == INVALID_SOCKET) return;
	//TODO: send data through socket
	send(socket, data, size, 0);
}

/**
 * receive data via socket
 * socket - the connecting socket
 * data - the buffer to store the data
 * size - the size of buffer in byte
 */
void receive_data(int socket, void* data, int size)
{
	assert(data != NULL);
	assert(size >= 0);
	if (socket == INVALID_SOCKET) return;
	//TODO: fetch data via socket

	recv(socket, data, size, 0);
}
