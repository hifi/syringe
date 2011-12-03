/*
 * Copyright (c) 2011 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    PROCESS_INFORMATION pInfo;
    STARTUPINFOA sInfo;
    char *str_syringe_dll = "syringe.dll";

    ZeroMemory(&sInfo, sizeof(STARTUPINFO));
    ZeroMemory(&pInfo, sizeof(PROCESS_INFORMATION));
    sInfo.cb = sizeof(sInfo);

    if (argc < 2)
    {
        MessageBoxA(NULL, "usage: syringe.exe application.exe", "syringe", MB_OK|MB_ICONSTOP);
        return 1;
    }

    if (CreateProcessA(argv[1], NULL, 0, 0, FALSE, CREATE_SUSPENDED, 0, 0, &sInfo, &pInfo))
    {
        DWORD dwThread = 0;

        printf("syringe: Host program loaded, injecting %s\n", str_syringe_dll);

        HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE, TRUE, pInfo.dwProcessId);

        LPVOID ptr_syringe_dll = VirtualAllocEx(hProcess, NULL, strlen(str_syringe_dll)+1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        WriteProcessMemory(hProcess, ptr_syringe_dll, str_syringe_dll, strlen(str_syringe_dll)+1, NULL);

        DWORD ptr_LoadLibraryA = (DWORD)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

        CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)ptr_LoadLibraryA, ptr_syringe_dll, 0, &dwThread);

        printf("syringe: Resuming host program\n");
        ResumeThread(pInfo.hThread);

        WaitForSingleObject(pInfo.hThread, INFINITE);
    }
    else
    {
        MessageBoxA(NULL, "Failed to launch given executable.", "syringe", MB_OK|MB_ICONSTOP);
        return 1;
    }

    return 0;
}
