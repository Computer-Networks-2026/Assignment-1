#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>

#define MAX_RETRIES 5   //max retries 
#define LISTEN_TIMEOUT_MS 500 // 500ms listen window per burst

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s <IP Address> <Low Port> <High Port>\n", argv[0]);
        return 1;
    }

    const char *ipaddr = argv[1];
    int low_port = atoi(argv[2]);
    int high_port = atoi(argv[3]);

    if (low_port <= 0 || high_port <= 0 || low_port > high_port) {
        fprintf(stderr, "error: Invalid port range.\n");
        return 1;
    }

    int port_count = high_port - low_port + 1;
    bool *open_ports = calloc(port_count, sizeof(bool));
    if (!open_ports) {
        perror("calloc");
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("error creating socket");
        free(open_ports);
        return 1;
    }

    // sets a short socket receive timeout for inc replies
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000; // 100ms per
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("scanning %s across ports %d-%d...\n", ipaddr, low_port, high_port);

    const char *msg = "PING";

    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        for (int port = low_port; port <= high_port; port++) {
            int idx = port - low_port;
            if (open_ports[idx]) continue; // skip  discovered ports

            struct sockaddr_in destaddr;
            memset(&destaddr, 0, sizeof(destaddr));
            destaddr.sin_family = AF_INET;
            destaddr.sin_port = htons(port);
            inet_pton(AF_INET, ipaddr, &destaddr.sin_addr);

            sendto(sockfd, msg, strlen(msg), 0,
                   (struct sockaddr *)&destaddr, sizeof(destaddr));
        }

        //listen for responses across the entire socket
        // check for incoming packets til listening window expires
        struct timeval start, now;
        gettimeofday(&start, NULL);

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

            if (bytes > 0) {
                int resp_port = ntohs(srcaddr.sin_port);
                if (resp_port >= low_port && resp_port <= high_port) {
                    int idx = resp_port - low_port;
                    if (!open_ports[idx]) {
                        open_ports[idx] = true;
                        printf("[+] Port %d is OPEN\n", resp_port);
                    }
                }
            } else if (bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                continue;
            }
        }
    }

    printf("scan complete\n");

    free(open_ports);
    close(sockfd);
    return 0;
}