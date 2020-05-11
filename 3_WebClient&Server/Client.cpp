#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "80"

int __cdecl main(int argc, char** argv)
{
    // Initialize variables
    WSADATA wsaData;
    int iResult;
    // Client socket 
    SOCKET ClientSocket = INVALID_SOCKET;
    // Variables to resolve hostname
    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    struct addrinfo hints;
    // Variables to transfer
    int recvbuflen = DEFAULT_BUFLEN;
    char sendbuf[DEFAULT_BUFLEN] = "\0";
    char recvbuf[DEFAULT_BUFLEN] = "\0";
    const char SimGetHead[DEFAULT_BUFLEN] = "GET / HTTP/1.1\r\n\r\n";
    char GetHead[DEFAULT_BUFLEN] = "GET / HTTP/1.1\r\nAccept: text/html\r\nHost: DESKTOP-LEODORM\r\n";

    // Validate the parameters
    if (argc < 2) {
        printf("usage: %s server-name file_name\n", argv[0]);
        return 1;
    }
    if(argc == 3)
        snprintf(GetHead, DEFAULT_BUFLEN,
            "GET /%s HTTP/1.1\r\nAccept: text/html\r\nHost: DESKTOP-LEODORM\r\n",
            argv[2]);

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Resolve the server address and port
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ClientSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ClientSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
        printf("Create socket...");
        // Connect to server.
        iResult = connect(ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ClientSocket);
            ClientSocket = INVALID_SOCKET;
            printf("failed.\n");
            continue;
        }
        printf("Done!\n");
        break;
    }
    freeaddrinfo(result);

    if (ClientSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    printf("Connect Successfully!\n");

    // Send Message
    memcpy(sendbuf, GetHead, strlen(GetHead) + 1);
    // memcpy(sendbuf, SimGetHead, strlen(GetHead) + 1);
    iResult = send(ClientSocket, sendbuf, int(strlen(sendbuf) + 1), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send fail with %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    printf("Sented: \n%s", sendbuf);
    printf("Bytes Sent: %ld\n", iResult);

    // Shutdown the connection since no more data will be sent
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // Receive information
    do {
        printf("Receiving...\n");
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("\n%s\n", recvbuf);
            printf("Bytes received: %d\n\n", iResult);
        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());
    } while (iResult > 0);
    
    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}
