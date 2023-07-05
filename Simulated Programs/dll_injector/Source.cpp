#include <Windows.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

    DWORD       TID = NULL;
    DWORD       PID = NULL;
    LPVOID      rBuffer = NULL;
    HANDLE      hProcess = NULL;
    HANDLE      hThread = NULL;
    HMODULE     hKernel32 = NULL;
    wchar_t     dllPath[MAX_PATH];
    size_t      pathSize = sizeof(dllPath);
    size_t      bytesWritten = 0;

    if (argc < 3) {
        printf("usage: %s <PID> <DLL Path>\n", argv[0]);
        return 1;
    }

    PID = atoi(argv[1]);
    int convertedChars = MultiByteToWideChar(CP_UTF8, 0, argv[2], -1, NULL, 0);
    if (convertedChars == 0) {
        printf("failed to convert DLL path to wide character string, error: 0x%lx\n", GetLastError());
        return EXIT_FAILURE;
    }

    if (convertedChars > MAX_PATH) {
        printf("DLL path is too long");
        return EXIT_FAILURE;
    }

    if (MultiByteToWideChar(CP_UTF8, 0, argv[2], -1, dllPath, MAX_PATH) == 0) {
        printf("failed to convert DLL path to wide character string, error: 0x%lx\n", GetLastError());
        return EXIT_FAILURE;
    }
    printf("trying to get a handle to the process (%ld)\n", PID);
    hProcess = OpenProcess((PROCESS_VM_OPERATION | PROCESS_VM_WRITE), FALSE, PID);

    if (hProcess == NULL) {
        printf("unable to get a handle to the process (%ld), error: 0x%lx\n", PID, GetLastError());
        return 1;
    }
    printf("trying to get a handle to Kernel32.dll\n");
    hKernel32 = GetModuleHandleW(L"kernel32");

    if (hKernel32 == NULL) {
        printf("failed to get a handle to Kernel32.dll, error: 0x%lx\n", GetLastError());
        return 1;
    }

    LPTHREAD_START_ROUTINE loadLib = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
    rBuffer = VirtualAllocEx(hProcess, NULL, pathSize, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);

    if (rBuffer == NULL) {
        printf("couldn't allocate a buffer to the target process memory, error: 0x%lx\n", GetLastError());
        return 1;
    }
    WriteProcessMemory(hProcess, rBuffer, dllPath, pathSize, &bytesWritten);
    hThread = CreateRemoteThread(hProcess, NULL, 0, loadLib, rBuffer, 0, &TID);

    if (hThread == NULL) {
        printf("unable to create thread, error: 0x%lx\n", GetLastError());
        return 1;
    }

    printf("wrote %zu bytes to new process with thread ID: %ld\n", bytesWritten, TID);
    WaitForSingleObject(hThread, INFINITE);
    if (hThread) {
        CloseHandle(hThread);
    }
    if (hProcess) {
        CloseHandle(hProcess);
    }
    printf("Closed appropriate handles\n");

    return 0;
}
