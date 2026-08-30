#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cout << "Usage: ./scanner <IP> <low port> <high port>\n";
        return 1;
    }

    const char* ip = argv[1];
    int lowPort = std::stoi(argv[2]);
    int highPort = std::stoi(argv[3]);
    // create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    // set retrieve timeour
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // set up server address
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server.sin_addr);

    char message[] = "hello";
    char buffer[100];

    // scan each port in the range
    for (int port = lowPort; port <= highPort; port++)
    {
        server.sin_port = htons(port);

        bool open = false;
        // try each port a few times because UDP packets may be lost
        for (int attempt = 0; attempt < 3; attempt++)
        {
            // send the UDP packet
            sendto(sock,
                   message,
                   strlen(message),
                   0,
                   (struct sockaddr*)&server,
                   sizeof(server));

            struct sockaddr_in sender;
            socklen_t senderLength = sizeof(sender);

            // wait for a reply
            int received = recvfrom(sock,
                                    buffer,
                                    sizeof(buffer),
                                    0,
                                    (struct sockaddr*)&sender,
                                    &senderLength);

            if (received > 0)
            {
                open = true;
                break;
            }
        }
        // log the port as open if we get a response
        if (open)
        {
            std::cout << "Port " << port << " is open\n";
        }
    }
    // close the socket when finished
    close(sock);
    return 0;
}