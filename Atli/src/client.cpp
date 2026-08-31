/*
--------------------------------------------------------------------------------
Response from port 4013:
Send me a 4-byte message containing the signature you got from S.E.C.R.E.T in
the first 4 bytes (in network byte order).
--------------------------------------------------------------------------------
Response from port 4044:
Greetings! I am E.X.P.S.T.N, which stands for "Enhanced X-link Port Storage
Transaction Node".

What can I do for you?
- If you provide me with a list of secret ports (comma-separated), I can guide
you on the exact sequence of "knocks" to ensure you score full marks.

How to use E.X.P.S.T.N?
1. Each "knock" must be paired with both a secret phrase and your unique
   S.E.C.R.E.T signature.
2. The correct format to send a knock: First, 4 bytes containing your
   S.E.C.R.E.T signature, followed by the secret phrase.

Tip: To discover the secret ports and their associated phrases, start by
solving challenges on the ports detected using your port scanner. Happy hunting!
--------------------------------------------------------------------------------
Response from port 4052:
The dark side of network programming is a pathway to many abilities some
consider to be...unnatural. I am an evil port, I will only communicate with
evil processes! (https://en.wikipedia.org/wiki/Evil_bit)
Send us a message of 4 bytes containing the signature that you created with
S.E.C.R.E.T
--------------------------------------------------------------------------------
Response from port 4081:
Greetings from S.E.C.R.E.T. (Secure Encryption Certification Relay with
Enhanced Trust)! Here's how to access the secret port I'm safeguarding:
1. Generate a 32 bit secret number (and remember it for later)
2. Send me a message where the first byte is the letter 'S' followed by 4 bytes
   containing your secret number (in network byte order), and the rest of the
   message is a comma-separated list of the RU usernames of all your group
   members.
3. I will reply with a 5-byte message, where the first byte is your group ID
   and the remaining 4 bytes are a 32 bit challenge number (in network byte
   order)
4. Combine this challenge using the XOR operation with the secret number you
   generated in step 1 to obtain a 4 byte signature.
5. Reply with a 5-byte message: the first byte is your group number, followed
   by the 4-byte signature (in network byte order).
6. If your signature is correct, I will respond with a secret port number. Good
   luck!
7. Remember to keep your group ID and signature for later, you will need them
   for other ports. (But do not hard-code them!)
--------------------------------------------------------------------------------
*/

#include <asm-generic/socket.h>
#include <cerrno>
#include <iostream>
#include <netinet/in.h>
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

        std::cout << "Attempting to receive group number and challenge..." << std::endl;

        char buffer[2048];
        sockaddr_in receive_address;
        socklen_t receive_address_size = sizeof(receive_address);

        ssize_t res = recvfrom(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&receive_address, &receive_address_size);
        if(res < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                // No response
            } else {
                perror("Error receiving");
                return 1;
            }
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

        std::cout << "Attempting to receive secret port response..." << std::endl;

        char buffer[2048];
        sockaddr_in receive_address;
        socklen_t receive_address_size = sizeof(receive_address);
        ssize_t res = recvfrom(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&receive_address, &receive_address_size);

        if(res < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                // No response
            } else {
                perror("Error receiving");
                return 1;
            }
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

    if(!gotResponse) {
        std::cout << "Didn't get secret port response, continuing to 4013..." << std::endl;
    }

    // First message to 4013:
    port = 4013;
    dest_address.sin_port = htons(port);

    std::vector<unsigned char> message4013;
    message4013.resize(4);

    memcpy(message4013.data(), &signature_n, sizeof(signature_n));

    gotResponse = false;
    for(int i = 0; i < 5; ++i) {
        if(gotResponse) {
            break;
        }

        std::cout << "Sending signature to port 4013..." << std::endl;

        if(sendto(sockfd, message4013.data(), message4013.size(), 0, (struct sockaddr*)&dest_address, sizeof(dest_address)) < 0) {
            perror("Error sending");
            return 1;
        }

        std::cout << "Attempting to receive challenge from 4013..." << std::endl;

        char buffer[2048];
        sockaddr_in receive_address;
        socklen_t receive_address_size = sizeof(receive_address);
        ssize_t res = recvfrom(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&receive_address, &receive_address_size);
        if(res < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                // no response
            } else {
                perror("Error receiving");
                return 0;
            }
        }

        int received_port = ntohs(receive_address.sin_port);

        if(received_port == port) {
            std::cout << "Response (" << port << "), " << res << " bytes:" << std::endl;
            std::string response(buffer, res-6);
            std::cout << response << std::endl;

            short challenge_checksum = 0;
            memcpy(&challenge_checksum, buffer + res - 6, sizeof(challenge_checksum));
            challenge_checksum = ntohs(challenge_checksum);

            //std::cout << "Checksum: " << std::hex << challenge_checksum << std::dec << std::endl;

            int challenge_IP = 0;
            memcpy(&challenge_IP, buffer + res - 4, sizeof(challenge_IP));
            /*
            char IPstr[INET_ADDRSTRLEN];
            if(inet_ntop(AF_INET, &challenge_IP, IPstr, sizeof(IPstr)) == 0) {
                perror("Error parsing IP");
                return 1;
            }
            std::cout << "IP: " << IPstr << std::endl;
            */
            gotResponse = true;
            /*
            Hello group 6! To get the secret phrase, reply to this message with a UDP
            message where the payload is an encapsulated, valid UDP IPv4 packet, that
            has a valid UDP checksum of 0x420a, and with the source address being
            116.217.223.100! (Hint: all you need is a normal UDP socket which you use
            to send the IPv4 and UDP headers possibly with a payload) (the last 6
            bytes of this message contain the checksum and ip address in network
            byte order for your convenience)
            */
        }
    }

    // TODO - construct packet for challenge response

    return 0;
}
