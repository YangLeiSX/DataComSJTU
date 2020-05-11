/**
 * @file: GetHostAddress.cpp
 * @
 */
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

void main(int argc, char* argv[]) {
	/**
	 * Check Parameters
	 */
	if (argc != 4) {
		std::cout << "usage: HW_2.exe A www.github.com 80" << std::endl;
		return 1;
	}

	/**
	 * Initialize Winsock
	 */
	WORD wVersion = MAKEWORD(2, 2);
	WSADATA wsaDATA;
	int iResult;

	// initialize winsock
	iResult = WSAStartup(wVersion, &wsaDATA);
	if (0 != iResult) {
		std::cout << "WSAStartup failed : " << iResult << std::endl;
		return 1;
	}

	/**
	 * Get Host Address
	 */
	DWORD dwRetVal;
	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;
	
	// setup hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	std::cout << "getaddrinfo with parameters: " << std::endl;
	std::cout << "\tnodename : " << argv[2] << std::endl;
	std::cout << "\tservname : " << argv[3] << std::endl << std::endl;

	// get information
	dwRetVal = getaddrinfo(argv[2], argv[3], &hints, &result);
	if (0 != dwRetVal) {
		std::cout << "getaddrinfo failed with #" << dwRetVal << std::endl;
		WSACleanup();
		return 1;
	}
	std::cout << "getaddrinfo successfully" << std::endl;

	// parse the result
	struct sockaddr_in *sockaddr_ipv4;
	struct sockaddr_in6* sockaddr_ipv6;
	DWORD ipbufferlength = 46;
	char ipstringbuffer[46];

	int i = 1;
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		std::cout << "getaddrinfot responce " << i++ << std::endl;
		std::cout << "\tFlags : 0x" << std::hex << ptr->ai_flags << std::dec << std::endl;
		// display address family
		std::cout << "\tFamily: ";
		switch (ptr->ai_family) {
		case AF_UNSPEC:
			std::cout << "Unspecified" << std::endl; break;
		case AF_INET:
			std::cout << "AF_INET(IPv4)" << std::endl; 
			sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;
			inet_ntop(AF_INET, 
				&sockaddr_ipv4->sin_addr, 
				ipstringbuffer, 
				sizeof(ipstringbuffer));
			std::cout << "\tIPV4 address " << ipstringbuffer << std::endl;
			std::cout << "\tport = " << ntohs(sockaddr_ipv4->sin_port) << std::endl;
			break;
		case AF_INET6:
			std::cout << "AF_INET6(IPv6)" << std::endl; 
			sockaddr_ipv6 = (struct sockaddr_in6*)ptr->ai_addr;
			inet_ntop(AF_INET6,
				&sockaddr_ipv6->sin6_addr,
				ipstringbuffer,
				ipbufferlength);
			std::cout << "\tIPv6 address " << ipstringbuffer << std::endl;
			std::cout << "\tport = " << ntohs(sockaddr_ipv6->sin6_port) << std::endl;
			break;
		case AF_NETBIOS:
			std::cout << "AF_NETBIOS(NetBIOS)" << std::endl; break;
		default:
			std::cout << "Other " << ptr->ai_family << std::endl; break;
		}
		// display socket type
		std::cout << "\tSocket Type: ";
		switch (ptr->ai_socktype) {
		case 0:
			std::cout << "Unspecified" << std::endl; break;
		case SOCK_STREAM:
			std::cout << "SOCK_STREAM(stream)" << std::endl; break;
		case SOCK_DGRAM:
			std::cout << "SOCK_DGRAM(datagram)" << std::endl; break;
		case SOCK_RAW:
			std::cout << "SOCK_RAW(raw)" << std::endl; break;
		case SOCK_RDM:
			std::cout << "SOCK_RDM(reliable message datagram)" << std::endl; break;
		case SOCK_SEQPACKET:
			std::cout << "SOCK_SEQPACKET(pseudo-stream packet)" << std::endl; break;
		default:
			std::cout << "Other " << ptr->ai_socktype << std::endl; break;
		}
		// display protocol type
		std::cout << "\tProtocal: ";
		switch (ptr->ai_protocol) {
		case 0:
			std::cout << "Unspecified" << std::endl; break;
		case IPPROTO_TCP:
			std::cout << "IPPROTO_TCP(TCP)" << std::endl; break;
		case IPPROTO_UDP:
			std::cout << "IPPROTO_UDP(UDP)" << std::endl; break;
		default:
			std::cout << "Other " << ptr->ai_protocol << std::endl; break;
		}
		// display other information
		std::cout << "\tLength of sockaddr: " << ptr->ai_addrlen << std::endl;
		std::cout << "\tCanonical name: " << ptr->ai_canonname << std::endl;
	}

	WSACleanup();
	return 0;
}