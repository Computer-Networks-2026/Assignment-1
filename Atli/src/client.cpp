#include <asm-generic/socket.h>
#include <cerrno>
#include <iostream>
#include <vector>
#include <cstring>

#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char ** argv) {
    if(argc < 2) {
        std::cout << "Usage: ./client IP" << std::endl;
        return 0;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if(sockfd == -1) {
        perror("Error creating socket");
    }

    struct timeval to;
    to.tv_sec = 1;
    to.tv_usec = 0;

    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*) &to, sizeof(struct timeval)) != 0) {
        perror("Error setting socket timeout.");
        return 1;
    }

    struct sockaddr_in dest_address;
    dest_address.sin_family = AF_INET;

    int res = inet_pton(AF_INET, argv[1], &dest_address.sin_addr);
    if(res < 0) {
        perror("Error converting IP");
        return 1;
    } else if(res == 0) {
        std::cout << "Invalid IP: \"" << argv[1] << "\"" << std::endl;
        return 1;
    }

    // First message to 4081:
    int port = 4081;
    int secret = 0xDEADBEEF;
    int secret_n = htonl(secret);
    unsigned char group_number = 0;
    int challenge = 0;
    int signature = 0;
    int signature_n = 0;

    std::vector<unsigned char> message;
    std::string group_members = "atlif23,evah23,olib22";

    int message_length = 1 + 4 + group_members.length();
    message.resize(message_length);

    message[0] = 'S';
    memcpy(message.data() + 1, &secret_n, sizeof(secret_n));
    memcpy(message.data() + 5, group_members.data(), group_members.length());

    dest_address.sin_port = htons(port);
    bool gotResponse = false;
    for(int i = 0; i < 5; ++i) {
        if(gotResponse) {
            break;
        }

        std::cout << "Sending secret number and group members to 4081..." << std::endl;

        if(sendto(sockfd, message.data(), message.size(), 0, (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
            perror("Error sending");
            return 1;
        }

        if(!gotResponse) {
            std::cout << "Attempting to receive group number and challenge..." << std::endl;

            char buffer[2048];
            sockaddr_in receive_address;
            socklen_t receive_address_size = sizeof(receive_address);

            ssize_t res = recvfrom(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&receive_address, &receive_address_size);
            if(res < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    // No response
                }
                perror("Error receiving");
                return 1;
            }

            int received_port = ntohs(receive_address.sin_port);

            if(received_port == port) {
                std::cout << "Response (" << port << "), " << res << " bytes:" << std::endl;

                memcpy(&group_number, buffer, sizeof(group_number));
                memcpy(&challenge, buffer + 1, sizeof(challenge));

                challenge = ntohl(challenge);
                signature = challenge ^ secret;
                signature_n = htonl(signature);

                std::cout << "Group: " << (int)group_number << std::endl;
                std::cout << "Challenge: " << std::hex << challenge << " (" << std::dec << challenge << ")" << std::endl;
                std::cout << "Signature: " << std::hex << signature << " (" << std::dec << signature << ")" << std::endl;
                gotResponse = true;
                /*
                Group: 6
                Challenge: bacd906b (-1160933269)
                Signature: 64602e84 (1684024964)
                */
            }
        }
    }

    if(!gotResponse) {
        std::cout << "Didn't get a response so no point continuing..." << std::endl;
        return 0;
    }

    // Second message to 4081:

    std::vector<unsigned char> message2;
    message2.resize(5);

    message2[0] = group_number;
    memcpy(message2.data() + 1, &signature_n, sizeof(signature_n));

    gotResponse = false;
    for(int i = 0; i < 5; ++i) {
        if(gotResponse) {
            break;
        }

        std::cout << "Sending group number and signature to 4081..." << std::endl;

        if(sendto(sockfd, message2.data(), message2.size(), 0, (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
            perror("Error sending");
            return 1;
        }

        if(!gotResponse) {
            std::cout << "Attempting to receive secret port response..." << std::endl;
            char buffer[2048];
            sockaddr_in receive_address;
            socklen_t receive_address_size = sizeof(receive_address);
            ssize_t res = recvfrom(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&receive_address, &receive_address_size);
            if(res < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    // No response
                }
                perror("Error receiving");
                return 1;
            }
            int received_port = ntohs(receive_address.sin_port);
            if(received_port == port) {
                std::cout << "Response (" << port << "), " << res << " bytes:" << std::endl;
                std::string response(buffer, res);
                std::cout << response << std::endl;
                gotResponse = true;
                /*
                Response (4081), 68 bytes:
                Well done group 6. You have earned the right to know the port: 4075!
                */
            }
        }
    }

    if(!gotResponse) {
        std::cout << "Didn't get secret port response, continuing to 4013..." << std::endl;
    }

    // First message to 4013:
    port = 4013;

    std::vector<unsigned char> message4013;
    message4013.resize(4);

    memcpy(message4013.data(), &signature_n, sizeof(signature_n));

    dest_address.sin_port = htons(port);

    for(int i = 0; i < 5; ++i) {
        if(sendto(sockfd, message4013.data(), message4013.size(), 0, (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
            perror("Error sending");
        }
    }

    while(true) {
        char buffer[2048];
        sockaddr_in receive_address;
        socklen_t receive_address_size = sizeof(receive_address);
        ssize_t res = recvfrom(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&receive_address, &receive_address_size);
        if(res < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("Error receiving");
            break;
        }
        int received_port = ntohs(receive_address.sin_port);
        if(received_port == port) {
            std::cout << "Response (" << port << "), " << res << " bytes:" << std::endl;
            std::string response(buffer, res);
            std::cout << response << std::endl;
            /*
            */
        }
    }


    return 0;
}
