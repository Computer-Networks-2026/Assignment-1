# INSTRUCTIONS

be in console 
run the commands in order
make 
./udp 130.208.246.98 4000 4100


# Results

> 1.a
```
ping 130.208.246.98 works and responds with 64 byte packages taking 2-6seconds each

open ports are 

Nmap scan report for 130.208.246.98
Host is up (0.0053s latency).
Not shown: 884 filtered tcp ports (no-response), 115 closed tcp ports (reset)
PORT   STATE SERVICE
22/tcp open  ssh

```

>1.b
```
screenshot called tcpdump
```
>1.c
```
sudo tcpdump −vAX −i ens192 ’port 5000’ captures or monitors network traffic or packets passing through a network interface of our choice. v is verbose, A and X are both in what kind of print format we want it, -i interface is what network interface it is, port is what port we are using 
ncat −ln 5000 starts a listening server that waits for a "connection", -l is listen, -n is is using a raw IP address to make it faster. 5000 is the port 
ncat 130.208.246.98 5000  this is the client, the ip address is the destination, and the port is the port. the command estabilishes an outbound connection to server on port 5000. 
```

>1.d
```
screenshot called udp
```

>1.e
```
the udp is a lot cleaner to read, no need for the connection management overhead of handshakes. Just clean packet style where you can see the messages even. 
```

>2
```
code in udp

oli0202@Oli-talva:/mnt/c/Users/oli_0/desktop/tsam/Assignment-1/Oli$ ./udp 130.208.246.98 4000 4100
Scanning 130.208.246.98 across ports 4000-4100...
[+] Port 4013 is OPEN
[+] Port 4044 is OPEN
[+] Port 4081 is OPEN
[+] Port 4052 is OPEN

--- Scan Complete ---


Running nmap via  sudo nmap -sU -p 4000-4100 -T4 --max-retries 3 130.208.246.98

oli0202@Oli-talva:/mnt/c/Users/oli_0/desktop/tsam/Assignment-1/Oli$ sudo nmap -sU -p 4000-4100 -T4 --max-retries 3 130.2
08.246.98
Starting Nmap 7.94SVN ( https://nmap.org ) at 2026-08-31 14:59 GMT
Warning: 130.208.246.98 giving up on port because retransmission cap hit (3).
Nmap scan report for 130.208.246.98
Host is up (0.0061s latency).
Not shown: 82 open|filtered udp ports (no-response)
PORT     STATE  SERVICE
4012/udp closed pda-gate
4013/udp open   acl-manager
4016/udp closed talarian-mcast2
4020/udp closed trap
4025/udp closed partimage
4028/udp closed dtserver-port
4030/udp closed jdmn-port
4035/udp closed wap-push-http
4038/udp closed fazzt-ptp
4044/udp open   ltp
4052/udp open   interact
4067/udp closed idp
4071/udp closed aibkup
4073/udp closed iRAPP
4075/udp closed perimlan
4077/udp closed ascomalarm
4081/udp open   lorica-in-sec
4083/udp closed lorica-out-sec
4097/udp closed patrolview

Nmap done: 1 IP address (1 host up) scanned in 8.02 seconds

basically the same it seems 
```
