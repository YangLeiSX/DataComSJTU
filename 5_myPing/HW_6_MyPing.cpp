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

// ICMP header
typedef struct icmp_hdr {
	unsigned char	icmp_type;
	unsigned char	icmp_code;
	unsigned short	icmp_checksum;
	unsigned short	icmp_id;
	unsigned short	icmp_sequence;
} ICMP_HDR;

// ICMP Constants
#define ICMP_ECHO_REQUEST_TYPE	8
#define ICMP_ECHO_REQUEST_CODE	0
#define ICMP_ECHO_REPLY_TYPE	0
#define ICMP_ECHO_REPLY_CODE	0
#define ICMP_MINIMUM_HEADER		8

// Default Configuration
#define DEFAULT_BUFLEN			512
#define DEFAULT_DATA_SIZE       32
#define DEFAULT_SEND_COUNT      12
#define DEFAULT_RECV_TIMEOUT    4000
#define DEFAULT_TTL             64
#define MAX_RECV_BUF_LEN        0xFFFF

// Compute checksum for buffer
USHORT checksum(USHORT* buffer, int size);
// Set checksum for ICMP package
void ComputeIcmpchecksum(char* buf, int packetlen);
// Set sequence number for ICMP package
void SetIcmpSequence(char* buf);
// Get name info from address
int	PrintAddress(struct sockaddr* sa, int salen);
// Get address info from address
struct addrinfo* ResolveAddress(char* addr, char* port, int af, int type, int proto);
// Set overlapped I/O
int PostRecvfrom(SOCKET s, char* buf, int buflen, SOCKADDR* from, int* fromlen, WSAOVERLAPPED* ol);

