#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <mswsock.h>

#pragma comment(lib, "ws2_32.lib")

#if __STDC_VERSION__ < 202311L
#define nullptr NULL
#endif

typedef enum IoKind
{
	IoREAD,
	IoWRITE
} IoKind;

typedef struct IoData
{
	OVERLAPPED Overlapped;
	WSABUF wsabuf;
	DWORD nBytes;
	IoKind opCode;
	SOCKET cliSock;
} IoData;

void CloseConn(IoData* ctx);

BOOL PostRead(IoData* data)
{
	memset(&data->Overlapped, 0, sizeof(data->Overlapped));
	data->opCode = IoREAD;
	DWORD dwFlags = 0;
	int nRet = WSARecv(data->cliSock, &data->wsabuf, 1, &data->nBytes, &dwFlags, &data->Overlapped, nullptr);
	if (nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()))
	{
		printf("WASRecv Failed:%d\n", WSAGetLastError());
		CloseConn(data);
		return FALSE;
	}
	return TRUE;
}

BOOL PostWrite(IoData* data)
{
	memset(&data->Overlapped, 0, sizeof(data->Overlapped));
	data->opCode = IoWRITE;
	printf("%lld Send %s\n", data->cliSock, data->wsabuf.buf);
	int nRet = WSASend(data->cliSock, &data->wsabuf, 1, &data->nBytes, 0, &data->Overlapped, nullptr);
	if (nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError()))
	{
		printf("WASSend Failed:%d", WSAGetLastError());
		CloseConn(data);
		return FALSE;
	}
	return TRUE;
}

DWORD WINAPI WorkerThread(HANDLE hIOCP)
{
	IoData* ctx = nullptr;
	DWORD dwIoSize = 0;
	void* lpCompletionKey = NULL;
	LPOVERLAPPED lpOverlapped = nullptr;

	while (1)
	{
		GetQueuedCompletionStatus(hIOCP, &dwIoSize, (PULONG_PTR)&lpCompletionKey, &lpOverlapped, INFINITE);
		ctx = (IoData*)lpOverlapped;
		if (dwIoSize == 0)
		{
			if (ctx == NULL)
			{
				printf("WorkerThread Exit...\n");
				break;
			}
			printf("Client:%lld disconnect\n", ctx->cliSock);
			CloseConn(ctx);
			continue;
		}
		ctx->nBytes = dwIoSize;
		if (ctx->opCode == IoREAD)
		{
			ctx->wsabuf.buf[dwIoSize] = 0;
			PostWrite(ctx);
		}
		else if (ctx->opCode == IoWRITE)
		{
			PostRead(ctx);
		}
	}
	return 0;
}

static SOCKET curAcceptSock = INVALID_SOCKET;

void CloseConn(IoData* ctx)
{
	if (ctx->Overlapped.hEvent != NULL)
	{
		WSACloseEvent(ctx->Overlapped.hEvent);
		ctx->Overlapped.hEvent = NULL;
	}
	if (ctx->wsabuf.buf != NULL)
	{
		free(ctx->wsabuf.buf);
		ctx->wsabuf.buf = nullptr;
	}
	closesocket(ctx->cliSock);
	free(ctx);
	curAcceptSock = INVALID_SOCKET;
}

void OnSignal(int sig)
{
	if (curAcceptSock != INVALID_SOCKET)
	{
		closesocket(curAcceptSock);
	}
	printf("Recv exit signal...\n");
}

