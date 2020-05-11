/**
 * @file: GetHostName.cpp
 * @brief: get host name for given IP
 * @author: YangLei 517021910881
 * @date: 2020-04-25
 */
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[]) {
	/**
	 * Initialize WinSock
	 */
	WORD wVersion = MAKEWORD(2, 2);
	WSADATA wsaDATA;
	int iResult;

	iResult = WSAStartup(wVersion, &wsaDATA);
	if (0 != iResult) {  // fail to setup winsock
		std::cout << "WSAStartup failed : " << iResult << std::endl;
		WSACleanup();
		return 1;
	}

	/**
	 *  Get Host Name
	 */
	DWORD dwRetVal;
	struct sockaddr_in saGHN;
	char hostname[NI_MAXHOST];
	char servInfo[NI_MAXSERV];
	u_short port = 8080;
	const char* localhost = "127.0.0.1";

	// setup parameters
	if (2 == argc) {
		inet_pton(AF_INET, localhost, &saGHN.sin_addr.s_addr);
	}
	else {
		inet_pton(AF_INET, argv[2], &saGHN.sin_addr.s_addr);
	}
	saGHN.sin_family = AF_INET;
	saGHN.sin_port = htons(port);
	// get host name
	dwRetVal = getnameinfo(
		(struct sockaddr*) & saGHN,
		sizeof(struct sockaddr), 
		hostname, NI_MAXHOST,
		servInfo, NI_MAXSERV, 0);
	// display result
	if (0 != dwRetVal) {
		std::cout << "getnameinfo failed with error #" << WSAGetLastError() << std::endl;
	}
	else {
		std::cout << "getnameinfo returned hostname : " << hostname << std::endl;
	}

	WSACleanup();
	return 0;
}