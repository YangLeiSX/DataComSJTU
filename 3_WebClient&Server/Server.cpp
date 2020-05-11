#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "80"

int __cdecl main(void)
{
    // Initialize variables
    WSADATA wsaData;
    int iResult;
    // Sockets for data transfer
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
    // Variables to resolve server hostname and address
    char hostname[NI_MAXHOST];
    struct sockaddr_in server_addr;
    char ipstringbuffer[46];
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    // Variables to resolve client address
    struct sockaddr_in  client_addr;
    struct in_addr client_ip_addr;
    int addr_len;
    // Variables to transfer
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];
    int iSendResult;
    char sendbuf[DEFAULT_BUFLEN];
    const char HelloPage[] = "<!DOCTYPE html>\r\n<html>\r\n<head><title>Response</title></head>\r\n<body>\r\n<h1>Response</h1>\r\n</body>\r\n</html>";
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Get server hostname to resolve 
    gethostname(hostname, NI_MAXHOST);
    printf("hostname: %s\n", hostname);

    // Resolve the server address and port
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(hostname, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    // Display server information
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

    // Accept a client connection
    addr_len = sizeof(client_addr);
    ClientSocket = accept(ListenSocket, (struct sockaddr*)&client_addr, &addr_len);
    memcpy(&client_ip_addr, &client_addr.sin_addr.s_addr, 4);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("Connected successfully.\n");
    // Display client information
    inet_ntop(AF_INET, &client_ip_addr, ipstringbuffer, sizeof(ipstringbuffer));
    printf("Client socket number: %d\n", ClientSocket);
    printf("IPv4 address: %s\n", ipstringbuffer);
    printf("Port number: %d\n", ntohs(client_addr.sin_port));
    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);
            printf("Received: \n%s", recvbuf);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);
    printf("\n****************\nReceive Finish!\n****************\n");
    
    // Send the web page to any client
    memcpy(sendbuf, HelloPage, strlen(HelloPage)+1);
    iSendResult = send(ClientSocket, sendbuf, (strlen(HelloPage)+1), 0);
    if (iSendResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    printf("Bytes sent: %d\n", iSendResult);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}
