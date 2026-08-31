#include <iostream>
#include <string>

#include <cstdlib>
#include <cerrno>
#include <cstdio>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char ** argv) {
    if(argc < 4) {
        std::cout << "Usage:" << std::endl;
        std::cout << "./main IP LOWPORT HIGHPORT" << std::endl;
        return 0;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }

    struct timeval to;
    to.tv_sec = 1;
    to.tv_usec = 0;

    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to)) != 0) {
        perror("Error setting socket timeout");
        return 1;
    }

    struct sockaddr_in dest_address{};
    dest_address.sin_family = AF_INET;

    int res = inet_pton(AF_INET, argv[1], &dest_address.sin_addr);
    if( res < 0) {
        perror("Error converting IP");
        return 1;
    } else if(res == 0) {
        std::cout << "Invalid IP" << std::endl;
        return 0;
    }

    int start_port = atoi(argv[2]);
    int end_port = atoi(argv[3]);

    if( start_port < 1 || end_port < 1 || start_port > 65535 || end_port > 65535) {
        std::cout << "Port out of range" << std::endl;
        return 0;
    }

    if( end_port < start_port) {
        std::cout << "Enter the start port first" << std::endl;
        return 0;
    }

    int total_ports = (end_port - start_port) + 1;
    bool* responses = new bool[total_ports];

    std::string message = "Hello\n";

    for( int p = start_port; p <= end_port; ++p) {
        dest_address.sin_port = htons(p);
        responses[p - start_port] = false;
        for( int i = 0; i < 5; ++i) {
            if(sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
                perror("Error sending");
                return 1;
            }
        }
    }

    while(true) {
        char buffer[2048];
        sockaddr_in receive_address{};
        socklen_t receive_address_size = sizeof(receive_address);

        ssize_t res = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&receive_address, &receive_address_size);

        if(res < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("Error receiving");
            return 1;
        }

        int received_port = ntohs(receive_address.sin_port);
        if(received_port >= start_port && received_port <= end_port) {
            responses[received_port - start_port] = true;
        }
    }

    for( int i = 0; i < total_ports; ++i) {
        if(responses[i]) {
            std::cout << "Port " << (start_port + i) << " is open." << std::endl;
        }
    }

    delete[] responses;

    close(sockfd);

    return 0;
}
