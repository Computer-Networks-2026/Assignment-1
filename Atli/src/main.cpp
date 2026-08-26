#include <iostream>

#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char ** argv) {

    if(argc < 4) {
        std::cout << "Usage:" << std::endl;
        std::cout << "./main IP LOWPORT HIGHPORT" << std::endl;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd == -1) {
        perror("Error connecting");
    }

    struct timeval to;
    to.tv_sec = 1;
    to.tv_usec = 0;

    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*) &to, sizeof(struct timeval)) != 0) {
        perror("Error setting socket timeout");
    }

    struct sockaddr_in dest_address;
    dest_address.sin_family = AF_INET;

    int res = inet_pton(AF_INET, argv[1], &dest_address.sin_addr);
    if( res < 0) {
        perror("Error converting IP");
    } else if(res == 0) {
        std::cout << "Invalid IP" << std::endl;
        return 0;
    }

    int startport = 4000;
    int endport = 4000;

    std::string message = "Hello\n";

    for( int p = startport; p <= endport; ++p) {
        dest_address.sin_port = htons(p);
        for( int i = 0; i < 5; ++i) {
            int res = sendto(sockfd, message.c_str(), message.length(), 0, (struct sockaddr*)&dest_address, sizeof(dest_address));
        }
    }
    return 0;
}
