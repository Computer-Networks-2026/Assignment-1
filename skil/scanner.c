#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>

#define MAX_RETRIES 5  //max retries for sending probes until give up 
#define LISTEN_TIMEOUT_MS 500 // 500ms listen window per burst

int main(int argc, char *argv[]) {

    //error checking for command line arguments
    if (argc != 4) {
        fprintf(stderr, "usage: %s <IP Address> <Low Port> <High Port>\n", argv[0]);
        return 1;
    }

    //parse command line arguments
    const char *ipaddr = argv[1];
    int low_port = atoi(argv[2]);
    int high_port = atoi(argv[3]);

    //validate port range
    if (low_port < 1 || high_port < 1 || low_port > 65535 || high_port > 65535) {
        fprintf(stderr, "Ports must be between 1 and 65535.\n");
        return 1;
    }

    //validate port order
    if (high_port < low_port) {
        fprintf(stderr, "Low port must be less than or equal to high port.\n");
        return 1;
    }

    // Initialize destination address once
    struct sockaddr_in destaddr;
    memset(&destaddr, 0, sizeof(destaddr));
    destaddr.sin_family = AF_INET;

    // Convert IP address from string to binary form
    int res = inet_pton(AF_INET, ipaddr, &destaddr.sin_addr);
    if (res < 0) {
        perror("Error converting IP");
        return 1;
    } else if (res == 0) {
        fprintf(stderr, "Error: Invalid IP address '%s'\n", ipaddr);
        return 1;
    }

    // Calculate the number of ports to scan
    int port_count = high_port - low_port + 1;
    bool *open_ports = calloc(port_count, sizeof(bool));
    if (!open_ports) {
        perror("calloc");
        return 1;
    }

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("error creating socket");
        free(open_ports);
        return 1;
    }

    // Set socket receive timeout for incoming datagrams
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000; // 100ms
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("scanning %s across ports %d-%d...\n", ipaddr, low_port, high_port);

    const char *msg = "$PING$";

    // Main scanning loop with retries
    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        // Send probes to unresolved ports
        for (int port = low_port; port <= high_port; port++) {
            int idx = port - low_port;
            if (open_ports[idx]) continue;

            destaddr.sin_port = htons(port);
            sendto(sockfd, msg, strlen(msg), 0,
                   (struct sockaddr *)&destaddr, sizeof(destaddr));
        }

        // Listen for responses across the entire socket
        struct timeval start, now;
        gettimeofday(&start, NULL);
        // Listen for responses until timeout
        while (1) {
            gettimeofday(&now, NULL);
            long elapsed_ms = (now.tv_sec - start.tv_sec) * 1000 +
                              (now.tv_usec - start.tv_usec) / 1000;
            if (elapsed_ms >= LISTEN_TIMEOUT_MS) break;

            struct sockaddr_in srcaddr;
            socklen_t srcaddrlen = sizeof(srcaddr);
            char buffer[2048];

            ssize_t bytes = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                     (struct sockaddr *)&srcaddr, &srcaddrlen);
            // Check if we received a response
            if (bytes > 0) {
                int resp_port = ntohs(srcaddr.sin_port);
                if (resp_port >= low_port && resp_port <= high_port) {
                    int idx = resp_port - low_port;
                    if (!open_ports[idx]) {
                        open_ports[idx] = true;
                        printf("[+] Port %d is OPEN\n", resp_port);
                    }
                }// Handle timeout or no data received
            } else if (bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                continue;
            }
        }
    }
    // After all attempts, print the results
    printf("Scan Complete ---\n");

    free(open_ports);
    close(sockfd);
    return 0;
}