int __cdecl main(int argc, char* argv[]) {
	// variables for initialize
	WSADATA wsd;
	int iResult;
	// cmd parameter
	char* destHost = NULL;
	// variables for resolve address
	char Port[] = "0";
	int AddrFamily = AF_UNSPEC;
	struct addrinfo* remote = NULL;
	struct addrinfo* local = NULL;
	// socket and socket option
	SOCKET s = INVALID_SOCKET;
	int SocketType = SOCK_RAW;
	int Proto = IPPROTO_ICMP;
	int Ttl = DEFAULT_TTL;
	// variables to create icmp packet space
	int packetlen = 0;
	int dataSize = DEFAULT_DATA_SIZE;
	char* icmpbuf = NULL;
	// variables to setup icmp package
	ICMP_HDR* icmp_hdr = NULL;
	char* datapart = NULL;
	// create overlapped I/O event
	WSAOVERLAPPED recvol;
	recvol.hEvent = WSA_INVALID_EVENT;
	// variables for transfer
	SOCKADDR_STORAGE from;
	int fromlen = sizeof(SOCKADDR_STORAGE);
	DWORD flags, bytes;
	char recvbuf[MAX_RECV_BUF_LEN];
	int recvbuflen = MAX_RECV_BUF_LEN;
	// counters
	ULONG time = 0;
	UINT RecvPack = 0;
	UINT Low = 1000;
	UINT High = 0;
	UINT SumTime = 0;

	// check arguments
	printf("------------------------------------------------------------------------\n");
	printf("Check arguments...");
	if (argc != 2) {
		printf("Usage: %s URL", argv[0]);
		return -1;
	}
	destHost = argv[1];
	printf("\t\tDone.\n");

	// initialise
	printf("WSAStartup...");
	if ((iResult = WSAStartup(MAKEWORD(2, 2), &wsd)) != 0) {
		printf("WSAStartup failed with %d\n", iResult);
		return -1;
	}
	printf("\t\t\tDone.\n");

	// resolve target address
	printf("Resolve remote address...");
	remote = ResolveAddress(destHost, Port, AddrFamily, 0, 0);
	AddrFamily = remote->ai_family;
	printf("\tDone.\n");

	// resolve local address
	printf("Resolve local address...");
	local = ResolveAddress(NULL, Port, AddrFamily, 0, 0);
	printf("\tDone.\n");

	// create socket
	printf("Create socket...");
	s = socket(AddrFamily, SocketType, Proto);
	if (s == INVALID_SOCKET) {
		printf("socket() failed with %d\n", WSAGetLastError());
		freeaddrinfo(remote);
		freeaddrinfo(local);
		WSACleanup();
		return -1;
	}
	printf("\t\tDone.\n");

	// setup ttl
	printf("Set ttl value...");
	iResult = setsockopt(s, IPPROTO_IP, IP_TTL, (char*)&Ttl, sizeof(int));
	if (iResult == SOCKET_ERROR) {
		printf("setsockopt failed with %d\n", GetLastError());
		freeaddrinfo(remote);
		freeaddrinfo(local);
		closesocket(s);
		WSACleanup();
		return -1;
	}
	printf("\t\tDone.\n");

	// create icmp packge space
	printf("Create send buffer...");
	packetlen += sizeof(ICMP_HDR);
	packetlen += dataSize;
	icmpbuf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, packetlen);
	if (icmpbuf == NULL) {
		printf("HeapAlloc failed with %d\n", GetLastError());
		freeaddrinfo(remote);
		freeaddrinfo(local);
		closesocket(s);
		WSACleanup();
		return -1;
	}
	memset(icmpbuf, 0, packetlen);
	printf("\t\tDone.\n");

	// setup icmp packge header
	printf("Set ICMP header...");
	icmp_hdr = (ICMP_HDR*)icmpbuf;
	icmp_hdr->icmp_type = ICMP_ECHO_REQUEST_TYPE;
	icmp_hdr->icmp_code = ICMP_ECHO_REQUEST_CODE;
	icmp_hdr->icmp_id = (unsigned short)GetCurrentProcessId();
	icmp_hdr->icmp_sequence = 0;
	icmp_hdr->icmp_checksum = 0;
	printf("\t\tDone.\n");

	// setup icmp packge data
	printf("Set ICMP header...");
	datapart = icmpbuf + sizeof(ICMP_HDR);
	memset(datapart, 'Q', dataSize);
	printf("\t\tDone.\n");

	// bind socket
	printf("Bind socket...");
	iResult = bind(s, local->ai_addr, (int)local->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with %d\n", WSAGetLastError());
		freeaddrinfo(remote);
		freeaddrinfo(local);
		closesocket(s);
		HeapFree(GetProcessHeap(), 0, icmpbuf);
		WSACleanup();
		return -1;
	}
	printf("\t\t\tDone.\n");

	// create wait event
	printf("Create overlapped event...");
	memset(&recvol, 0, sizeof(WSAOVERLAPPED));
	recvol.hEvent = WSACreateEvent();
	if (recvol.hEvent == WSA_INVALID_EVENT) {
		printf("WSACreateEvent faild with %d\n", WSAGetLastError());
		freeaddrinfo(remote);
		freeaddrinfo(local);
		closesocket(s);
		HeapFree(GetProcessHeap(), 0, icmpbuf);
		WSACleanup();
		return -1;
	}
	printf("\tDone.\n");

	// setup overlapped receive
	printf("Set receive event...");
	PostRecvfrom(s, recvbuf, recvbuflen, (SOCKADDR*)&from, &fromlen, &recvol);
	printf("\t\tDone.\n");

	// ping information
	printf("------------------------------------------------------------------------\n");
	printf("\nPinging: %s", destHost);
	PrintAddress(remote->ai_addr, remote->ai_addrlen);
	printf(" with %d bytes of data\n\n", dataSize);

	// begin ping
	for (int i = 0; i < DEFAULT_SEND_COUNT; i++) {
		// set up sequence
		SetIcmpSequence(icmpbuf);
		
		// set up checksum
		ComputeIcmpchecksum(icmpbuf, packetlen);
		
		// send package
		time = (ULONG)GetTickCount64();
		iResult = sendto(s, icmpbuf, packetlen, 0, remote->ai_addr, (int)remote->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("sendto failed with %d\n", WSAGetLastError());
			freeaddrinfo(remote);
			freeaddrinfo(local);
			closesocket(s);
			HeapFree(GetProcessHeap(), 0, icmpbuf);
			WSACloseEvent(recvol.hEvent);
			WSACleanup();
			return -1;
		}

		// wait for reply
		iResult = WaitForSingleObject((HANDLE)recvol.hEvent, DEFAULT_RECV_TIMEOUT);
		if (iResult == WAIT_FAILED) {
			printf("WaitForSigleObject failed with %d\n", WSAGetLastError());
			freeaddrinfo(remote);
			freeaddrinfo(local);
			closesocket(s);
			HeapFree(GetProcessHeap(), 0, icmpbuf);
			WSACloseEvent(recvol.hEvent);
			WSACleanup();
			return -1;
		}
		else if (iResult == WAIT_TIMEOUT) {
			printf("Request Time Out.\n");
		}
		else {
			// check result
			iResult = WSAGetOverlappedResult(s, &recvol, &bytes, FALSE, &flags);
			if (iResult == FALSE) {
				printf("WSAGetOverlappedResult failed with %d\n", WSAGetLastError());
			}
			WSAResetEvent(recvol.hEvent);
			// update log value
			time = (ULONG)GetTickCount64() - time;
			RecvPack += 1;
			SumTime += time;
			if (time < Low)
				Low = time;
			if (time > High)
				High = time;
			// display information
			printf("Replay From");
			PrintAddress((SOCKADDR*)&from, fromlen);
			if (time == 0) {
				printf(": bytes = %d time < 1 ms TTL = %d\n", dataSize, Ttl);
			}
			else {
				printf(": bytes = %d time = %d ms TTL = %d\n", dataSize, time, Ttl);
			}
			if (i < DEFAULT_SEND_COUNT - 1) {
				fromlen = sizeof(SOCKADDR_STORAGE);
				// setup overlapped receive
				PostRecvfrom(s, recvbuf, recvbuflen, (SOCKADDR*)&from, &fromlen, &recvol);
			}
		}	
		Sleep(1000);

	}
	printf("------------------------------------------------------------------------\n");
	printf("Send %d packages,  Receive %d packages,  Loss %d(%0.2lf%%) packages.\n",
			DEFAULT_SEND_COUNT, RecvPack, DEFAULT_SEND_COUNT - RecvPack,
		(DEFAULT_SEND_COUNT - RecvPack) * 100.0 / DEFAULT_SEND_COUNT);
	printf("Max time %d ms,  Minimum time %d ms,  Average time %d ms.\n",
		High, Low, SumTime / RecvPack);
	printf("-------------------------------------------------------------------------\n");
	// cleanup
	WSACleanup();
	return 0;
}

