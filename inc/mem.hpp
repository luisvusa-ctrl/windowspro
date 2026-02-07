#pragma once

// utilitarios de memoria para ler e escrever no processo alvo

#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

// wrapper simples de ReadProcessMemory e WriteProcessMemory
class Memory
{
public:
	DWORD m_PID;
	HANDLE m_hProc;

	// abre handle do processo alvo
	Memory(DWORD PID) : m_PID(PID)
	{
		m_hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_PID);
		if (!m_hProc) {
			std::cout << "[ERROR] Failed to open process handle. Error: " << GetLastError() << std::endl;
		}
	}
	
	// fecha o handle ao destruir
	~Memory()
	{
		if (m_hProc) {
			CloseHandle(m_hProc);
			m_hProc = nullptr;
		}
	}

	// escreve um valor tipado no endereco
	template <typename T>
	bool write(uintptr_t addr, T value)
	{
		if (!m_hProc || !addr) return false;
		return WriteProcessMemory(m_hProc, (LPVOID)addr, &value, sizeof(value), 0);
	}

	// le um valor tipado do endereco
	template <typename T>
	T read(uintptr_t addr)
	{
		T buffer{};
		
		if (!m_hProc || !addr) return buffer;
		
		if (!ReadProcessMemory(m_hProc, (LPCVOID)addr, &buffer, sizeof(buffer), 0)) {
			return T{};
		}
		return buffer;
	}

	// le um bloco de memoria para um buffer
	template <typename T>
	bool readArray(uintptr_t addr, T* buffer, size_t size)
	{
		if (!m_hProc || !addr || !buffer || size == 0) return false;
		SIZE_T bytesRead;

		if (!ReadProcessMemory(m_hProc, (LPCVOID)addr, buffer, size, &bytesRead))
		{
			return false;
		}

		return bytesRead == size;
	}

	// le string zero-terminated do processo
	std::string readstr(uintptr_t addr, size_t maxLength = 256)
	{
		if (!m_hProc || !addr || maxLength == 0) return {};
		std::vector<BYTE> buffer(maxLength);
		SIZE_T bytesRead{};
		if (!ReadProcessMemory(m_hProc, (LPCVOID)addr, buffer.data(), maxLength, &bytesRead) || bytesRead == 0)
		{
			return {};
		}

		buffer.resize(bytesRead);

		auto it{ std::find(buffer.begin(), buffer.end(), 0) };
		size_t length{ (it != buffer.end()) ? static_cast<size_t>(std::distance(buffer.begin(), it)) : buffer.size() };
		std::string result{ reinterpret_cast<char*>(buffer.data()), length };
		
		size_t start{ result.find_first_not_of(" \t\r\n") };
		size_t end{ result.find_last_not_of(" \t\r\n") };
		if (start == std::string::npos)
		{
			return {};
		}

		return result.substr(start, end - start + 1);
	}
};

// retorna o pid pelo nome do executavel
DWORD getPID(std::wstring procName)
{
	HANDLE hSnapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };
	if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

	PROCESSENTRY32W pe32;
	pe32.dwSize = sizeof(pe32);

	if (Process32FirstW(hSnapshot, &pe32))
	{

		do
		{

			if (pe32.szExeFile == procName)
			{
				CloseHandle(hSnapshot);
				return pe32.th32ProcessID;
			}

		} while (Process32NextW(hSnapshot, &pe32));

	}

	CloseHandle(hSnapshot);
	return 0;
}

// retorna base address de um modulo do processo
uintptr_t getModuleBase(DWORD PID, std::wstring moduleName)
{
	HANDLE hSnapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, PID) };
	if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    MODULEENTRY32W me32;
    me32.dwSize = sizeof(me32);

    if (Module32FirstW(hSnapshot, &me32))
    {
        do
        {

            if (moduleName == me32.szModule)
            {
                CloseHandle(hSnapshot);
				return reinterpret_cast<uintptr_t>(me32.modBaseAddr);
            }

        } while (Module32NextW(hSnapshot, &me32));
    }

    CloseHandle(hSnapshot);
    return 0;
}
