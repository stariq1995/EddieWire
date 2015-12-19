#include "common.h"
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>



int receive_from_prev(int clientFD, char *filename, int *cSize);
int find_next();
int connect_to_next();
int send_to_next();
int disconnect_from_next();
int open_listen_socket();


char logfile[32] = {0};

int open_listen_socket() {
	int sock;

	/* 
	 * starting the socket with TCP	
	 */
	sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	/*
	 * bind socket to port 8888 of the first available
	 * network interface
	 */
	struct sockaddr_rc addr = { 0 };
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	addr.rc_bdaddr = *BDADDR_ANY;
	bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	return sock;
}

int main(int argc, char **argv) {
	int chunkSize, role, clientFD;
	char filename[32] = {0};
	char address[32] = {0};

	/* 
	 * Check command-line arguments. Just checks count, not validity. 
     */
	if (argc < 3) {
        printf("Usage: %s <log file name> <role : 1 = sender | 2 = forwarder | 3 = receiver>\n", argv[0]);
        return -1;
    }

    role = atoi(argv[2]);
    sprintf(logfile, "%s", argv[1]);


    if (role < 1 || role > 3) {
    	printf("Usage: %s <log file name> <role : 1 = sender, 2 = forwarder, 3 = receiver>\n", argv[0]);
        return -1;
    }

    if (role == 1) {
    	if (argc != 6) {
    		printf("Usage: %s <log file name> 1 <filename> <server address> <chunk size>\n", argv[0]);
	        return -1;
    	}


    	chunkSize = atoi(argv[5]);
   		sprintf(filename, "%s", argv[3]);

   		if (find_next(argv[4])) {
   			printf("Could not find the next node in the Network\n");
   			return -1;
   		}

   		int serverFD = connect_to_next(argv[4]);
   		int sent = send_to_next(serverFD, chunkSize, filename);
   		int dc = disconnect_from_next(serverFD);
   		printf("Sender operation complete\n");
   		return 0;
    }

    if (role == 2) {
    	if (argc != 4) {
    		printf("Usage: %s <log file name> 2 <server address>\n", argv[0]);
	        return -1;
    	}

    	system("hciconfig hci0 piscan");

    	int listen_sock = open_listen_socket();

			/*
			 * put socket into listening mode - THIS IS BLOCKING
			 */
		printf("Server running in %s mode..\n", argv[2]);

    	while (1) {

			// system("./code/discovery_on.sh");
			printf("Listening...\n");
			listen(listen_sock, 5);

			clientFD = accept(listen_sock, (struct  sockaddr *) NULL, NULL);
			printf("New Client Instance.\n");
			time_t current_time = time(NULL);
			sprintf(filename, "tmp/f-%lu.tmp", (long unsigned)current_time);

			int r = receive_from_prev(clientFD, filename, &chunkSize);
			printf("Completed receiving. Searching for next...\n");

			if (find_next(argv[3])) {
   			printf("Could not find the next node in the Network\n");
   			return -1;
			}
			printf("Found, connecting...\n");
			int serverFD = connect_to_next(argv[3]);
			printf("Connected, sending...\n");
			printf("File: %s\n", filename);
			int sent = send_to_next(serverFD, chunkSize, filename);
			int dc = disconnect_from_next(serverFD);
			printf("Sender operation complete\n");
			char cmd[32] = {0};
			sprintf(cmd, "rm -f %s", filename);
			system(cmd);
		}
	}

	if (role == 3) {
		if (argc != 3) {
			printf("Usage: %s <log file name> 3\n", argv[0]);
	        return -1;
		}
			system("hciconfig hci0 piscan");
		    	int listen_sock = open_listen_socket();

			/*
			 * put socket into listening mode - THIS IS BLOCKING
			 */
		printf("Server running in %s mode..\n", argv[2]);

    	while (1) {

			// system("./code/discovery_on.sh");
			printf("Listening...\n");
			listen(listen_sock, 5);

			clientFD = accept(listen_sock, (struct  sockaddr *) NULL, NULL);
			printf("New Client Instance.\n");
			time_t current_time = time(NULL);
			sprintf(filename, "tmp/f-%lu", (long unsigned)current_time);

			int r = receive_from_prev(clientFD, filename, &chunkSize);
			printf("Completed receiving.\n");
			char cmd[32] = {0};
			sprintf(cmd, "rm -f %s", filename);
			system(cmd);
		}
	}
	return 0;
}

