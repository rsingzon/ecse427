#include "client/dfs_client.h"
#include "datanode/ext.h"
#include  <sys/types.h>                        
#include  <sys/socket.h>  

int connect_to_nn(char* address, int port)
{
	assert(address != NULL);
	assert(port >= 1 && port <= 65535);

	int socket_descriptor;
	struct sockaddr_in serv_addr;
	in_addr_t address_int;
	char *ip_string;

	socket_descriptor = create_tcp_socket();
	if(socket_descriptor < 0){
		error("Error opening socket!\n");
	}

	//Allocate memory for the server address and set values
	//bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	//VERIFY THAT htons IS THE RIGHT METHOD
	serv_addr.sin_port = htons(port);
	//sin_addr is a struct that contains another variable for the address
	
	address_int = inet_addr(address);
//	ip_string = inet_ntoa(*(struct in_addr*)address);
//	printf("IP: %s\n", ip_string);

	serv_addr.sin_addr.s_addr = address_int;

	printf("SERVER ADDRESS VARIABLES:\n");
	printf("\tPort: %d\n", port);
	printf("\tsin_family: %d\n", serv_addr.sin_family);
	printf("\tsin_port: %d\n", serv_addr.sin_port);
	printf("\tsin_address: %d\n", serv_addr.sin_addr.s_addr);


	printf("CONNECTING\n");
	connect(socket_descriptor, &serv_addr, sizeof(serv_addr));
	printf("CONNECTED\n");

	//TODO: create a socket and connect it to the server (address, port)
	//assign return value to client_socket 
	int client_socket = -1;
	
	return client_socket;
}

int modify_file(char *ip, int port, const char* filename, int file_size, int start_addr, int end_addr)
{
	int namenode_socket = connect_to_nn(ip, port);
	if (namenode_socket == INVALID_SOCKET) return -1;
	FILE* file = fopen(filename, "rb");
	assert(file != NULL);

	//TODO:fill the request and send
	dfs_cm_client_req_t request;
	
	//TODO: receive the response
	dfs_cm_file_res_t response;

	//TODO: send the updated block to the proper datanode

	fclose(file);
	return 0;
}

int push_file(int namenode_socket, const char* local_path)
{
	assert(namenode_socket != INVALID_SOCKET);
	assert(local_path != NULL);
	FILE* file = fopen(local_path, "rb");
	assert(file != NULL);

	// Create the push request
	dfs_cm_client_req_t request;

	//TODO:fill the fields in request and 
	
	//TODO:Receive the response
	dfs_cm_file_res_t response;

	//TODO: Send blocks to datanodes one by one

	fclose(file);
	return 0;
}

int pull_file(int namenode_socket, const char *filename)
{
	assert(namenode_socket != INVALID_SOCKET);
	assert(filename != NULL);

	//TODO: fill the request, and send
	dfs_cm_client_req_t request;

	//TODO: Get the response
	dfs_cm_file_res_t response;
	
	//TODO: Receive blocks from datanodes one by one
	
	FILE *file = fopen(filename, "wb");
	//TODO: resemble the received blocks into the complete file
	fclose(file);
	return 0;
}

dfs_system_status *get_system_info(int namenode_socket)
{
	assert(namenode_socket != INVALID_SOCKET);
	//TODO fill the result and send 
	dfs_cm_client_req_t request;
	
	//TODO: get the response
	dfs_system_status *response; 

	return response;		
}

int send_file_request(char **argv, char *filename, int op_type)
{
	int namenode_socket = connect_to_nn(argv[1], atoi(argv[2]));
	if (namenode_socket < 0)
	{
		return -1;
	}

	int result = 1;
	switch (op_type)
	{
		case 0:
			result = pull_file(namenode_socket, filename);
			break;
		case 1:
			result = push_file(namenode_socket, filename);
			break;
	}
	close(namenode_socket);
	return result;
}

dfs_system_status *send_sysinfo_request(char **argv)
{
	int namenode_socket = connect_to_nn(argv[1], atoi(argv[2]));
	if (namenode_socket < 0)
	{
		return NULL;
	}
	dfs_system_status* ret =  get_system_info(namenode_socket);
	close(namenode_socket);
	return ret;
}
