/* bt_forwarder.c
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

/*****************************************************************************/
/*** bt_forwarder.c		                                                   ***/
/***                                                                       ***/
/*****************************************************************************/

/**************************************************************************
*	This is a simple forwarder for bluetooth clients and servers
**************************************************************************/

#include "common.h"
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define MAXBUF	1008

int main(int Count, char *Strings[])
{   
	int sockfd, serverFD, status, n;
	char buffer[MAXBUF];

	if (Count != 2) {
		printf("Usage %s <server address>\n", Strings[0]);
	}


	/*---Create streaming socket---*/
    if ( (sockfd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0 )
	{
		perror("Socket");
		exit(errno);
	}

	/*---Initialize address/port structure---*/
	struct sockaddr_rc self;
	bzero(&self, sizeof(self));
	self.rc_family = AF_BLUETOOTH;
	self.rc_channel = (uint8_t) 1;
	self.rc_bdaddr = *BDADDR_ANY;

	/*---Assign a port number to the socket---*/
    if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
	{
		perror("socket--bind");
		exit(errno);
	}



	/*---Make it a "listening socket"---*/
	if ( listen(sockfd, 20) != 0 )
	{
		perror("socket--listen");
		exit(errno);
	}

	/*---Forever... ---*/
	while (1)
	{	int clientfd;
		struct sockaddr_rc client_addr;
		
		int addrlen=sizeof(client_addr);

		/*---accept a connection (creating a data pipe)---*/
		clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
		printf("connected to a client\n");
		serverFD = connect_to_server(Strings[1]);

		while((n = recv(clientfd, buffer, MAXBUF, 0)) != 0) {

			send(serverFD, buffer, n, 0);
			if (status < 0) perror("Sending Error:");
		}

		/*---Close data connection---*/
		close(clientfd);
		close(serverFD);
	}

	/*---Clean up (should never get here!)---*/
	close(sockfd);
	return 0;
}

int connect_to_server(char *address) {
	int serverFD, status;
	
	serverFD = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	struct sockaddr_rc addr = { 0 };
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba(address, &addr.rc_bdaddr);

	status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
	while (status < 0) {
		perror("Connection failure, retrying in 1 sec..");
		sleep(1);
		serverFD = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
		status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
	}
	return serverFD;
}