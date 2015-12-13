/** @file recv-server.c
 *  @brief the client used for testing data transfer
 *  
 *  Simple client that establishes a connection with a known server, 
 *  reads one of the test files then sends it to the server in chunks. The 
 *  client can run over TCP (wifi) or RFCOMM (bluetooth) as specified per 
 *  the first command line argument. The second command line argument specifies
 *  the chunksize and the third specifies whether chunks should be sent with 
 *  checksum appended to their end.
 *
 *  @author Aliaa Essameldin <aeahmed@qatar.cmu.edu>
 *  @bug No known bugs.
 */

#include "common.h"


#define SERVER_PORT 8888
unsigned short check_sum (unsigned short *buffer, int size); 

int main(int argc, char **argv) {
	/* 
	 * Check command-line arguments. Just checks count, not validity. 
     */
	if (argc != 6) {
        printf("Usage: %s <filename> <bluetooth/wifi>", argv[0]);
		printf(" <server address> <Chunk Size> <checksum/no-checksum>)\n");
        return -1;
    }

	struct stat st;
    int chunkSize, size, serverFD, fileFD, status, writeBytes, errorCheck;
    char *buf;

	/*
	 * Preparing the file to be sent 
	 */
    fileFD = open(argv[1], O_RDWR);
    if (fileFD < 0) perror ("Error opening file:");
    fstat(fileFD, &st);
    size = st.st_size;

	/* 
	 * allocating memory for buffer according to chunk size and whether 
	 * a checksum will be appended 
	 */
	chunkSize = atoi(argv[4]);	
	if (chunkSize < 0){
		printf("Chunk Size invalid.\n");
		printf("Usage: %s <filename> <bluetooth/wifi>", argv[0]);
		printf(" <server address> <Chunk Size> <checksum/no-checksum>)\n");
        return -1;
	} else if (!strcmp(argv[5],"no-checksum")){
		buf = (char *)malloc(sizeof(char)*chunkSize);
		errorCheck = 0;
	} else if (!strcmp(argv[5],"checksum")){
		buf = (char *)malloc(sizeof(char)*chunkSize + sizeof(unsigned short));
		errorCheck = 1;
	} else {
		printf("Checksum argument invalid.\n");
		printf("Usage: %s <filename> <'bluetooth'/'wifi'>", argv[0]);
		printf(" <server address> <Chunk Size> <'checksum'/'no-checksum'>)\n");
        return -1;
    }

	/* 
	 * Check the wifi/bluetooth argument and the address argument then
	 * start socket, declare socket address type and populate the address
	 * accordingly. 
	 * 
     * Then it attempts to the convenient address an inifinite number of times
	 * until one successful connection is established
	 */ 
	if(!strcmp(argv[2], "wifi")){
        serverFD = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr = { 0 };
        addr.sin_family = AF_INET;
        addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, argv[3], &addr.sin_addr);
		
		status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
		while (status < 0){
			perror("Connection failure, retrying in 1 sec..");
			sleep(1);
			serverFD = socket(AF_INET, SOCK_STREAM, 0);
			status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
		}
    } else if (!strcmp(argv[2], "bluetooth")) {
        serverFD = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	 	struct sockaddr_rc addr = { 0 };
        addr.rc_family = AF_BLUETOOTH;
        addr.rc_channel = (uint8_t) 1;
        str2ba(argv[3], &addr.rc_bdaddr);

		status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
		while (status < 0){
			perror("Connection failure, retrying in 1 sec..");
			sleep(1);
			serverFD = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
			status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
		}
    } else {
		struct sockaddr_in addr = { 0 }; // to silent compilation warnings
		printf("wifi/bluetooth argument invalid.\n");
		printf("Usage: %s <filename> <'bluetooth'/'wifi'>", argv[0]);
		printf(" <server address> <Chunk Size> <'checksum'/'no-checksum'>)\n");
        return -1;
	}

	/* 
	 * The first message it sends to the servre instance specifies: 
 	 *  	- File size
 	 *		- Chunk size
 	 *		- Whether a checksum is included
	 */
	sprintf(buf, "%i %i %i", chunkSize, size, errorCheck);
    write(serverFD, buf, strlen(buf));
	sleep(3);

	/*
	 * Sending the files in chunks of size ChunkSize, either raw or with the
	 * checksum appended to the end of the buffer
	 */
	int total; 
	writeBytes = read(fileFD, buf, chunkSize);
	union csum checksum;
	char replyBuf[10];
	int total_sent = 0;
	int x = writeBytes;
        while (writeBytes > 0) {
		if (errorCheck) {
			checksum.i = check_sum((unsigned short *)buf, writeBytes);
			buf[writeBytes] = checksum.c[0];
			buf[writeBytes+1] = checksum.c[1];
        	        status = write(serverFD, buf, writeBytes + 2);
			if (status < 0) perror("Sending Error:");
			
			status = read(serverFD, replyBuf, sizeof(replyBuf));
			if (status < 0) perror("Reading reply error:");
			while(!strcmp(replyBuf, "NO")){
				printf("Retransmitting corrupted packet\n");
				status = write(serverFD, buf, writeBytes + 2);
				if (status < 0) perror("Sending Error:");
				status = read(serverFD, replyBuf, sizeof(replyBuf));
				if (status < 0) perror("Reading reply error:");
			}
			
		} else {
			status = write(serverFD, buf, writeBytes);
			total_sent += status;
			printf("total_sent : %d, status : %d\n", total_sent, status);
			if (status < 0) perror("Sending Error:");
		}
                writeBytes = read(fileFD, buf, chunkSize);
		x+=writeBytes; 
		//printf("X: %i\n", x);
		
	}
	
	if(writeBytes < 0) perror("Reading Error:");
	
	/* 
	 * Cleanup and close, make sure not to close the serverFD to protect
	 * last packet (??)
	 */
	sleep(2);
    close(fileFD);
    free(buf);
    return 0;
}

/* 
 * @brief TCP Checksum implementation copied from 
 * http://stackoverflow.com/questions/8845178/c-programming-tcp-checksum
 */
unsigned short check_sum(unsigned short *buffer, int size)
{
    unsigned long cksum=0;
    while(size >1)
    {
        cksum+=*buffer++;
        size -=sizeof(unsigned short);
    }
    if(size)
        cksum += *(unsigned char*)buffer;

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16);
    return (unsigned short)(~cksum);
}