void NetWork(int port)
{
	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0)
	{
		printf("Init Network failed:%d", res);
		return;
	}

	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listenSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("bind failed:%d", WSAGetLastError());
		return;
	}
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		printf("listen failed:%d", WSAGetLastError());
		return;
	}

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	DWORD threadCount = sysInfo.dwNumberOfProcessors;

	HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threadCount);
	if (hIOCP == NULL)
	{
		printf("CreateIoCompletionPort failed:%lu", GetLastError());
		return;
	}
	for (int i = 0; i < threadCount; ++i)
	{
		DWORD dwThreadId = 0;
		HANDLE hThread = CreateThread(nullptr, 0, WorkerThread, hIOCP, 0, &dwThreadId);
		if (hThread == NULL)
		{
			printf("CreateThread failed:%lu", GetLastError());
			continue;
		}
		CloseHandle(hThread);
	}

	LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;

	// Load the AcceptEx function into memory using WSAIoctl.
	// The WSAIoctl function is an extension of the ioctlsocket()
	// function that can use overlapped I/O. The function's 3rd
	// through 6th parameters are input and output buffers where
	// we pass the pointer to our AcceptEx function. This is used
	// so that we can call the AcceptEx function directly, rather
	// than refer to the Mswsock.lib library.
	int iResult = WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		(LPVOID)&lpfnAcceptEx, sizeof(lpfnAcceptEx),
		&dwBytes, nullptr, nullptr);
	if (iResult == SOCKET_ERROR)
	{
		wprintf(L"WSAIoctl failed with error: %u\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	while (1)
	{
		IoData* data = (IoData*)malloc(sizeof(IoData));
		if (data == NULL)
		{
			printf("out of memory");
			break;
		}

		SOCKET cliSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		curAcceptSock = cliSock;

		data->cliSock = cliSock;
		data->opCode = IoREAD;
		data->wsabuf.len = 1024;
		data->wsabuf.buf = malloc(data->wsabuf.len);
		if (data->wsabuf.buf == NULL)
		{
			printf("out of memory");
			CloseConn(data);
			break;
		}
		ZeroMemory(&data->Overlapped, sizeof(data->Overlapped));
		data->Overlapped.hEvent = WSACreateEvent();
		if (data->Overlapped.hEvent == NULL)
		{
			printf("WSACreateEvent failed with error: %d\n", WSAGetLastError());
			CloseConn(data);
			break;
		}
		BOOL bRetVal = lpfnAcceptEx(listenSocket, cliSock, data->wsabuf.buf,
			data->wsabuf.len - ((sizeof(struct sockaddr_in) + 16) * 2),
			sizeof(struct sockaddr_in) + 16, sizeof(struct sockaddr_in) + 16,
			&dwBytes, &data->Overlapped);
		if (!bRetVal)
		{
			int err = WSAGetLastError();
			if (err != ERROR_IO_PENDING)
			{
				printf("AcceptEx failed with error: %d\n", err);
				CloseConn(data);
				break;
			}
		}
		DWORD res = WSAWaitForMultipleEvents(1, &data->Overlapped.hEvent, 1, 100, 1);
		if (res == WSA_WAIT_FAILED)
		{
			printf("AcceptEx failed with error: %d\n", WSAGetLastError());
			CloseConn(data);
			break;
		}
		WSAResetEvent(data->Overlapped.hEvent);
		DWORD flag = 0;
		BOOL ret = WSAGetOverlappedResult(cliSock, &data->Overlapped, &data->nBytes, 1, &flag);
		if (!ret)
		{
			CloseConn(data);
			break;
		}
		if (data->nBytes == 0)
		{
			CloseConn(data);
			continue;
		}
		printf("Client:%lld connected.\n", cliSock);
		if (CreateIoCompletionPort((HANDLE)cliSock, hIOCP, 0, 0) == nullptr)
		{
			printf("Binding Client Socket to IO Completion Port Failed:%lu\n", GetLastError());
			CloseConn(data);
			continue;
		}
		data->wsabuf.buf[data->nBytes] = 0;
		PostWrite(data);
	}
	for (int i = 0; i < threadCount; ++i)
	{
		PostQueuedCompletionStatus(hIOCP, 0, 0, nullptr);
	}
	closesocket(listenSocket);
	WSACleanup();
}

int main()
{
	SetConsoleOutputCP(65001);
	typedef void (*SignalHandlerPointer)(int);
	SignalHandlerPointer previousHandler = signal(SIGINT, OnSignal);
	NetWork(6000);
	printf("exit\n");
	return 0;
}
