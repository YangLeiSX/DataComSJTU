#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

// number of threads
int Count;
// thread function
void do_service(void* client_s);

int __cdecl main(void)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
    SOCKET client_s;
    struct sockaddr_in  client_addr;
    struct in_addr client_ip_addr;
    int addr_len;
    char ipstringbuffer[46];

    struct sockaddr_in server_addr;
    struct addrinfo* result = NULL;
    struct addrinfo hints;

    // int iSendResult;
    // char recvbuf[DEFAULT_BUFLEN];
    // int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    char hostname[NI_MAXHOST];
    gethostname(hostname, NI_MAXHOST);
    printf("hostname: %s\n", hostname);

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(hostname, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    memcpy(&server_addr, result->ai_addr, sizeof(server_addr));
    inet_ntop(AF_INET, &server_addr.sin_addr, ipstringbuffer, sizeof(ipstringbuffer));
    printf("Server Address: %s\n", ipstringbuffer);
    printf("Server Port Number: %d\n", ntohs(server_addr.sin_port));
    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    printf("Socket Done.\n");

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("Bind Done.\n");

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("Listen Done.\n");

    Count = 0;
    while (1) {
        Count++;
        printf("Count=%d\n", Count);
        addr_len = sizeof(client_addr);
        client_s = accept(ListenSocket, (struct sockaddr*) & client_addr, &addr_len);
        memcpy(&client_ip_addr, &client_addr.sin_addr.s_addr, 4);

        // display information
        printf("Connection %d accepted!!\n", Count);
        inet_ntop(AF_INET, &client_ip_addr, ipstringbuffer, sizeof(ipstringbuffer));
        printf("\tClient socket number: %d\n", client_s);
        printf("\tIPv4 address: %s\n", ipstringbuffer);
        printf("\tPort number: %d\n", ntohs(client_addr.sin_port));

        if (_beginthread(do_service, 4096, (void*)client_s) < 0) {
            printf("ERROR - Unable to create thread \n");
            exit(1);
        }
    }
    
    closesocket(ListenSocket);
    // cleanup
    WSACleanup();

    return 0;
}

void do_service(void* client_s) {
    char in_buf[1024];
    printf("thread begining...\n");
    while (1) {
        if (recv((SOCKET)client_s, in_buf, sizeof(in_buf), 0) == 0)
            break;
        printf("Receiving from client %d ... data is \"%s\"\n", (SOCKET)client_s,in_buf);
        send((SOCKET)client_s, in_buf, (strlen(in_buf) + 1), 0);
    }
    printf("thread completed...\n");
    Count--;
    closesocket((SOCKET)client_s);
    _endthread();
}