// Compute checksum for buffer
USHORT checksum(USHORT* buffer, int size) {
	unsigned long cksum = 0;
	while (size > 1) {
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}
	if (size) {
		cksum += *(UCHAR*)buffer;
	}
	// addup all bytes
	cksum = (cksum >> 16) + (cksum & 0xffff);
	// add carryup digit
	cksum += (cksum >> 16);
	return (USHORT)(~cksum);
}

// Set checksum for ICMP package
void ComputeIcmpchecksum(char* buf, int packetlen) {
	ICMP_HDR* icmpv4 = NULL;
	icmpv4 = (ICMP_HDR*)buf;
	icmpv4->icmp_checksum = 0;
	icmpv4->icmp_checksum = checksum((USHORT*)buf, packetlen);
}

// Set sequence number for ICMP package
void SetIcmpSequence(char* buf) {
	ULONG sequence = 0;
	sequence = (ULONG)GetTickCount64();
	ICMP_HDR* icmpv4 = NULL;
	icmpv4 = (ICMP_HDR*)buf;
	icmpv4->icmp_sequence = (USHORT)sequence;
}

// Get name info from address
int	PrintAddress(struct sockaddr* sa, int salen) {
	//variables for nameinfo
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	int	hostlen = NI_MAXHOST;
	int servlen = NI_MAXSERV;
	int iResult;

	iResult = getnameinfo(
		sa, salen,
		host, hostlen,
		serv, servlen,
		NI_NUMERICHOST | NI_NUMERICSERV
	);
	if (iResult != 0)
	{
		printf("getnameinfo failed: %d\n", iResult);
		return SOCKET_ERROR;
	}
	// display nameinfo
	if (strcmp(serv, "0") != 0) {
		printf(" [%s:%s]", host, serv);
	}
	else {
		printf(" [%s]", host);
	}
	return NO_ERROR;
}

// Get address info from address
struct addrinfo* ResolveAddress(char* addr, char* port, int af, int type, int proto) {
	// variables for address info
	struct addrinfo hints, * res = NULL;
	int iResult;
	// setup the hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = ((addr) ? 0 : AI_PASSIVE);
	hints.ai_family = af;
	hints.ai_socktype = type;
	hints.ai_protocol = proto;

	iResult = getaddrinfo(addr, port, &hints, &res);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		return NULL;
	}
	return res;
}

// Set overlapped I/O
int PostRecvfrom(SOCKET s, char* buf, int buflen, SOCKADDR* from, int* fromlen, WSAOVERLAPPED* ol) {
	// variables for recvfrom
	WSABUF wbuf;
	DWORD flags, bytes;
	int iResult;
	// setup 
	wbuf.buf = buf;
	wbuf.len = buflen;
	flags = NULL;
	iResult = WSARecvFrom(s, &wbuf, 1, &bytes, &flags, from, fromlen, ol, NULL);
	if (iResult == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			printf("WSARecvfrom failed: %d\n", WSAGetLastError());
			return SOCKET_ERROR;
		}
	}
	return NO_ERROR;
}