int receive_from_prev(int clientFD, char *filename, int *cSize) {
	char *buf;
	char temp_buf[1024] = { 0 };
	int chunkSize, status, readBytes, size, insize;
	struct timeval startTime, endTime;
    double delay;

    FILE *log, *ofile;

	readBytes = read(clientFD, temp_buf, 21);

    if (readBytes == 0);

    if (readBytes < 0) perror ("First read error:");

    sscanf(temp_buf, "%i %i", &chunkSize, &insize);
    printf("[%i] Chunk: %i\n", insize, chunkSize);
    *cSize = chunkSize;

	buf = (char *)malloc(sizeof(char)*chunkSize);

	/* 
	 * Initialize connection, open logfile, output file then start timer
	 */
	memset(buf, 0, chunkSize);

    delay = 0;
	size = 0;

    log = fopen(logfile, "a");	
    ofile = fopen(filename, "a");
    if (ofile == NULL) perror ("Error Opening file:");
	gettimeofday(&startTime, NULL);

	/* 
	server just simply reads and writes what it read to the file.
	 */
	while (readBytes > 0 && size < insize) {
		readBytes = read(clientFD, buf, chunkSize);
//		printf("read bytes before loop : %d\n", readBytes);

		while (readBytes < chunkSize && (insize - size - readBytes) > 0 && readBytes > 0){        
			readBytes += read(clientFD, buf+readBytes, chunkSize-readBytes);
//			printf("read bytes in loop:%d out of remaining: %d\n", readBytes, insize - size - readBytes);
		}

		size += readBytes;
		//printf("size : %d, readbytes: %d\n", size, readBytes);
        status = fwrite(buf, sizeof(char), readBytes, ofile);
        if (status < 0) perror ("Error writing:");
	}
	
	if (readBytes<0) perror("Error receiving:");

	/* 
	 * Done receiving data, stop timer and calculate delay
	 */
    gettimeofday(&endTime, NULL);
    delay = ((endTime.tv_sec * 1000000) + (endTime.tv_usec)) - ((startTime.tv_sec * 1000000) + (startTime.tv_usec));
    fprintf(log, "Receive Delay : %i %f\n", size, delay);
    fclose(ofile);
    fclose(log);
    return 0;
}

int scan(int len, char *find_addr) {
    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };
	flags = IREQ_CACHE_FLUSH;
    dev_id = hci_get_route(NULL);
    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        exit(1);
    }

    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
    
    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) perror("hci_inquiry");

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
        printf("%s == %s\n", addr, find_addr);
        if (!strcmp(find_addr, addr)){
        	return 1;
        }
    }

    free( ii );
    close( sock );
    return 0;
}

int find_next(char *address) {

	struct timeval startTime, endTime;
    double delay;
    FILE *log;


	log = fopen(logfile, "a");
	gettimeofday(&startTime, NULL);
	char cmd[100] = {0};
	sprintf(cmd, "./code/bt_find.py %s", address);
	system(cmd);


	gettimeofday(&endTime, NULL);
	
    delay = ((endTime.tv_sec * 1000000) + (endTime.tv_usec)) - ((startTime.tv_sec * 1000000) + (startTime.tv_usec));
    fprintf(log, "Scan Delay : %f\n", delay);
	fclose(log);
	
	return 0;
}

// int find_next(char *addr)
// {
// 	struct timeval startTime, endTime;
// 	double delay;
// 	FILE *log;
// 	int i;

// 	log = fopen(logfile, "a");
// 	gettimeofday(&startTime, NULL);

