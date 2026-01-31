#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>

class Memory
{
private:
	DWORD m_PID;
	HANDLE m_hProc;

public:

	Memory(DWORD PID) : m_PID(PID)
	{
		m_hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_PID);
	};

	template <typename T>
	bool write(uintptr_t addr, T value)
	{
		return WriteProcessMemory(m_hProc, (LPVOID)addr, &value, sizeof(value), 0);
	}

	template <typename T>
	T read(uintptr_t addr)
	{
		T buffer{};

		ReadProcessMemory(m_hProc, (LPCVOID)addr, &buffer, sizeof(buffer), 0);
		return buffer;
	}

	template <typename T>
	bool readArray(uintptr_t addr, T* buffer, size_t size)
	{
		SIZE_T bytesRead;

		if (!ReadProcessMemory(m_hProc, (LPCVOID)addr, buffer, size, &bytesRead))
		{
			return false;
		}

		return bytesRead == size;
	}

	std::string readstr(uintptr_t addr, size_t maxLength = 256)
	{
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

DWORD getPID(std::wstring procName)
{
	HANDLE hSnapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);

	if (Process32First(hSnapshot, &pe32))
	{

		do
		{

			if (pe32.szExeFile == procName)
			{
				CloseHandle(hSnapshot);
				return pe32.th32ProcessID;
			}

		} while (Process32Next(hSnapshot, &pe32));

	}

	return 0;
}

uintptr_t getModuleBase(DWORD PID, std::wstring moduleName)
{
	HANDLE hSnapshot{ CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, PID) };

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(me32);

    if (Module32First(hSnapshot, &me32))
    {
        do
        {

            if (moduleName == me32.szModule)
            {
                CloseHandle(hSnapshot);
				return reinterpret_cast<uintptr_t>(me32.modBaseAddr);
            }

        } while (Module32Next(hSnapshot, &me32));
    }

    CloseHandle(hSnapshot);
    return 0;
}