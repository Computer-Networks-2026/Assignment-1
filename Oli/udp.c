#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
// This program demonstrates a simple UDP client that sends a message to a server and waits for a reply. It sets a timeout for receiving the reply to avoid blocking indefinitely.
int main() {// Start of the main function

	int sock = socket(AF_INET, SOCK_DGRAM, 0); // Create a UDP socket
	struct timeval tv = { .tv_sec = 4, .tv_usec = 0 }; // Set a timeout of 4 seconds for receiving data
	setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));// Set the socket option to apply the timeout

	struct sockaddr_in dest;// Define the destination address structure
	memset(&dest, 0, sizeof(dest));// Initialize the destination address structure to zero
	dest.sin_family = AF_INET;// Set the address family to IPv4
	dest.sin_port = htons(9000);// Set the destination port to 9000 (in network byte order)
	inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);// Convert the IP address from text to binary form and store it in the destination address structure

	char *msg = "I am here\n";// Define the message to be sent
	printf("Sending message to port 9000 \n");// Print a message indicating that the message is being sent to port 9000
	sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&dest, sizeof(dest));// Send the message to the destination address

	char buffer[100];// Define a buffer to store the received reply
	int bytes = recvfrom(sock, buffer, sizeof(buffer) -1, 0, NULL, NULL);// Receive a reply from the destination address and store it in the buffer

	if (bytes > 0) {// Check if any bytes were received
		buffer[bytes] = '\0';// Null-terminate the received data to make it a valid string
		printf("recieved reply: %s\n", buffer);// Print the received reply
	} else { // If no bytes were received, it means the receive operation timed out
		printf("No reply gotten, timed out. \n");// Print a message indicating that no reply was received and the operation timed out
	}

	close(sock);// Close the socket to free up resources
	return 0;// Return 0 to indicate successful execution of the program
}

