#include "client/dfs_client.h"
#include "datanode/ext.h"
#include  <sys/types.h>                        
#include  <sys/socket.h>  

int connect_to_nn(char* address, int port)
{
	assert(address != NULL);
	assert(port >= 1 && port <= 65535);

	int socket_descriptor;

	socket_descriptor = create_client_tcp_socket(address, port);
	if(socket_descriptor < 0){
		error("Error opening socket!\n");
	}	

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
