#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#pragma comment(lib, "ws2_32.lib")

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

void CloseConn(IoData* ctx)
{
	closesocket(ctx->cliSock);
	free(ctx->wsabuf.buf);
	free(ctx);
}

BOOL PostRead(IoData* data)
{
	memset(&data->Overlapped, 0, sizeof(data->Overlapped));
	data->opCode = IoREAD;
	DWORD dwFlags = 0;
	int nRet = WSARecv(data->cliSock, &data->wsabuf, 1, &data->nBytes, &dwFlags, &data->Overlapped, NULL);
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
	int nRet = WSASend(data->cliSock, &data->wsabuf, 1, &data->nBytes, 0, &(data->Overlapped), NULL);
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
	IoData* ctx = NULL;
	DWORD dwIoSize = 0;
	void* lpCompletionKey = NULL;
	LPOVERLAPPED lpOverlapped = NULL;

	while (1)
	{
		GetQueuedCompletionStatus(hIOCP, &dwIoSize, (PULONG_PTR)&lpCompletionKey, (LPOVERLAPPED*)&lpOverlapped, INFINITE);
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
		if (ctx->opCode == IoREAD)
		{
			ctx->wsabuf.buf[dwIoSize] = 0;
			PostWrite(ctx, dwIoSize);
		}
		else if (ctx->opCode == IoWRITE)
		{
			PostRead(ctx);
		}
	}
	return 0;
}

static BOOL IsExit = FALSE;

void OnSignal(int sig)
{
	IsExit = TRUE;
	printf("Recv exit signal...\n");
}

void SetNonblocking(SOCKET fd)
{
	// Windows�����������������׽���Ϊ��������һ���ǳ�����ioctlsocket����һ���Ǳ�����ǿ���WSAIoctl��
#if 1
	unsigned long inBuf = 1; // ���Ҫ���÷���ֹģʽ����Ϊ����;���Ҫ���ø�ģʽ����Ϊ�㡣
	DWORD lpcbBytesReturned = 0;
	WSAOVERLAPPED Overlapped;
	ZeroMemory(&Overlapped, sizeof(Overlapped));
	if (SOCKET_ERROR == WSAIoctl(fd, FIONBIO, &inBuf, sizeof(inBuf), NULL, 0, &lpcbBytesReturned, &Overlapped, NULL))
	{
		printf("set socket:%lld non blocking failed:%d\n", fd, WSAGetLastError());
	}
#else
	unsigned long ul = 1;  // ���Ҫ���÷���ֹģʽ����Ϊ����;���Ҫ���ø�ģʽ����Ϊ�㡣
	int ret = ioctlsocket(fd, FIONBIO, &ul);
	if (ret == SOCKET_ERROR)
	{
		printf("set socket:%lld non blocking failed:%d\n", fd, WSAGetLastError());
	}
#endif
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

	SOCKET m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(m_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("bind failed:%d", WSAGetLastError());
		return;
	}
	if (listen(m_socket, 0) == SOCKET_ERROR)
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
		printf("CreateIoCompletionPort failed:%d", GetLastError());
		return;
	}
	for (int i = 0; i < threadCount; ++i)
	{
		DWORD dwThreadId = 0;
		HANDLE hThread = CreateThread(NULL, 0, WorkerThread, hIOCP, 0, &dwThreadId);
		if (hThread == NULL)
		{
			printf("CreateThread failed:%d", GetLastError());
			continue;
		}
		CloseHandle(hThread);
	}
	// ����Ϊ������ģʽ�������accept�������������أ���������������accept������ȴ�һ���µ����ӵ���ʱ���Ż᷵�ء�
	SetNonblocking(m_socket);
	while (!IsExit)
	{
		SOCKET cliSock = accept(m_socket, NULL, NULL);
		if (cliSock == SOCKET_ERROR)
		{
			Sleep(10);
			continue;
		}
		printf("Client:%lld connected.\n", cliSock);
		if (CreateIoCompletionPort((HANDLE)cliSock, hIOCP, 0, 0) == NULL)
		{
			printf("Binding Client Socket to IO Completion Port Failed:%u\n", GetLastError());
			closesocket(cliSock);
		}
		else
		{
			IoData* data = (IoData*)malloc(sizeof(IoData));
			if (data == NULL)
			{
				printf("out of memory");
				closesocket(cliSock);
				continue;
			}
			memset(data, 0, sizeof(IoData));
			data->wsabuf.buf = (char*)malloc(1024);
			data->wsabuf.len = 1024;
			data->cliSock = cliSock;
			PostRead(data);
		}
	}
	PostQueuedCompletionStatus(hIOCP, 0, 0, 0);
	closesocket(m_socket);
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
