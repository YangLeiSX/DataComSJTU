#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <process.h>

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define OK_IMAGE  "HTTP/1.0 200 OK\r\nContent-Type:image/gif\r\n\r\n"
#define OK_TEXT   "HTTP/1.0 200 OK\r\nContent-Type:text/html\r\n\r\n"
#define NOTOK_404 "HTTP/1.0 404 Not Found\r\nContent-Type:text/html\r\n\r\n"
#define MESS_404  "<html><body><h1>FILE NOT FOUND</h1></body></html>"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "80"

void handle_get(void* in_arg);

int __cdecl main(void)
{
    // Initialize variables
    WSADATA wsaData;
    int iResult;
    // Sockets for data transfer
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET client_s = INVALID_SOCKET;
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

    while (1) {
        printf("\nmain loop: listening...\n");
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
        client_s = accept(ListenSocket, (struct sockaddr*) & client_addr, &addr_len);
        memcpy(&client_ip_addr, &client_addr.sin_addr.s_addr, 4);
        if (client_s == INVALID_SOCKET) {
            printf("ERROR - Unable to create a socket \n");
            closesocket(ListenSocket);
            WSACleanup();
            exit(1);
        }

        // Display client information
        printf("\nConnection %d accepted!!\n", client_s);
        inet_ntop(AF_INET, &client_ip_addr, ipstringbuffer, sizeof(ipstringbuffer));
        printf("\tClient socket number: %d\n", client_s);
        printf("\tIPv4 address: %s\n", ipstringbuffer);
        printf("\tPort number: %d\n", ntohs(client_addr.sin_port));
        
        // Create a server to deal with the client 
        if (_beginthread(handle_get, 4096, (void*)client_s) < 0) {
            printf("ERROR - Unable to create thread \n");
            exit(1);
        }
    }
    printf("main loop completed.\n");
    // cleanup
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}

void handle_get(void* in_arg) {
    // Variables for data transfer
    SOCKET  client_s;
    char    in_buf[DEFAULT_BUFLEN];
    char    out_buf[DEFAULT_BUFLEN];
    int     retcode;
    int     j;
    // Variables for file operation
    int     fh;
    int     buf_len;
    // Variables to resolve request
    char    command[DEFAULT_BUFLEN];
    char    file_name[DEFAULT_BUFLEN];
    ZeroMemory(command, DEFAULT_BUFLEN);
    ZeroMemory(file_name, DEFAULT_BUFLEN);
    
    // Get the Socket
    client_s = (SOCKET)in_arg;
    printf("\nthread %d\n", client_s);
    // Receive the request
    retcode = recv(client_s, in_buf, DEFAULT_BUFLEN, 0);
    printf("thread %d receive web request\n", client_s);
    for (j = 0; j < retcode; j++)
        printf("%c", in_buf[j]);
    printf("\n");
    if (retcode <= 0) {
        printf("ERROR - Receive failed\n");
        closesocket(client_s);
        _endthread();
    }
    // Resolve the request
    sscanf_s(in_buf, "%s %s \n", &command, DEFAULT_BUFLEN, &file_name, DEFAULT_BUFLEN);
    // Check the command
    if (strcmp(command, "GET") != 0) {
        printf("ERROR - not a get receive %s\n", command);
        closesocket(client_s);
        _endthread();
    }
    // Open the requested file
    _sopen_s(&fh, &file_name[1], _O_RDONLY | _O_BINARY,
        _SH_DENYNO, _S_IREAD | _S_IWRITE);
    if (fh == -1) {
        printf("File %s not found\n", &file_name[1]);
        strcpy_s(out_buf, NOTOK_404);
        send(client_s, out_buf, strlen(out_buf), 0);
        strcpy_s(out_buf, MESS_404);
        send(client_s, out_buf, strlen(out_buf), 0);
        closesocket(client_s);
        _endthread();
    }
    // Validate the file name
    if (((file_name[1] == '.') && (file_name[2] == '.')) ||
        (file_name[1] == '/') || (file_name[1] == '\\') ||
        (file_name[2] == ':')) {
        printf("SECURITY VIOLATION file %s\n", &file_name[1]);
        _close(fh);
        closesocket(client_s);
        _endthread();
    }
    // Send the HTTP head
    printf("Thread %d, file %s begin\n", client_s, &file_name[1]);
    if (strstr(file_name, ".gif") != NULL)
        strcpy_s(out_buf, OK_IMAGE);
    else
        strcpy_s(out_buf, OK_TEXT);
    send(client_s, out_buf, strlen(out_buf), 0);
    // Read file and send the HTTP body
    while (!_eof(fh)) {
        buf_len = _read(fh, out_buf, DEFAULT_BUFLEN);
        send(client_s, out_buf, buf_len, 0);
    }
    _close(fh);
    printf("Thread %d, file %s completed\n", client_s, &file_name[1]);
    // Send finish
    closesocket(client_s);
    printf("Socket %d closed\n", client_s);
    printf("Thread %d closed\n", client_s);
    _endthread();
}
