/* simple-server.c
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
/*** simple-server.c                                                       ***/
/***                                                                       ***/
/*****************************************************************************/

/**************************************************************************
*	This is a simple echo server.  This demonstrates the steps to set up
*	a streaming server.
**************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>

#define MY_PORT		8888
#define MAXBUF		1024

int main(int Count, char *Strings[])
{   int sockfd, serverFD, status, n;
	struct sockaddr_in self, addr;
	char buffer[MAXBUF];

	if (Count != 2) {
		printf("Usage %s <server address>\n", Strings[0]);
	}
	/*---Create streaming socket---*/
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Socket");
		exit(errno);
	}

	/*---Initialize address/port structure---*/
	bzero(&self, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(MY_PORT);
	self.sin_addr.s_addr = INADDR_ANY;

	/*---Assign a port number to the socket---*/
    if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
	{
		perror("socket--bind");
		exit(errno);
	}

	serverFD = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(MY_PORT);
	inet_pton(AF_INET, Strings[1], &addr.sin_addr);

	status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
	while (status < 0){
		perror("Connection failure, retrying in 1 sec..");
		sleep(1);
		serverFD = socket(AF_INET, SOCK_STREAM, 0);
		status = connect(serverFD, (struct sockaddr *)&addr, sizeof(addr));
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
		struct sockaddr_in client_addr;
		
		int addrlen=sizeof(client_addr);

		/*---accept a connection (creating a data pipe)---*/
		clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
		printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		while((n = recv(clientfd, buffer, MAXBUF, 0)) != 0) {

			send(serverFD, buffer, n, 0);
			if (status < 0) perror("Sending Error:");
		}

		/*---Close data connection---*/
		close(clientfd);
	}

	/*---Clean up (should never get here!)---*/
	close(sockfd);
	return 0;
}