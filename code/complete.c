// accept(), bind(), connect(), listen(), send(), recv(), setsockopt(), socket()
#include <sys/socket.h>
// C basics (printf(), fprintf())
#include <stdio.h>
// strcmp()
#include <string.h>
// getaddrinfo()
#include <netdb.h>
// fork()
#include <unistd.h>
// exit()
#include <stdlib.h>
// waitpid()
#include <sys/wait.h>
// errno
#include <errno.h>

// defining the constants for easier use
// our assigned port
#define SERVER_PORT "22013"
// HTTP port
#define REQUEST_PORT "80"
// one of the few sites that accepted HTTP requets
#define REQUEST_ADDRESS "axu.tm"
// the command corresponding to the site
#define OUR_COMMAND "06#"
// the length of the queue in case connections cannot be accepted
#define BACKLOG 10

int main() {

// struct for the foreign address
struct sockaddr_storage their_addr;
// length of this address
socklen_t addr_size;

int info, sockfd, b, l, new_fd;
// hints and a pointer to the results
struct addrinfo hints, *res;

// making sure the hints struct is empty at first
memset(&hints, 0, sizeof hints);
// IPv4 family
hints.ai_family = AF_INET;
// TCP stream sockets
hints.ai_socktype = SOCK_STREAM;
// together with the NULL node it allows us to access any of the host's network addresses
hints.ai_flags = AI_PASSIVE;

// getting the status of getaddrinfo()
info = getaddrinfo(NULL, SERVER_PORT, &hints, &res);

// handling the errors by printing the string according to the error number
if (info != 0) {
	printf("getaddrinfo() failed: %s\n", gai_strerror(info));
	exit(1);
}
// in case of success continue
else
	printf("Address successfully obtained\n");

// opening an IPv4 TCP socket
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// handling the errors by printing the string according to the error number
if (sockfd == -1) {
	printf("socket() failed: %s\n", strerror(errno));
	exit(1);
}
// in case of success continue
else
	printf("Socket successfully created\n");

const int enable = 1;

// enabling the reuse of the address and port combination in case
// no socket is already listening to this combination
// usually this is done to bypass the TIME-WAIT state
// which lasts 60 seconds by default
if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
	printf("setsockopt(SO_REUSEADDR) failed: %s\n", strerror(errno));
	exit(1);
}

if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
	printf("setsockopt(SO_REUSEPORT) failed: %s\n", strerror(errno));
	exit(1);
}

// binding the socket to our IP address
b = bind(sockfd, res->ai_addr, res->ai_addrlen);

// handling the errors by printing the string according to the error number
if (b == -1) {
	printf("bind() failed: %s\n", strerror(errno));
	exit(1);
}
// in case of success continue
else
	printf("Bind successful\n");

// listening for incoming connections
l = listen(sockfd, BACKLOG);

// handling the errors by printing the string according to the error number
if (l == -1) {
	printf("listen() failed: %s\n", strerror(errno));
	exit(1);
}
// in case of success continue
else
	printf("Listening\n");

// character array for receiving the command
char command[4];

