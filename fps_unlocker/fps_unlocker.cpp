// fps_unlocker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#include "driver_boot.h"

HMODULE m_win32u;

typedef struct OPERMEMORY
{
	DWORD ProcessId;
	PVOID Address;
	PVOID Buffer;
	SIZE_T dwSize;
	DWORD Protect;
	DWORD OldProtect;
	HANDLE hThread;
	DWORD ThreadId;
	wchar_t name[260];
	bool Wow64;
	bool Hig;
	bool bWait;
	BYTE version;
}OPERMEMORY, * LPOPERMEMORY;

enum KERNEL_TYPE : DWORD
{
	KERNEL_GET_VERSION = 0x40000000,
	KERNEL_READ_MEMORY,
	KERNEL_WRITE_MEMORY,
	KERNEL_QUE_PROCESS,
	KERNEL_ALL_MEMORY,
	KERNEL_FREE_MEMORY,
	KERNEL_QUE_MEMORY,
	KERNEL_SET_MEMORY,
	KERNEL_CRE_THREAD,
	KERNEL_SCR_SHOT,
	KERNEL_GET_MODULE,
	KERNEL_GET_PROCESS_BASE,
	KERNEL_VIRUTAlMEMORY,
	KERNEL_CALL_BACK,
	KERNEL_GET_CONTEXT,
	KERNEL_SET_CONTEXT
};

template<typename ... A>
uint64_t call_hook(const A ... arguments)
{
	void* control_function = GetProcAddress(m_win32u, "NtOpenCompositionSurfaceSectionInfo");
	if (control_function)
	{
		const auto control = static_cast<uint64_t(__stdcall*)(A...)>(control_function);
		if (control)
			return control(arguments ...);
	}
	return 10;
}

bool get_version()
{
	OPERMEMORY buffer{};
	bool result{};
	result = call_hook(&buffer, KERNEL_TYPE::KERNEL_GET_VERSION, nullptr, nullptr);
	return result && buffer.version;
}

bool initialize()
{
	m_win32u = LoadLibrary("win32u.dll");

	if (m_win32u == NULL)
	{
		return false;
	}

	if (!get_version())
	{
		return false;
	}

	return true;
}

bool __stdcall write_process_memory(DWORD ProcessId, ULONG64 Address, PVOID Buffer, ULONG64 dwSize)
{
	OPERMEMORY mm{};
	mm.ProcessId = ProcessId;
	mm.Address = reinterpret_cast<PVOID64>(Address);
	mm.Buffer = Buffer;
	mm.dwSize = dwSize;
	//SetVirtualMemory(Address, dwSize);
	return call_hook(&mm, KERNEL_TYPE::KERNEL_WRITE_MEMORY, nullptr, nullptr);
}

// 1000/16 ≈ 60
int deltaTime = 16;

int main()
{
	char szPath[MAX_PATH] = {0};

	GetModuleFileName(NULL, szPath, sizeof(szPath));

	auto remain = strrchr(szPath, '\\');

	memcpy(remain, "\\dxgkrnl_hook.sys", sizeof("\\dxgkrnl_hook.sys"));

	// "dxgkrnl_hook.sys"
	printf("[+] 安装驱动服务 %d\n", StartDriverService("C:\\Users\\yafen\\source\\repos\\fps_unlocker\\x64\\Debug\\dxgkrnl_hook.sys"));

	printf("[+] 正在搜索三国进程...\n");


	auto nums = 0;
	while (nums == 0)
	{
		PROCESSENTRY32 entry = { 0 };

		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshots = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (Process32First(snapshots, &entry) == TRUE)
		{
			while (Process32Next(snapshots, &entry) == TRUE)
			{
				if (strcmp(entry.szExeFile, "QQSG.exe") == 0)
				{
					printf("PID: %d \n", entry.th32ProcessID);
					nums++;
				}
			}
		}

		printf("共搜索到 %d 个进程 \n", nums);

		// research 2s later...
		if (nums == 0)
			Sleep(2000);
	}

	printf("请输入三国的进程ID ：\n");

	int dwPid = 0;

	while (scanf_s("%d", &dwPid) == EOF || dwPid == 0)
	{
		printf("[+] invalid pid \n");
	}

	int fps = 30;

	printf("请输入帧数 ：\n");

	while (scanf_s("%d", &fps) == EOF || fps < 30)
	{
		printf("[+] invalid fps \n");
	}

	deltaTime = 1000 / fps;

	// set qqsg main loop tick delta time.
	printf("设置帧数 %d \n", write_process_memory(dwPid, 0x007CA27E, &deltaTime, sizeof(int)));
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