// 	for (i = 2; i < 9; i++) {
// 		if (scan(i, addr)) {
// 			gettimeofday(&endTime, NULL);
// 		    delay = ((endTime.tv_sec * 1000000) + (endTime.tv_usec)) - ((startTime.tv_sec * 1000000) + (startTime.tv_usec));
// 		    fprintf(log, "Scan Delay : %f\n", delay);
// 			fclose(log);
// 			return 0;
// 		}
// 	}

// 	while (1) {
// 		if (scan(i, addr)) {
// 			gettimeofday(&endTime, NULL);
// 		    delay = ((endTime.tv_sec * 1000000) + (endTime.tv_usec)) - ((startTime.tv_sec * 1000000) + (startTime.tv_usec));
// 		    fprintf(log, "Scan Delay : %f\n", delay);
// 			fclose(log);
// 			return 0;
// 		}
// 	}
// }

int connect_to_next(char *address) {
	int serverFD, status;
	struct timeval startTime, endTime;
	double delay;
	FILE *log;
	
	serverFD = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	struct sockaddr_rc addr = { 0 };
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba(address, &addr.rc_bdaddr);

	log = fopen(logfile, "a");
	gettimeofday(&startTime, NULL);

	status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
	while (status < 0) {
		perror("Connection failure, retrying in 1 sec..");
		sleep(1);
		serverFD = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
		status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
	}

	gettimeofday(&endTime, NULL);
    delay = ((endTime.tv_sec * 1000000) + (endTime.tv_usec)) - ((startTime.tv_sec * 1000000) + (startTime.tv_usec));
    fprintf(log, "Connect Delay : %f\n", delay);
	fclose(log);

	return serverFD;
}

int send_to_next(int serverFD, int chunkSize, char *filename) {
	char *buf;
	struct stat st;
    int size, fileFD, status, writeBytes;

	fileFD = open(filename, O_RDWR);
    if (fileFD < 0) perror ("Error opening file:");
    fstat(fileFD, &st);
    size = st.st_size;

	buf = (char *)calloc(sizeof(char), chunkSize);
	sprintf(buf, "%10i %10i", chunkSize, size);
	printf("buf_length = %d\n", (int)strlen(buf));
    write(serverFD, buf, strlen(buf));


    /*
	 * Sending the files in chunks of size ChunkSize, either raw or with the
	 * checksum appended to the end of the buffer
	 */
	int total; 
	writeBytes = read(fileFD, buf, chunkSize);
	int total_sent = 0;
	int x = writeBytes;
	while (writeBytes > 0) {
		status = write(serverFD, buf, writeBytes);
		total_sent += status;
		//printf("total_sent : %d, status : %d\n", total_sent, status);
		if (status < 0) perror("Sending Error:");
		writeBytes = read(fileFD, buf, chunkSize);
		x+=writeBytes; 
	}
	
	if(writeBytes < 0) perror("Reading Error:");
	
	/* 
	 * Cleanup and close, make sure not to close the serverFD to protect
	 * last packet (??)
	 */
    close(fileFD);
    free(buf);
    return 0;
}

int disconnect_from_next(int serverFD) {

	struct timeval startTime, endTime;
	FILE *log;
	double delay;

	log = fopen(logfile, "a");
	gettimeofday(&startTime, NULL);

	if (close(serverFD) < 0) {
		printf("Error while disconnecting.\n");

		gettimeofday(&endTime, NULL);
	    delay = ((endTime.tv_sec * 1000000) + (endTime.tv_usec)) - ((startTime.tv_sec * 1000000) + (startTime.tv_usec));
		fprintf(log, "Disconnect Delay : %f\n--\n", delay);
		fclose(log);

		return -1;
	}

	gettimeofday(&endTime, NULL);
    delay = ((endTime.tv_sec * 1000000) + (endTime.tv_usec)) - ((startTime.tv_sec * 1000000) + (startTime.tv_usec));
    fprintf(log, "Disconnect Delay : %f\n--\n", delay);
	fclose(log);

	return 0;
}