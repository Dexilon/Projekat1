#include <winsock2.h>
#include <stdio.h>
#include <conio.h>
#include <WS2tcpip.h>
#include <time.h>

#define SERVER_PORT 15000
#define OUTGOING_BUFFER_SIZE 1024
// for demonstration purposes we will hard code
// local host ip adderss
#define SERVER_IP_ADDERESS "127.0.0.1"
#define TIMEOUT 5

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();
void SelectFun(SOCKET* listenSocket, int n);
void Send(SOCKET socket, char* message, int length);
int Receive(SOCKET socket, char* message, int length);

int main(int argc, char* argv[])
{
	// Server address
	sockaddr_in serverAddress;
	// size of sockaddr structure    
	int sockAddrLen = sizeof(struct sockaddr);
	// buffer we will use to store message
	char outgoingBuffer[OUTGOING_BUFFER_SIZE];
	// port used for communication with server
	int serverPort = SERVER_PORT;
	// variable used to store function return value
	int iResult;

	// Initialize windows sockets for this process
	InitializeWindowsSockets();

	// Initialize serverAddress structure
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	inet_pton(AF_INET,SERVER_IP_ADDERESS, &(serverAddress.sin_addr));
	serverAddress.sin_port = htons((u_short)serverPort);

	// create a socket
	SOCKET clientSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram socket
		IPPROTO_UDP); // UDP

					  // check if socket creation succeeded
	if (clientSocket == INVALID_SOCKET)
	{
		printf("Creating socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Set socket to nonblocking mode
	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(clientSocket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	time_t start;
	time_t stop;
	time_t difference;

	while (1) {
		printf("Enter message for server:\n");

		// Read string from user into outgoing buffer
		gets_s(outgoingBuffer, OUTGOING_BUFFER_SIZE);

		SelectFun(&clientSocket, 0);

		iResult = sendto(clientSocket,
			outgoingBuffer,
			strlen(outgoingBuffer),
			0,
			(LPSOCKADDR)&serverAddress,
			sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("sendto failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		start = time(NULL);

		char accessBuffer[6];

		SelectFun(&clientSocket, 1);

		iResult = recvfrom(clientSocket,
			accessBuffer,
			1024,
			0,
			(LPSOCKADDR)&serverAddress,
			&sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("sendto failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		stop = time(NULL);

		difference = start - stop;

		printf("%ld",difference);

		if (difference > TIMEOUT) {
			printf("Timeout!");
		}

		printf("%s\n", accessBuffer);
	}

	_getch();

	iResult = closesocket(clientSocket);
	if (iResult == SOCKET_ERROR)
	{
		printf("closesocket failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printf("WSACleanup failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	return 0;
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}
	return true;
}

void SelectFun(SOCKET* listenSocket, int n) {

	int iResult;
	FD_SET set;
	timeval timeVal;

	time_t value,value1,difference;

	value = time(NULL);

	do {

		FD_ZERO(&set);
		// Add socket we will wait to read from
		FD_SET(*listenSocket, &set);

		// Set timeouts to zero since we want select to return
		// instantaneously
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		value1 = time(NULL);

		difference = value1 - value;

		if (difference > 5)
			printf("Isteklo vreme!");

		if (n == 1) {
			//Receive
			iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		}
		else {
			iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);
		}

		// lets check if there was an error during select
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			continue;
		}

		if (iResult == 0)
		{
			// there are no ready sockets, sleep for a while and check again
			Sleep(50);
			continue;
		}
	} while (iResult != 1);

}

void Send(SOCKET socket, char* message, int length) {
	int iResult, fileLen = 0;
	do {
		SelectFun(&socket, 0);
		iResult = send(socket, message + fileLen, length - fileLen, 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(socket);
			WSACleanup();
			break;
		}

		fileLen += iResult;

	} while (fileLen < length);
}

int Receive(SOCKET socket, char* message, int length) {
	int iResult, fileLen = 0;
	do {

		SelectFun(&socket, 1);
		iResult = recv(socket, message + fileLen, length - fileLen, 0);

		fileLen += iResult;

	} while (fileLen < length);

	return fileLen;
}