README
This code implements a simple distributed file system using socket programming, inter-process communication and some multi-threaded programming. 

TO RUN
Namenode:
namenode <Port>

Datanode:
datanode <Port> <IP Address> <Datanode ID>

Client:
./dfs <IP Address> <Port>

ARCHITECTURE
Namenode: 
	-maintains information about the files which are stored in the system
	-holds the location of each block and which datanode stores it
	-responds to client requests to upload/download files to the system and returns the socket address of the datanode that stores or is storing the block

Datanode:
	-stores data blocks on its local file system
	-client uploads/downloads blocks directly to datanodes after being told which datanode to upload/download to by the namenode
	
Client:
	-sends requests to the namenode and sends/receives blocks to/from the datanodes

CONTACT
Ryan Singzon
ryan.singzon@mail.mcgill.ca