#include "client/dfs_client.h"
#include "datanode/ext.h"
#include  <sys/types.h>                        
#include  <sys/socket.h>  

int connect_to_nn(char* address, int port)
{
	assert(address != NULL);
	assert(port >= 1 && port <= 65535);

	//TODO: create a socket and connect it to the server (address, port)
	//assign return value to client_socket 
	int client_socket = -1;

	client_socket = create_client_tcp_socket(address, port);
	if(client_socket < 0){
		error("Error opening socket!\n");
	}
	
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
	
	strcpy(request.file_name, local_path);

	//Get the offset of the file pointer when it is at the end of the file
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	request.file_size = size;
	request.req_type = 1;

	printf("\nPushing file [%s]: \n", local_path);
	printf("\tFile name: %s\n", request.file_name);
	printf("\tFile size: %d\n", request.file_size);

	send_data(namenode_socket, &request, sizeof(dfs_cm_client_req_t));
	
	//TODO:Receive the response
	dfs_cm_file_res_t *response;
	response = malloc(sizeof(*response));

	receive_data(namenode_socket, response, sizeof(*response));

	//TODO: Send blocks to datanodes one by one
	dfs_cm_file_t block_info = *(dfs_cm_file_t*)response;
	int blocks_to_send = block_info.blocknum;
	printf("File name: %s\n", block_info.filename);
	printf("File size: %d\n", block_info.file_size);
	printf("Blocks to send: %d\n", blocks_to_send);

	dfs_cli_dn_req_t datanode_request;
	dfs_cm_block_t block_to_send;
	int datanode_socket = 0;

	//dfs_cm_block_t block_list[MAX_FILE_BLK_COUNT];

	int count = 0;
	while(count < blocks_to_send){

		//Read data from file into the block
		block_to_send = block_info.block_list[count];
		fread( &(block_to_send.content), DFS_BLOCK_SIZE, 1, file);

		char *dn_address = block_to_send.loc_ip;
		int dn_port = block_to_send.loc_port;

		printf("\nDN IP: %s\n", dn_address);
		printf("DN Port: %d\n", dn_port);

		datanode_socket = create_client_tcp_socket(dn_address, dn_port);
     
		datanode_request.op_type = 1;
		datanode_request.block = block_to_send;

		send_data(datanode_socket, &datanode_request, sizeof(datanode_request));
		count++;
	}

	fclose(file);
	free(response);
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

	request.req_type = 2;
	send_data(namenode_socket, &request, sizeof(dfs_cm_client_req_t));
	
	//TODO: get the response
	dfs_system_status *response; 
	response = malloc(sizeof(dfs_system_status));

	receive_data(namenode_socket, response, sizeof(dfs_system_status));

	printf("Number of datanodes: %d\n", response->datanode_num);
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
	printf("Client: namenode socket: %d\n", namenode_socket);
	if (namenode_socket < 0)
	{
		return NULL;
	}
	dfs_system_status* ret =  get_system_info(namenode_socket);
	close(namenode_socket);
	return ret;
}
