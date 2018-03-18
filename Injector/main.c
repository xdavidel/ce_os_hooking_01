#include <stdio.h>
#include <Windows.h>

#define LOAD_LIBRARY_NAME "LoadLibraryA"

STARTUPINFO CreateStartup()
{
	STARTUPINFO startInfo;
	startInfo.cb = 0;
	startInfo.lpReserved = NULL;
	startInfo.lpDesktop = NULL;
	startInfo.lpTitle = NULL;
	startInfo.dwX = 0;
	startInfo.dwY = 0;
	startInfo.dwXSize = 100;
	startInfo.dwYSize = 100;
	startInfo.dwXCountChars = 0;
	startInfo.dwYCountChars = 0;
	startInfo.dwFillAttribute = 0;
	startInfo.dwFlags = 0;
	startInfo.wShowWindow = 0;
	startInfo.cbReserved2 = 0;
	startInfo.lpReserved2 = NULL;
	startInfo.hStdInput = NULL;
	startInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	startInfo.hStdError = NULL;
	return startInfo;
}

int handleError()
{
	DWORD error = GetLastError();
	printf("Error (%d), Injection Failed...", error);
	return error;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		printf("Program must have <Target Process> and <DLL> provided");
		return 0;
	}

	char* targetProcessName = argv[1];
	char* dllName = argv[2];

	STARTUPINFO startInfo = CreateStartup();
	PROCESS_INFORMATION processesInfo;

	if (0 == CreateProcess(
		NULL,						// app name
		targetProcessName,			// cmd parameters
		NULL,						// process security
		NULL,						// thread security
		FALSE,						// inheritance
		CREATE_SUSPENDED,			// creation flags
		NULL,						// env
		NULL,						// current dir
		&startInfo,					// startup info
		&processesInfo				// procces info
	))
	{
		return handleError();
	}

	HANDLE kernel32 = GetModuleHandle("Kernel32.dll");
	if (NULL == kernel32) 
	{
		return handleError();
	}

	LPVOID LoadLibraryAtRemoteProcess =
		(LPVOID)GetProcAddress(kernel32, (LPCSTR)LOAD_LIBRARY_NAME);

	LPVOID dllPathLocalAddress = (LPVOID)dllName;
	SIZE_T dllPathLocalSize = strlen(dllPathLocalAddress);
	LPVOID dllPathRemoteAddress = (LPVOID)VirtualAllocEx(
		processesInfo.hProcess,								// process handle
		NULL,												// lpAddress
		dllPathLocalSize,									// size
		MEM_RESERVE | MEM_COMMIT,							// allocation type
		PAGE_READWRITE										// protection
	);
	if (0 == WriteProcessMemory(
		processesInfo.hProcess,								// process handle
		dllPathRemoteAddress,								// target address
		dllPathLocalAddress,								// source buffer
		dllPathLocalSize,									// size
		NULL												// num of bytes written
	)) 
	{
		return handleError();
	}

	if (NULL == CreateRemoteThread(
		processesInfo.hProcess,								// process handle
		NULL,												// thread security
		0,													// stack size
		(LPTHREAD_START_ROUTINE)LoadLibraryAtRemoteProcess, // start address
		dllPathRemoteAddress,								// parameters
		0,													// creation flags
		NULL												// thread id
	))
	{
		return handleError();
	}

	ResumeThread(
		processesInfo.hThread
	);


	return 0;
}