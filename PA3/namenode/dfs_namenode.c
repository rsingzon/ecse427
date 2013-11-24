#include "namenode/dfs_namenode.h"
#include <assert.h>
#include <unistd.h>

dfs_datanode_t* dnlist[MAX_DATANODE_NUM];
dfs_cm_file_t* file_images[MAX_FILE_COUNT];
int fileCount;
int dncnt = 0;
int safeMode = 1;

int mainLoop(int server_socket)
{
	while (safeMode == 1)
	{
		printf("the namenode is running in safe mode\n");
		sleep(5);
	}

	for (;;)
	{
		sockaddr_in client_address;
		unsigned int client_address_length = sizeof(client_address);
		int client_socket = -1;
		//TODO: accept the connection from the client and assign the return value to client_socket
		printf("Waiting for client\n\n");
		client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_length);

		assert(client_socket != INVALID_SOCKET);

		dfs_cm_client_req_t request;
		//TODO: receive requests from client and fill it in request
		receive_data(client_socket, &request, sizeof(dfs_cm_client_req_t));

		printf("CLIENT REQUEST RECEIVED\n");
		if(request.req_type == 0){
			printf("\tType: READ\n");	
			printf("\tFile name: %s\n", request.file_name);
		} else if(request.req_type == 1){
			printf("\tType: WRITE");	
			printf("\tFile name: %s\n", request.file_name);
			printf("\tFile size: %d\n", request.file_size);
		} else if(request.req_type == 2){
			printf("\tType: QUERY\n");	
		} else if(request.req_type == 3){
			printf("\tType: MODIFY\n");	
		}
 		
		requests_dispatcher(client_socket, request);
		close(client_socket);
	}
	return 0;
}

static void *heartbeatService()
{
	int socket_handle = create_server_tcp_socket(50030);
	register_datanode(socket_handle);
	close(socket_handle);
	return 0;
}


/**
 * start the service of namenode
 * argc - count of parameters
 * argv - parameters
 */
int start(int argc, char **argv)
{
	printf("NAMENODE START\n");

	assert(argc == 2);
	int i = 0;
	for (i = 0; i < MAX_DATANODE_NUM; i++) dnlist[i] = NULL;
	for (i = 0; i < MAX_FILE_COUNT; i++) file_images[i] = NULL;

	//TODO:create a thread to handle heartbeat service
	//you can implement the related function in dfs_common.c and call it here
	pthread_t heartbeat_thread;
	pthread_t *thread_pointer = create_thread( heartbeatService, NULL);
	heartbeat_thread = *thread_pointer;
	printf("Thread created\n");

	int server_socket = INVALID_SOCKET;
	//TODO: create a socket to listen the client requests and replace the value of server_socket with the socket's fd
	int namenode_listen_port = atoi(argv[1]);
	server_socket = create_server_tcp_socket(namenode_listen_port);

	assert(server_socket != INVALID_SOCKET);
	
	return mainLoop(server_socket);
}

