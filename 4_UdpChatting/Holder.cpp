#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define LISTEN_PORT "19014"
#define TALK_PORT "19015"
void recvThread(void* sock);

int main(int argc, char* argv[]) {
    
    // Initialize variables
    WSADATA wsaData;
    int iResult;

    // Variables to resolve server
    char hostname[NI_MAXHOST];
    struct addrinfo hints;
    struct addrinfo* result = NULL;
    struct sockaddr_in host_addr;
    char ipstringbuffer[46];
    short port;

    // Variables to chatting
    SOCKET udp_s;
    struct sockaddr_in server;
    struct sockaddr_in si_other;
    int addrlen = sizeof(si_other);
    int recvlen;
    char msgbuf[DEFAULT_BUFLEN];

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        exit(EXIT_FAILURE);
    }
    printf("Initialize done.\n");

    // Get server hostname to resolve 
    gethostname(hostname, NI_MAXHOST);
    printf("\thostname: %s\n", hostname);

    // Resolve the address and port
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    iResult = getaddrinfo(hostname, LISTEN_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("getaddrinfo done.\n");

    // Display server information
    memcpy(&host_addr, result->ai_addr, sizeof(host_addr));
    inet_ntop(AF_INET, &host_addr.sin_addr, ipstringbuffer, sizeof(ipstringbuffer));
    port = ntohs(host_addr.sin_port);
    printf("\tServer Address: %s\n", ipstringbuffer);
    printf("\tServer Port Number: %d\n", port);

    // Open Socket
    if ((udp_s = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Create socket %d done.\n", udp_s);
    freeaddrinfo(result);

    // Bind server
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = host_addr.sin_addr.s_addr;
    server.sin_port = host_addr.sin_port;

    if (bind(udp_s, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR) {
        printf("Could not Bind the Socket: %d\n", WSAGetLastError());
        closesocket(udp_s);
        exit(EXIT_FAILURE);
    }
    printf("Bind socket done.\n");

    // Initialize receive
    memset(msgbuf, 0, DEFAULT_BUFLEN);
    if ((recvlen = recvfrom(udp_s, msgbuf, DEFAULT_BUFLEN, 0, (struct sockaddr*) & si_other, &addrlen)) == SOCKET_ERROR) {
        printf("recvfrom() fail with error code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    inet_ntop(AF_INET, &si_other.sin_addr, ipstringbuffer, 46);
    printf("\r                 \t\t\t\t|Receive from %s:%d\n", ipstringbuffer, ntohs(si_other.sin_port));
    printf("\t\t\t\t\t\t|Data: %s\n", msgbuf);
    
    if (_beginthread(recvThread, 4096, (void*)udp_s) < 0)
        printf("thread error.\n");

    while (1) {
        memset(msgbuf, 0, DEFAULT_BUFLEN);
        printf("Enter Sentence:");
        gets_s(msgbuf);
        if (sendto(udp_s, msgbuf, int(strlen(msgbuf) + 1), 0, (struct sockaddr*) & si_other, addrlen) == SOCKET_ERROR) {
            printf("sendto() fail with error code: %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        if (strcmp(msgbuf, "quit") == 0)
            break;
    }

    closesocket(udp_s);
    WSACleanup();
    return 0;
}
void recvThread(void* sock) {
    char msgbuf[DEFAULT_BUFLEN];
    SOCKET udp_s = (SOCKET)sock;
    struct sockaddr_in si_other;
    int addrlen = sizeof(si_other);
    int recvlen = 0;
    char ipstringbuffer[46];
    memset(ipstringbuffer, 0, 46);

    while (1) {
        memset(msgbuf, 0, DEFAULT_BUFLEN);
        if ((recvlen = recvfrom(udp_s, msgbuf, DEFAULT_BUFLEN, 0, (struct sockaddr*) & si_other, &addrlen)) == SOCKET_ERROR) {
            printf("recvfrom() fail with error code: %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        inet_ntop(AF_INET, &si_other.sin_addr, ipstringbuffer, 46);
        printf("\r                 \t\t\t\t|Receive from %s:%d\n", ipstringbuffer, ntohs(si_other.sin_port));
        printf("\t\t\t\t\t\t|Data: %s\n", msgbuf);
        printf("Enter Sentence:");

        if (strcmp(msgbuf, "quit") == 0)
            break;
    }
    closesocket(udp_s);
    _endthread();
}