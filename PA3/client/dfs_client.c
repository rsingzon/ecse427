#include "client/dfs_client.h"
#include "datanode/ext.h"
#include  <sys/types.h>                        
#include  <sys/socket.h>  

/**
 * This process will run on the client server which will request modifications
 * to files, and push and pull them from the requested server
 */

//Connects to the namenode server
int connect_to_nn(char* address, int port)
{
	assert(address != NULL);
	assert(port >= 1 && port <= 65535);

	//create a socket and connect it to the server (address, port)
	//assign return value to client_socket 
	int client_socket = -1;

	client_socket = create_client_tcp_socket(address, port);
	if(client_socket < 0){
		error("Error opening socket!\n");
	}
	
	return client_socket;
}

//Modifies an existing file
int modify_file(char *ip, int port, const char* filename, int file_size, int start_addr, int end_addr)
{
	int namenode_socket = connect_to_nn(ip, port);
	if (namenode_socket == INVALID_SOCKET) return -1;
	FILE* file = fopen(filename, "rb");
	assert(file != NULL);

	//Request and send
	dfs_cm_client_req_t request;
	memset(&request, 0, sizeof(request));

	strcpy(request.file_name, filename);
	request.file_size = file_size;
	request.req_type = 3;

	send_data(namenode_socket, &request, sizeof(request));
	
	//Receive response
	dfs_cm_file_res_t response;
	memset(&response, 0, sizeof(response));

	receive_data(namenode_socket, &response, sizeof(response));

	dfs_cm_file_t file_info; 
	memset(&file_info, 0, sizeof(file_info));
	file_info = response.query_result;

	//Start writing at the block which contains start_addr
	int start_block_index = start_addr / DFS_BLOCK_SIZE;

	//End writing at the block which contains end_addr
	int end_block_index = end_addr / DFS_BLOCK_SIZE;
	int blocks_to_modify = end_block_index - start_block_index + 1;

	//Seek to the appropriate location in the file
	fseek(file, start_addr, SEEK_SET);

	//Send the updated block to the proper datanode
	int block_index = start_block_index;
	int count = 0;
	while( count < blocks_to_modify ){

		dfs_cm_block_t file_block;
		memset(&file_block, 0, sizeof(file_block));

		int datanode_socket = 0;

		//Get the IP and port from the namenode response
		file_block = file_info.block_list[block_index];

		//Read data from file into the block
		fread( &(file_block.content), DFS_BLOCK_SIZE, 1, file);

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

//Creates a new file at the server at the specified socket
int push_file(int namenode_socket, const char* local_path)
{
	assert(namenode_socket != INVALID_SOCKET);
	assert(local_path != NULL);
	FILE* file = fopen(local_path, "rb");
	assert(file != NULL);

	// Create the push request
	dfs_cm_client_req_t request;

	//fill the fields in the request
	strcpy(request.file_name, local_path);

	//Get the offset of the file pointer when it is at the end of the file
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	request.file_size = size;
	request.req_type = 1;

	//Send the request to the namenode
	send_data(namenode_socket, &request, sizeof(dfs_cm_client_req_t));

	//Receive the response
	dfs_cm_file_res_t response;
	memset(&response, 0, sizeof(response));

	receive_data(namenode_socket, &response, sizeof(dfs_cm_file_res_t));

	//Retrieve file info from the response
	dfs_cm_file_t file_info = response.query_result;

	//Send blocks to datanodes one by one
	int count = 0;
	while(count < file_info.blocknum){

		dfs_cm_block_t file_block;
		memset(&file_block, 0, sizeof(file_block));

		int datanode_socket = 0;

		//Get the IP and port from the namenode response
		file_block = file_info.block_list[count];

		//Read data from file into the block
		fread( &(file_block.content), DFS_BLOCK_SIZE, 1, file);

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

//Retrieves a file from the server at the specified socket address
int pull_file(int namenode_socket, const char *filename)
{
	assert(namenode_socket != INVALID_SOCKET);
	assert(filename != NULL);

	//Fill the request and send
	dfs_cm_client_req_t request;
	strcpy(request.file_name, filename);
	request.req_type = 0;

	send_data(namenode_socket, &request, sizeof(dfs_cm_client_req_t));

	//Allocate space for the response and receive it
	dfs_cm_file_res_t response;
	memset(&response, 0, sizeof(response));
	receive_data(namenode_socket, &response, sizeof(response));
	
	dfs_cm_file_t file_info;
	memset(&file_info, 0, sizeof(file_info));
	file_info = response.query_result;

	//Receive blocks from datanodes one by one
	int num_blocks = file_info.blocknum;
	int count = 0;

	//Allocate space for all the blocks for a particular file
	dfs_cm_block_t block_list[num_blocks];

	while (count < num_blocks){
		//Obtain the block information from the namenode
		dfs_cm_block_t file_block;
		memset(&file_block, 0, sizeof(file_block));
		
		file_block = file_info.block_list[count];

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

		fwrite(&(block_list[count].content), DFS_BLOCK_SIZE, 1, file);
		count++;
	}
	fclose(file);
	return 0;
}


//Prints system info
dfs_system_status *get_system_info(int namenode_socket)
{
	assert(namenode_socket != INVALID_SOCKET);
	
	//Fill the result and send 
	dfs_cm_client_req_t request;

	request.req_type = 2;
	send_data(namenode_socket, &request, sizeof(dfs_cm_client_req_t));
	
	//Get the response
	dfs_system_status *response; 
	response = malloc(sizeof(dfs_system_status));

	receive_data(namenode_socket, response, sizeof(dfs_system_status));

	return response;		
}

//Creates a request to send a file
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