int register_datanode(int heartbeat_socket)
{
	for (;;)
	{
		int datanode_socket = -1;

		struct sockaddr_in client_addr;
		int client_address_length = sizeof(client_addr);

		//TODO: accept connection from DataNodes and assign return value to datanode_socket;
		int result = accept(heartbeat_socket, (struct sockaddr*) &client_addr, &client_address_length);

		if(result < 0){
			perror("Datanode could not be registered!\n");
		} else {
			datanode_socket = result;
		}	

		assert(datanode_socket != INVALID_SOCKET);
		dfs_cm_datanode_status_t datanode_status;
		//TODO: receive datanode's status via datanode_socket

		receive_data(datanode_socket, &datanode_status, sizeof(dfs_cm_datanode_status_t));

		if (datanode_status.datanode_id < MAX_DATANODE_NUM)
		{

			//TODO: fill dnlist
			//principle: a datanode with id of n should be filled in dnlist[n - 1] (n is always larger than 0)

			dfs_datanode_t *datanode;	
			int dnlist_index = datanode_status.datanode_id - 1;

			//If there does not exist a datanode at the specified index, allocate 
			//space in memory for it
			if(dnlist[dnlist_index] == NULL){
				
				//Increment the number of datanodes that exist
				dncnt++;
				datanode = malloc(sizeof(dfs_datanode_t));
				memset(datanode, 0, sizeof(dfs_datanode_t));
				dnlist[dnlist_index] = datanode;
			}

			//Otherwise, set the working datanode to the correct address in the array
			datanode = dnlist[dnlist_index];
			datanode->dn_id = datanode_status.datanode_id;
			datanode->port = ntohs(datanode_status.datanode_listen_port);

			//Index of datanode list corresponds to the datanode_id-1			

			struct sockaddr *dn_addr;
			dn_addr = malloc(sizeof(sockaddr));
			struct sockaddr_in dn_addr_in;
			int addr_length = sizeof(*dn_addr);

			if( getpeername(datanode_socket, dn_addr, &addr_length) < 0){
				perror("Server: Could not get peer name!\n");
			}

			dn_addr_in = *(struct sockaddr_in*) dn_addr;
			struct in_addr dn_ip = dn_addr_in.sin_addr;
			char *dn_ip_ascii = inet_ntoa(dn_ip);

			strcpy(datanode->ip, dn_ip_ascii);

			safeMode = 0;
			free(dn_addr);

/*			//Print datanode information
			printf("\n\nDATANODE INFORMATION: \n");
			count = 0;
			while(count < MAX_DATANODE_NUM){
				if(dnlist[count] != NULL){
					dfs_datanode_t dn;
					dn = *(dnlist[count]);
					printf("Index %d\n", count);
					printf("\tDN ID: %d\n", dn.dn_id);
					printf("\tIP: %s\n", dn.ip);
					printf("\tPORT: %d\n\n", dn.port);
				}
				count++;
			}
*/
		} else{
			printf("Datanode ID [%d] out of bounds\n",datanode_status.datanode_id);
		}
		
		close(datanode_socket);	
	}
	return 0;
}

int get_file_receivers(int client_socket, dfs_cm_client_req_t request)
{
	printf("Responding to request for block assignment of file '%s'!\n", request.file_name);

	dfs_cm_file_t** end_file_image = file_images + MAX_FILE_COUNT;
	dfs_cm_file_t** file_image = file_images;
	
	// Try to find if there is already an entry for that file
	while (file_image != end_file_image)
	{
		if (*file_image != NULL && strcmp((*file_image)->filename, request.file_name) == 0){
			printf("FILE ALREADY EXISTS\n");
			break;	
		} 
		++file_image;
	}

	if (file_image == end_file_image)
	{
		// There is no entry for that file, find an empty location to create one
		file_image = file_images;
		while (file_image != end_file_image)
		{
			if (*file_image == NULL) break;
			++file_image;
		}

		if (file_image == end_file_image) return 1;
		// Create the file entry
		printf("CREATING NEW FILE\n");
		*file_image = (dfs_cm_file_t*)malloc(sizeof(dfs_cm_file_t));
	   	//TA BUG
	  //memset(*file_image, 0, sizeof(*file_image));
		memset(*file_image, 0, sizeof(**file_image));
		strcpy((*file_image)->filename, request.file_name);
		(*file_image)->file_size = request.file_size;
		(*file_image)->blocknum = 0;
	}
	
	int block_count = (request.file_size + (DFS_BLOCK_SIZE - 1)) / DFS_BLOCK_SIZE;
	
	int first_unassigned_block_index = (*file_image)->blocknum;
	int numIterations = block_count + first_unassigned_block_index;
	(*file_image)->blocknum = block_count;
	int next_data_node_index = 0;

	//TODO:Assign data blocks to datanodes, round-robin style (see the Documents)
	printf("\nBlocks to store: %d\n", block_count);
	printf("Block num: %d\n", (*file_image)->blocknum);
	printf("Unassigned: %d\n", first_unassigned_block_index);
	printf("Iterations needed: %d\n", numIterations);
	while(first_unassigned_block_index < numIterations){
		next_data_node_index = next_data_node_index % MAX_DATANODE_NUM;
		//Find a valid datanode
		while(dnlist[next_data_node_index] == NULL){
			printf("Incrementing dnlist index\n");
			next_data_node_index = (next_data_node_index + 1) % MAX_DATANODE_NUM;
		}

		//Allocate the datanode information
		dfs_cm_block_t file_block;

		strcpy(file_block.owner_name, request.file_name);
		
		//The indices of the datanode list correspond to their id-1
		file_block.dn_id = next_data_node_index + 1;
		file_block.block_id = first_unassigned_block_index;
		strcpy(file_block.loc_ip, dnlist[next_data_node_index]->ip);
		file_block.loc_port = dnlist[next_data_node_index]->port;
		printf("\tDatanode ID: %d\n", file_block.dn_id);
		printf("\tBlock ID:%d\n", file_block.block_id);
		printf("\tIP: %s\n", file_block.loc_ip);
		printf("\tPort: %d\n", file_block.loc_port);

		(*file_image)->block_list[first_unassigned_block_index] = file_block;

		first_unassigned_block_index++;
		next_data_node_index++;
	}

	//TODO: fill the response and send it back to the client
	dfs_cm_file_res_t *response;
	response = malloc(sizeof(dfs_cm_file_res_t));

	response->query_result = **file_image;

	printf("File name: %s\n", response->query_result.filename);
	printf("File size: %d\n", response->query_result.file_size);
	printf("Blocknum: %d\n", response->query_result.blocknum);

	send_data(client_socket, response, sizeof(dfs_cm_file_res_t));
	printf("Response to client sent!\n");
	free(response);
	return 0;
}

