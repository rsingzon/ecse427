#include "datanode/dfs_datanode.h"
#include <assert.h>
#include <errno.h>
#include <unistd.h>

int lastHeartbeatStamp = 0;//last time the datanode contacted the namenode

int datanode_id = 0;
int datanode_listen_port = 0;
char *working_directory = NULL;

// main service loop of datanode
int mainLoop()
{
	//we don't consider concurrent operations in this assignment
	int server_socket = -1;

	//TODO: create a server socket and listen on it, you can implement dfs_common.c and call it here
	server_socket = create_server_tcp_socket(datanode_listen_port);
	assert (server_socket != INVALID_SOCKET);

	// Listen to requests from the clients
	for (;;)
	{
		struct sockaddr_in client_address;
		int client_address_length = sizeof(client_address);
		int client_socket = -1;

		//TODO: accept the client request
		printf("Waiting for client\n\n");
		client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_length);
		printf("CLIENT ACCEPTED!\n");
				
		assert(client_socket != INVALID_SOCKET);
		dfs_cli_dn_req_t request;
		//TODO: receive data from client_socket, and fill it to request

		receive_data(client_socket, &request, sizeof(request));

		requests_dispatcher(client_socket, request);
		close(client_socket);
	}
	close(server_socket);
	return 0;
}

static void *heartbeat()
{
	dfs_cm_datanode_status_t datanode_status;
	datanode_status.datanode_id = datanode_id;
	datanode_status.datanode_listen_port = datanode_listen_port;

	for (;;)
	{
		int heartbeat_socket = -1;
		//TODO: create a socket to the namenode, assign file descriptor id to heartbeat_socket
		heartbeat_socket = create_client_tcp_socket("127.0.0.1", 50030);
		assert(heartbeat_socket != INVALID_SOCKET);

		//send datanode_status to namenode
		int data_size;

		dfs_cm_datanode_status_t datanode_status;
		datanode_status.datanode_id = datanode_id;
		datanode_status.datanode_listen_port = htons(datanode_listen_port);

		data_size = sizeof(dfs_cm_datanode_status_t);

		send_data(heartbeat_socket, (void*) &datanode_status, data_size);

		close(heartbeat_socket);
		sleep(HEARTBEAT_INTERVAL);
	}
}

/**
 * start the service of data node
 * argc - count of parameters
 * argv - parameters
 */
int start(int argc, char **argv)
{
	char namenode_ip[32] = { 0 };
	assert(argc == 5);
	strcpy(namenode_ip, argv[2]);

	datanode_id = atoi(argv[3]);
	datanode_listen_port = atoi(argv[1]);
	working_directory = (char *)malloc(sizeof(char) * strlen(argv[4]) + 1);
	strcpy(working_directory, argv[4]);

	//start one thread to report to the namenode periodically
	//TODO: start a thread to report heartbeat
	printf("DATANODE START\n");
	pthread_t heartbeat_thread;
	pthread_t *thread_pointer = create_thread( heartbeat, NULL);
	heartbeat_thread = *thread_pointer;

	return mainLoop();
}

int read_block(int client_socket, const dfs_cli_dn_req_t *request)
{
	assert(client_socket != INVALID_SOCKET);
	assert(request != NULL);
	char buffer[DFS_BLOCK_SIZE];
	ext_read_block(request->block.owner_name, request->block.block_id, (void *)buffer);
	//TODO:response the client with the data

	dfs_cm_block_t block_to_return;
	memset(&block_to_return, 0, sizeof(block_to_return));

	strcpy(block_to_return.owner_name, request->block.owner_name);
	block_to_return.dn_id = datanode_id;
	block_to_return.block_id = request->block.block_id;
	strcpy(block_to_return.content, buffer);

	printf("Block to return\n");
	printf("\tOwner name: %s\n", block_to_return.owner_name);
	printf("\tDatanode ID: %d\n", block_to_return.dn_id);
	printf("\tBlock ID: %d\n", block_to_return.block_id);
	printf("\tContent: %s\n", block_to_return.content);

	send_data(client_socket, &block_to_return, sizeof(block_to_return));
	return 0;
}

int create_block(const dfs_cli_dn_req_t* request)
{
	ext_write_block(request->block.owner_name, request->block.block_id, (void *)request->block.content);
	return 0;
}

void requests_dispatcher(int client_socket, dfs_cli_dn_req_t request)
{
	switch (request.op_type)
	{
		case 0:
			read_block(client_socket, &request);
			break;
		case 1:
			create_block(&request);
			break;
	}
}

int main(int argc, char **argv)
{
	if (argc != 5)
	{
		printf("usage:datanode local_port namenode_ip id working_directory\n");
		return 1;
	}
	ext_init_local_fs(argv[4]);
	start(argc, argv);
	ext_close_local_fs();
	return 0;
}
