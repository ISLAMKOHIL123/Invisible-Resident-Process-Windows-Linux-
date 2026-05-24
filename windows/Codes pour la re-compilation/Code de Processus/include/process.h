#ifndef PROCESS_H
#define PROCESS_H

#include <windows.h>

// Structure pour stocker les informations du PCB
typedef struct _PCB_Info {
    DWORD pid, ppid;
    HANDLE hProcess;

    char name[MAX_PATH];
    char path[MAX_PATH];

    DWORD priority;

    FILETIME creation_time;
    FILETIME kernel_time;
    FILETIME user_time;

    SIZE_T working_set;
    SIZE_T peak_working_set;
    SIZE_T private_bytes;
    SIZE_T pagefile_usage;

    ULONG_PTR read_bytes;
    ULONG_PTR write_bytes;
    ULONG_PTR read_ops;
    ULONG_PTR write_ops;
    ULONG_PTR other_ops;

    DWORD thread_count;
    DWORD handle_count;
    DWORD exit_code;
    DWORD session_id;

    BOOL is_wow64;

} PCB_Info;


// Fonction pour créer un nouveau processus
DWORD create_process(PROCESS_INFORMATION *pi);

// Fonction pour afficher les attributs du PCB
void display_pcb(DWORD pid);

// Fonction pour récupérer les informations du PCB
PCB_Info get_pcb_info(DWORD pid);

// Fonction pour rendre le processus résident
int make_resident(const char *program_path);

DWORD create_hidden_process(PROCESS_INFORMATION *pi);
int unlink_from_psapi(DWORD pid);

#endif // PROCESS_H