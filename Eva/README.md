UDP Port Scanner

Compilation
-----------
clang++ -std=c++17 -Wall -Wextra scanner.cpp -o scanner

Usage
-----
./scanner <IP address> <low port> <high port>

Example
-------
./scanner 130.208.246.98 4000 4100

Description
-----------
This program scans a range of UDP ports on a given IP address.

For each port, it sends a UDP packet and waits for a response.
Each port is tried multiple times because UDP packets can be lost.

If a response is received, the port is reported as open.