int get_file_location(int client_socket, dfs_cm_client_req_t request)
{
	int i = 0;

	// Loop searches for the file name within the list of files
	for (i = 0; i < MAX_FILE_COUNT; ++i)
	{
		dfs_cm_file_t* file_image = file_images[i];
		if (file_image == NULL) continue;
		if (strcmp(file_image->filename, request.file_name) != 0) continue;
		
		//Once a matching name has been found, this clode block is executed

		//Fill the response and send it back to the client
		dfs_cm_file_res_t *response;
		response = malloc(sizeof(dfs_cm_file_res_t));

		memset(response, 0, sizeof(dfs_cm_file_res_t));

//		dfs_cm_file_t query;

//		strcpy(query.filename, request.file_name);
//		query.file_size = file_image->file_size;

	//	dfs_cm_file_t
	//	char filename[256];
//		dfs_cm_block_t block_list[MAX_FILE_BLK_COUNT];
//		int file_size;
//		int blocknum;

		response->query_result = *file_image;

		printf("FILE FOUND\n");
		printf("\tFilename: %s\n", response->query_result.filename);
		printf("\tFile size: %d\n", response->query_result.file_size);
		printf("\tNumber of blocks: %d\n", response->query_result.blocknum);

		send_data(client_socket, response, sizeof(*response));

		printf("Read response sent to client!\n");
		free(response);
		return 0;
	}

	//FILE NOT FOUND
	return 1;
}

void get_system_information(int client_socket, dfs_cm_client_req_t request)
{
	assert(client_socket != INVALID_SOCKET);
	
	//TODO:fill the response and send back to the client	
	dfs_system_status *system_status;
	system_status = malloc(sizeof(dfs_system_status));
	memset(system_status, 0, sizeof(dfs_system_status));
	system_status->datanode_num = dncnt;

	printf("Number of datanodes: %d\n\n", system_status->datanode_num);

	send_data(client_socket, (void*)system_status, sizeof(dfs_system_status));
	free(system_status);
}

int get_file_update_point(int client_socket, dfs_cm_client_req_t request)
{
	int i = 0;
	for (i = 0; i < MAX_FILE_COUNT; ++i)
	{
		dfs_cm_file_t* file_image = file_images[i];
		if (file_image == NULL) continue;
		if (strcmp(file_image->filename, request.file_name) != 0) continue;
		dfs_cm_file_res_t response;
		//TODO: fill the response and send it back to the client
		// Send back the data block assignments to the client
		memset(&response, 0, sizeof(response));
		//TODO: fill the response and send it back to the client
		return 0;
	}
	//FILE NOT FOUND
	return 1;
}

int requests_dispatcher(int client_socket, dfs_cm_client_req_t request)
{
	//0 - read, 1 - write, 2 - query, 3 - modify
	printf("*******\nRequest type: %d\n", request.req_type);
	switch (request.req_type)
	{
		case 0:
			get_file_location(client_socket, request);
			break;
		case 1:
			get_file_receivers(client_socket, request);
			break;
		case 2:
			get_system_information(client_socket, request);
			break;
		case 3:
			get_file_update_point(client_socket, request);
			break;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int i = 0;
	for (; i < MAX_DATANODE_NUM; i++)
		dnlist[i] = NULL;
	return start(argc, argv);
}
