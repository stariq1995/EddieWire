#include "common.h"


#define FORWARDER_LISTEN 8888
#define LISTEN_PORT 8889

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
	sock = socket(AF_INET, SOCK_STREAM, 0);

	/*
	 * bind socket to port 8888 of the first available
	 * network interface
	 */
	struct sockaddr_in addr = { 0 };
	addr.sin_addr.s_addr = htons(INADDR_ANY);
	addr.sin_port = htons(LISTEN_PORT);
	bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	return sock;
}

int open_listen_socket_f() {
	int sock;

	/* 
	 * starting the socket with TCP	
	 */
	sock = socket(AF_INET, SOCK_STREAM, 0);

	/*
	 * bind socket to port 8888 of the first available
	 * network interface
	 */
	struct sockaddr_in addr = { 0 };
	addr.sin_addr.s_addr = htons(INADDR_ANY);
	addr.sin_port = htons(FORWARDER_LISTEN);
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

   		if (find_next(argv[3])) {
   			printf("Could not find the next node in the Network\n");
   			return -1;
   		}

   		int serverFD = connect_to_next_f(argv[4]);
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

    	int listen_sock = open_listen_socket_f();

			/*
			 * put socket into listening mode - THIS IS BLOCKING
			 */
		printf("Server running in %s mode..\n", argv[2]);

    	while (1) {

			
			printf("Listening...\n");
			listen(listen_sock, 5);

			clientFD = accept(listen_sock, (struct  sockaddr *) NULL, NULL);
			printf("New Client Instance.\n");
			time_t current_time = time(NULL);
			sprintf(filename, "tmp/f-%lu", (long unsigned)current_time);

			int r = receive_from_prev(clientFD, filename, &chunkSize);
			printf("Completed receiving. Searching for next...\n");

			if (find_next(argv[3])) {
   			printf("Could not find the next node in the Network\n");
   			return -1;
			}
			printf("Found, connecting...\n");
			int serverFD = connect_to_next(argv[3]);
			printf("Connected, sending...\n");
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

		    	int listen_sock = open_listen_socket();

			/*
			 * put socket into listening mode - THIS IS BLOCKING
			 */
		printf("Server running in %s mode..\n", argv[2]);

    	while (1) {

			
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
    ofile = fopen(filename, "w");
    if (ofile == NULL) perror ("Error Opening file:");
	gettimeofday(&startTime, NULL);

	/* 
	server just simply reads and writes what it read to the file.
	 */
	while (readBytes > 0 && size < insize) {
		readBytes = read(clientFD, buf, chunkSize);
//		printf("read bytes before loop : %d\n", readBytes);

		while (readBytes < chunkSize && (insize - size - readBytes) > 0){        
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

int find_next(char *address) {
	return 0;
	FILE *scan_results;
	char col1[100] = {0};
	char col2[100] = {0};
	char col3[100] = {0};
	char col4[100] = {0};
	system("wpa_cli scan");
	system("wpa_cli scan_results > temp_scan_results");
	scan_results = fopen("temp_scan_results", "r");
	if (scan_results == NULL){
		printf("File not present!\n");
		exit(0);
	}
	while(fscanf(scan_results, "%s %s %s %s", col1, col2, col3, col4) == 4) {
		if (!strcmp(col4, "EdiNet")){
			printf("Found %s at Mac : %s\n", col1, col4);
			return 0;
		}
	}
	printf("Adhoc Network not found\n");
	return 1;
}

int connect_to_next_f(char *address) {
	int serverFD, status;
	
	serverFD = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(FORWARDER_LISTEN);
	inet_pton(AF_INET, address, &addr.sin_addr);

	status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
	while (status < 0) {
		perror("Connection failure, retrying in 1 sec..");
		sleep(1);
		serverFD = socket(AF_INET, SOCK_STREAM, 0);
		status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
	}
	return serverFD;
}

int connect_to_next(char *address) {
	int serverFD, status;
	
	serverFD = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(LISTEN_PORT);
	inet_pton(AF_INET, address, &addr.sin_addr);

	status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
	while (status < 0) {
		perror("Connection failure, retrying in 1 sec..");
		sleep(1);
		serverFD = socket(AF_INET, SOCK_STREAM, 0);
		status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
	}
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
	if (close(serverFD) < 0) {
		printf("Error while disconnecting.\n");
		return -1;
	}
	return 0;
}