/** @file recv-server.c
 *  @brief the server used for testing data transfer
 *  
 *  Simple server that receives one of the test files and compares them to its
 *  local copy of the same files. The server can run over TCP (wifi) or RFCOMM 
 *  (bluetooth) as specified per the first command line argument. The second 
 *  command line argument specifies a starting transmission ID. This is usually
 *  1, unless a previous experiment was interrupted and needs to be resumed 
 *  from a certain point. 
 *
 *  @author Aliaa Essameldin <aeahmed@qatar.cmu.edu>
 */

#include "common.h"

unsigned short check_sum (unsigned short *buffer, int size); 

int main(int argc, char **argv) {
	/* 
	 * Check command-line arguments. Just checks count, not validity. 
     */
    if(argc != 4){
        printf("Usage: %s <bluetooth/wifi> <starting ID> <log filename>\n", argv[0]);
        return -1;
    }

	struct sockaddr_in addr = { 0 };		
    
	
    char temp_buf[1024] = { 0 };
    char *buf;
    int chunkSize, id, status, sock, clientFD, readBytes, size, insize, errorCheck;
    FILE *log, *ofile;

	/* 
	 * Initializing socket according to command-line argument, first
     * command-line argument's validity is checked here. 
	 */
	if(!strcmp(argv[1],"bluetooth")){
		/*
		 * starting the socket with RFCOMM 
		 */
		sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

        /*
		 * bind socket to channel 1 of the first available
         * local bluetooth adapter
		 */
		struct sockaddr_rc addr = { 0 };
        addr.rc_family = AF_BLUETOOTH;
        addr.rc_bdaddr = *BDADDR_ANY;
        addr.rc_channel = (uint8_t) 1;
        bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	} else if(!strcmp(argv[1],"wifi")){
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
        addr.sin_port = htons(8888);
        bind(sock, (struct sockaddr *)&addr, sizeof(addr));	
	} else {
		printf("Invalid argument %s\n", argv[1]);
        printf("Usage: %s <bluetooth/wifi> <id>\n", argv[0]);
        return -1;
	}

	/*
	 * put socket into listening mode - THIS IS BLOCKING
 	 */
    printf("Server running in %s mode..\n", argv[1]);
    listen(sock, 10);
    id = atoi(argv[2]);
	
	/* 
	 * Start main loop - exits if id was invalid or overflows
	 */
	while(id >= 0){
		id++;
		time_t startTime, endTime;
        double delay;

		/* 
		 * Block until new client instance connects
 		 */ 
		printf("Listening..\n");
		clientFD = accept(sock, (struct  sockaddr *) NULL, NULL);
		printf("New Client Instance.\n");

		/*
		 *  The first message it receives from each client instance specifies: 
 		 *  	- File size
 		 *		- Chunk size
 		 *		- Whether a checksum is included
		 */
		readBytes = read(clientFD, temp_buf, sizeof(temp_buf));
        if (readBytes == 0) continue; // Just an unread connection close
        if (readBytes < 0) perror ("First read error:");
        sscanf(temp_buf, "%i %i %i", &chunkSize, &insize, &errorCheck);
        printf("[%i] Chunk: %i - Checksum: %i\n", insize, chunkSize, errorCheck);
		if (errorCheck) buf = (char *)malloc(sizeof(char)*chunkSize + sizeof(unsigned short));
		else buf = (char *)malloc(sizeof(char)*chunkSize);

		/* 
		 * Initialize connection, open logfile, output file then start timer
		 */
		memset(buf, 0, sizeof(buf));
        delay = 0;
		size = 0;
        log = fopen(argv[3], "a");	
		char filename[32];
        sprintf(filename, "tmp/%i", id);
        ofile = fopen(filename, "w");
        if (ofile == NULL) perror ("Error Opening file:");
		time(&startTime);

		/* 
		 * Reading/writing behavior changes according to whether there is a checksum. 
		 * In case of checksum, server reads, compares to checksum (appended to the 
		 * end of the buffer and replies "OK" if chunk is correct and "NO" otherwise.
		 * In the case of a NO, the server would expect to recieve the same chunk again.
		 *
		 * Otherwise, server just simply reads and writes what it read to the file.
		 */
		if (errorCheck){
			char *OK = "OK";
			char *NO = "NO";
			unsigned short checkS;

		    while (readBytes > 0 && size < insize) {
				readBytes = read(clientFD, buf, sizeof(buf));
				readBytes -= sizeof(unsigned short);
				if (readBytes < 0) perror("Error receiving:");
				/* 
				 * Checking chunk and reacting accordingly 
				 */
				checkS = *((unsigned short *)(buf + readBytes));
				
				if ( checkS == check_sum((unsigned short *)buf, readBytes)){
					status = fwrite(buf, sizeof(char), readBytes, ofile);
		        	if (status < 0) perror ("Error writing to file:");
					status = write(clientFD, OK, sizeof(OK));
					if (status < 0) perror ("Error sending reply:");
					size += readBytes;
				} else {
					printf("Packet corrupted!\n");
					printf("CheckS is %d but calculated %d\n", checkS, check_sum((unsigned short *)buf, readBytes));
					return -1;
					status = write(clientFD, NO, sizeof(NO));
					if (status < 0) perror ("Error sending reply:");
				}
		    }
		} else {
			readBytes = read(clientFD, buf, sizeof(buf));
		    size += readBytes;
		    while (readBytes > 0 && size < insize) {
		        status = fwrite(buf, sizeof(char), readBytes, ofile);
		        if (status < 0) perror ("Error writing:");
		        readBytes = read(clientFD, buf, sizeof(buf));
		        size += readBytes;
		    }
		    if (readBytes<0) perror("Error receiving:");
		}

		/* 
		 * Done receiving data, stop timer and calculate delay
		 */
        time(&endTime);
        delay = difftime(endTime, startTime);
        fprintf(log, "%i: %i %f\n", id, size, delay*1000);
        fclose(ofile);
        fclose(log);

		/* 
		 * Run cmp on the received files and record count of corrupted bytes.
		 * In case of incomplete or bogus file transmission, write -1.
		 * This deletes the file after comparing it for smart disk space usage.
		 */
		char cmd[100] = { 0 };
        system(cmd);
        switch(size) {
            case 1048576:
                sprintf(cmd, "echo %i `cmp -l ./tests/A ./tmp/%i | wc -l` >> cmp.log", id, id);
                break;
            case 2097152:
                sprintf(cmd, "echo %i `cmp -l ./tests/B ./tmp/%i | wc -l` >> cmp.log", id, id);
                break;
            case 5242880:
                sprintf(cmd, "echo %i `cmp -l ./tests/C ./tmp/%i | wc -l` >> cmp.log", id, id);
                break;
            case 10485760:
                sprintf(cmd, "echo %i `cmp -l ./tests/D ./tmp/%i | wc -l` >> cmp.log", id, id);
                break;
            case 20971520:
                sprintf(cmd, "echo %i `cmp -l ./tests/E ./tmp/%i | wc -l` >> cmp.log", id, id);
                break;
            default:
                printf("filesize unknown: %i.", size);
				sprintf(cmd, "echo %i -1 >> cmp.log", id);
        }
        printf("%s\n", cmd);
        system(cmd);
        sprintf(cmd, "rm tmp/%i", id);
        printf("%s\n", cmd);
        system(cmd);

		/* 
		 * Clean connection resources 
		 */ 
		free(buf);
        close(clientFD);		
	}

	close(sock);
	printf("Initial ID is invalid or an overflow occured.\n");
	return -1;
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


