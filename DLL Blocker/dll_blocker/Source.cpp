#include <stdio.h>
#include <windows.h>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <target_program_path>\n", argv[0]);
        return 1;
    }

    // Convert the target program path to a wide-character string
    const char* targetPath = argv[1];
    int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, targetPath, -1, NULL, 0);
    wchar_t* wideCharBuffer = new wchar_t[wideCharLen];
    MultiByteToWideChar(CP_UTF8, 0, targetPath, -1, wideCharBuffer, wideCharLen);

    STARTUPINFOEXA si = {};
    PROCESS_INFORMATION pi = {};
    SIZE_T size = 0;

    // Initialize the STARTUPINFOEXA structure
    si.StartupInfo.cb = sizeof(STARTUPINFOEXA);
    si.StartupInfo.dwFlags = EXTENDED_STARTUPINFO_PRESENT;

    // Get the size of the PROC_THREAD_ATTRIBUTE_LIST to be allocated
    InitializeProcThreadAttributeList(NULL, 1, 0, &size);

    // Allocate memory for the PROC_THREAD_ATTRIBUTE_LIST
    si.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(HeapAlloc(
        GetProcessHeap(),
        0,
        size
    ));

    // Initialize the PROC_THREAD_ATTRIBUTE_LIST
    InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &size);

    // Enable blocking of non-Microsoft signed DLLs
    DWORD64 policy = PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_ON;

    // Assign the mitigation policy attribute
    UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_MITIGATION_POLICY, &policy, sizeof(policy), NULL, NULL);

    if (!CreateProcessW(
        NULL,
        wideCharBuffer,
        NULL,
        NULL,
        true,
        EXTENDED_STARTUPINFO_PRESENT,
        NULL,
        NULL,
        reinterpret_cast<LPSTARTUPINFOW>(&si),
        &pi
    ))
    {
        fprintf(stderr, "Failed to create the target process.\n");

        // Clean up allocated resources
        free(wideCharBuffer);
        HeapFree(GetProcessHeap(), 0, si.lpAttributeList);

        return 1;
    }

    // Close the process handles
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    // Clean up allocated resources
    free(wideCharBuffer);
    HeapFree(GetProcessHeap(), 0, si.lpAttributeList);

    return 0;
}
