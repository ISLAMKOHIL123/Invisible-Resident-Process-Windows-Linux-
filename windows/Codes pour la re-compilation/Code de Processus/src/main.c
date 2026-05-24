#include <stdio.h>
#include <windows.h>
#include "../include/process.h"

int main(int argc, char *argv[]) {
    printf("=== PROCESSUS MASQUE - WINDOWS ===\n");
    
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    
    // Créer un nouveau processus
    printf("1. Creation du processus masque...\n");
    DWORD child_pid = create_process(&pi);
    
    if (child_pid > 0) {
        
        // Afficher les informations PCB
        display_pcb(child_pid);
        
        // Configuration résidence
        printf("\n2. Configuration residence...\n");
        make_resident(argv[0]);
        
        // Activer le processus
        if (pi.hThread) {
            ResumeThread(pi.hThread);
            printf("Processus active\n");
        }
        
        // Nettoyage
        if (pi.hProcess) CloseHandle(pi.hProcess);
        if (pi.hThread) CloseHandle(pi.hThread);
        
        printf("\n=== OPERATION TERMINEE ===\n");
        printf("Processus: %lu\n", child_pid);
        printf("Redemarrage: Configure pour auto-demarrage\n");
        
    } else {
        printf("ERREUR: Creation processus echouee\n");
    }
    
    // Vérification
    printf("\nVerification dans Task Manager...\n");
    printf("Ouvrez Task Manager et cherchez le PID: %lu\n", child_pid);
    
    printf("\nAppuyez sur Entree pour quitter...");
    getchar();
    
    return 0;
}