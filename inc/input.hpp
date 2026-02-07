#pragma once

#include <Windows.h>

namespace NtUserInjectMouseInput
{
    // estrutura usada pela syscall de injecao de mouse
    typedef struct _INJECTED_INPUT_MOUSE_INFO
    {
        int PixelDeltaX;
        int PixelDeltaY;
        UINT32 MouseData;
        UINT32 MouseOptions;
        UINT32 TimeOffsetInMilliseconds;
        ULONG_PTR ExtraInfo;
    } INJECTED_INPUT_MOUSE_INFO, * PINJECTED_INPUT_MOUSE_INFO;

    // flags de movimento e clique para injecao
    enum InjectedInputMouseOptions : UINT32
    {
        IIMO_None = 0x0000,
        IIMO_Move = 0x0001,
        IIMO_LeftDown = 0x0002,
        IIMO_LeftUp = 0x0004,
        IIMO_RightDown = 0x0008,
        IIMO_RightUp = 0x0010,
        IIMO_MiddleDown = 0x0020,
        IIMO_MiddleUp = 0x0040,
        IIMO_XDown = 0x0080,
        IIMO_XUp = 0x0100,
        IIMO_Wheel = 0x0800,
        IIMO_HWheel = 0x1000,
        IIMO_MoveNoCoalesce = 0x2000,
        IIMO_VirtualDesk = 0x4000,
        IIMO_Absolute = 0x8000
    };

    // ponteiro de funcao para InjectMouseInput do user32
    typedef BOOL(WINAPI* PFN_InjectMouseInput)(_In_reads_(count) const INJECTED_INPUT_MOUSE_INFO* inputs, _In_ int count);

    static PFN_InjectMouseInput g_pInjectMouseInput{ nullptr };
    BYTE _sysCallBytes[30];
    bool IsLoaded{ false };

    // resolve InjectMouseInput do user32
    void InitInjectMouseInput()
    {
        if (g_pInjectMouseInput != nullptr) return;

        HMODULE hUser32{ GetModuleHandleW(L"user32.dll") };
        if (!hUser32) return;

        FARPROC p{ GetProcAddress(hUser32, "InjectMouseInput") };
        if (p) g_pInjectMouseInput = reinterpret_cast<PFN_InjectMouseInput>(p);
    }

    // carrega win32u e captura bytes da syscall
    bool Load()
    {
        LoadLibraryW(L"user32.dll");
        HMODULE win32u{ LoadLibraryW(L"win32u.dll") };
        if (!win32u) return false;

        void* addr{ (void*)GetProcAddress(win32u, "NtUserInjectMouseInput") };
        if (!addr) return false;

        memcpy(_sysCallBytes, addr, 30);
        return true;
    }

    // executa a syscall via stub alocado dinamicamente
    bool WINAPI NTInjectMouseInput(INJECTED_INPUT_MOUSE_INFO* info, int val)
    {
        if (!IsLoaded) IsLoaded = Load();

        auto IMIS{ VirtualAlloc(0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE) };
        if (!IMIS) return FALSE;

        memcpy(IMIS, _sysCallBytes, 30);
        NTSTATUS status{ reinterpret_cast<NTSTATUS(NTAPI*)(INJECTED_INPUT_MOUSE_INFO*, int)>(IMIS)(info, val) };

        SecureZeroMemory(IMIS, 0x1000);
        VirtualFree(IMIS, 0, MEM_RELEASE);

        return (status > 0);
    }

    // injeta movimento relativo usando a syscall
    bool NTInjectInput(int x, int y)
    {
        INJECTED_INPUT_MOUSE_INFO items[1]{};

        items[0].PixelDeltaX = x;
        items[0].PixelDeltaY = y;
        items[0].MouseOptions = IIMO_Move;
        items[0].ExtraInfo = (ULONG_PTR)GetMessageExtraInfo();

        bool ntResult{ NTInjectMouseInput(items, 1) };
        return ntResult;
    }

    // injeta movimento relativo usando a api do user32 quando disponivel
    bool InjectMouseMoveRelative(int dx, int dy, ULONG_PTR extraInfo = 0)
    {
        if (g_pInjectMouseInput)
        {
            INJECTED_INPUT_MOUSE_INFO entry = {};
            entry.PixelDeltaX = dx;
            entry.PixelDeltaY = dy;
            entry.MouseData = 0;
            entry.MouseOptions = IIMO_Move;
            entry.TimeOffsetInMilliseconds = 0;
            entry.ExtraInfo = extraInfo ? extraInfo : (ULONG_PTR)GetMessageExtraInfo();
            return g_pInjectMouseInput(&entry, 1) == TRUE;
        }
    }
}