while(1) {
	
	// setting the addr_size variable to the size of the foreign address
	addr_size = sizeof their_addr;
	// extracting the first connection request on the queue of pending connections
	// for the listening, creating the new_fd socket in the process
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
	
	// handling the errors by printing the string according to the error number
	// in case of an error, it moves to the next iteration of the while loop
	// to try again
	if (new_fd == -1) {

		printf("accept() failed: %s\n", strerror(errno));
		continue;
	}

	// child process id
	int pid;

	// using a fork() for each child (each connection requested by the client)
	// the return value of the fork() function is 0 for each child
    	if ((pid = fork()) == 0) {

      		int bytes_received;
		
		// checking if the command bytes were successfully received
		while ((bytes_received = recv(new_fd, command, 3, 0)) > 0) {

		int bytes_sent;
		
		// if the command that was received corresponds to our command
		if (strcmp(command, OUR_COMMAND) == 0) {

			int info2, sock, conn;

			// hints and a pointer to the results
			struct addrinfo hints2, *res2;

			// making sure the hints struct is empty at first
			memset(&hints2, 0, sizeof hints2);
			// IPv6 family
			hints2.ai_family = AF_INET6;
			// TCP stream sockets
			hints2.ai_socktype = SOCK_STREAM;

			// getting the status of getaddrinfo()
			// for the address where we make the HTTP request
			info2 = getaddrinfo(REQUEST_ADDRESS, REQUEST_PORT, &hints2, &res2);

			// handling the errors by printing the string according to the error number
			if (info2 != 0) {
				printf("getaddrinfo() failed: %s\n", gai_strerror(info2));
				exit(1);
			}
			// in case of success continue
			else
				printf("Address successfully obtained\n");

			// opening an IPv6 TCP socket
			sock = socket(res2->ai_family, res2->ai_socktype, res2->ai_protocol);

			// handling the errors by printing the string according to the error number
			if (sock == -1) {
				printf("socket() failed: %s\n", strerror(errno));
				exit(1);
			}
			// in case of success continue
			else
				printf("Socket successfully created\n");

			// connecting to the destionation IP address
			conn = connect(sock, res2->ai_addr, res2->ai_addrlen);

			// handling the errors by printing the string according to the error number
			if (conn == -1) {
				printf("connect() failed: %s\n", strerror(errno));
				exit(1);
			}
			// in case of success continue
			else
				printf("Connection successfully created\n");

			// setting up the HTTP GET request
			char *request = "GET / HTTP/1.0\r\n\r\n";
			int leng = strlen(request);
			int bytes_sent2, bytes_received2;

			// sending the request and handling the errors in case of failure
			if ((bytes_sent2 = send(sock, request, leng, 0)) == -1) {
				printf("send() failed: %s\n", strerror(errno));
				exit(1);
			}

			// checking that all the bytes were sent
			printf("Bytes sent: %d\n", bytes_sent2);

			// creating an HTML file
			char *filename = "/home/g0s1e3/index.html";

			// opening the file to write to it
			FILE *fp = fopen(filename, "w");

			// checking for errors
			if (fp == NULL) {
				printf("Could not open the file %s.", filename);
				exit(1);
			}

			// character array for receiving the HTML code
			char buffer[2];

			// receiving one byte at a time to ensure the HTML code is complete
			while ((bytes_received2 = recv(sock, buffer, 1, 0)) > 0) {

				// printing the byte in the HTML file
				fprintf(fp, buffer);
				
				// sending the byte to the client side on the socket created earlier
				// and handling the errors in case of failure	
				if ((bytes_sent = send(new_fd, buffer, 1, 0)) == -1) {
					printf("send() failed: %s\n", strerror(errno));
					exit(1);
				}

			}

			// closing the file
			fclose(fp);
			// closing the socket opened for the address
			close(sock);
			// freeing up the memory from the res2 pointer
			freeaddrinfo(res2);

			}

		// letting the client side know that the command is not implemented
		else {
			char *msg = "Command not implemented\n";
			int lng = strlen(msg);

			// sending the bytes to the client side on the socket created earlier
			// and handling the errors in case of failure
			if ((bytes_sent = send(new_fd, msg, lng, 0)) == -1) {
				printf("send() failed: %s\n", strerror(errno));
				exit(1);
			}
		}

	}

	// child process finished successfully
	exit(0);

    }

    // parent process
    else {
	printf("Spawned process %d\n", pid);
	// each child already has everything the parent process had,
	// including the sockets, so this socket can be closed here
      	close(new_fd);
	// check if any child process is ready to be reaped,
	// but if none is then do not wait for it
     	waitpid(-1, NULL, WNOHANG);
    }

}

// closing the remaining sockets
close(sockfd);
close(new_fd);
// freeing up the memory from the res pointer
freeaddrinfo(res);

return 0;

}
