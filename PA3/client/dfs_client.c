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
	memset(&request, 0, sizeof(request));

	strcpy(request.filename, filename);
	request.file_size = file_size;
	request.req_type = 3;

	send_data(namenode_socket, &request, sizeof(request));
	
	//TODO: receive the response
	dfs_cm_file_res_t response;
	memset(&response, 0, sizeof(response));

	receive_data(namenode_socket, &response, sizeof(response));

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

	printf("\nClient request to send [%s]: \n", local_path);

	printf("\tFile name: %s\n", request.file_name);
	printf("\tFile size: %d\n", request.file_size);

	//Send the request to the namenode
	send_data(namenode_socket, &request, sizeof(dfs_cm_client_req_t));

	
	//TODO:Receive the response
	dfs_cm_file_res_t response;
	memset(&response, 0, sizeof(response));

	receive_data(namenode_socket, &response, sizeof(dfs_cm_file_res_t));

	printf("Namenode response:\n\tFile name: %s\n", response.query_result.filename);
	printf("\tFile size: %d\n", response.query_result.file_size);
	printf("\tBlocks to send: %d\n", response.query_result.blocknum);

	//Retrieve file info from the response
	dfs_cm_file_t file_info = response.query_result;

	//TODO: Send blocks to datanodes one by one
	int count = 0;
	while(count < file_info.blocknum){

		dfs_cm_block_t file_block;
		memset(&file_block, 0, sizeof(file_block));

		int datanode_socket = 0;

		//Get the IP and port from the namenode response
		file_block = file_info.block_list[count];

		//Allocate space for the file content on the block
		//memset(&(file_block.content), 0, DFS_BLOCK_SIZE);

		//Read data from file into the block
		fread( &(file_block.content), DFS_BLOCK_SIZE, 1, file);

		printf("\nDN IP: %s\n", file_block.loc_ip);
		printf("DN Port: %d\n", file_block.loc_port);
//		printf("Content: %s\n", file_block.content);

		//Create a socket to communicate with the datanode
		datanode_socket = create_client_tcp_socket(file_block.loc_ip, file_block.loc_port);
     
     	//Fill datanode request and send
		dfs_cli_dn_req_t datanode_request;
		memset(&datanode_request, 0, sizeof(datanode_request));

		datanode_request.op_type = 1;
		datanode_request.block = file_block;

		send_data(datanode_socket, &datanode_request, sizeof(datanode_request));
		count++;
	
	}

	fclose(file);
	return 0;
}


int pull_file(int namenode_socket, const char *filename)
{
	assert(namenode_socket != INVALID_SOCKET);
	assert(filename != NULL);

	//TODO: fill the request, and send
	dfs_cm_client_req_t request;
	strcpy(request.file_name, filename);
	request.req_type = 0;

	printf("\nClient request to send [%s]: \n", request.file_name);

	send_data(namenode_socket, &request, sizeof(dfs_cm_client_req_t));

	//Allocate space for the response and receive it
	dfs_cm_file_res_t response;
	memset(&response, 0, sizeof(response));
	receive_data(namenode_socket, &response, sizeof(response));
	
	dfs_cm_file_t file_info;
	memset(&file_info, 0, sizeof(file_info));
	file_info = response.query_result;

	printf("Namenode response:\n\tFile name: %s\n", file_info.filename);
	printf("\tFile size: %d\n", file_info.file_size);
	printf("\tNumber of blocks: %d\n", file_info.blocknum);

	//TODO: Receive blocks from datanodes one by one
	int num_blocks = file_info.blocknum;
	int count = 0;

	//Allocate space for all the blocks for a particular file
	dfs_cm_block_t block_list[num_blocks];

	while (count < num_blocks){
		//Obtain the block information from the namenode
		dfs_cm_block_t file_block;
		memset(&file_block, 0, sizeof(file_block));
		
		file_block = file_info.block_list[count];
		printf("\nDN IP: %s\n", file_block.loc_ip);
		printf("DN Port: %d\n", file_block.loc_port);

		//Allocate space for a datanode request and fill it
		dfs_cli_dn_req_t datanode_request;
		memset(&datanode_request, 0, sizeof(datanode_request));

		datanode_request.op_type = 0;
		datanode_request.block = file_block;

		//Create a socket to communicate with the datanode
		int datanode_socket = 0;
		datanode_socket = create_client_tcp_socket(file_block.loc_ip, file_block.loc_port);

		//Send request for a block to the datanode
		send_data(datanode_socket, &datanode_request, sizeof(datanode_request));

		//Allocate space for the datanode response
		dfs_cm_block_t returned_block;
		memset(&returned_block, 0, sizeof(returned_block));

		//Receive the block from the datanode and extract its content
		receive_data(datanode_socket, &returned_block, sizeof(returned_block));	

		strcpy(file_block.content, returned_block.content);

		block_list[count] = file_block;
		count++;
	}
	
	FILE *file = fopen(filename, "wb");

	//Re-assemble the received blocks into the complete file
	count = 0;
	while(count < num_blocks){

		//Print received blocks
		printf("Returned blocks\n");
		printf("\tOwner name: %s\n", block_list[count].owner_name);
		printf("\tDatanode ID: %d\n", block_list[count].dn_id);
		printf("\tBlock ID: %d\n", block_list[count].block_id);
//		printf("\tContent: %s\n", block_list[count].content);

		fwrite(&(block_list[count].content), DFS_BLOCK_SIZE, 1, file);
		count++;
	}
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
