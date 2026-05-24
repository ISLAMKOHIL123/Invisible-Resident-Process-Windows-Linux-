Pour executer le code :
1-Entrer dans le dossier "Exécutables".
2-Lancer program.exe (il s'affiche dans le gestionnaire des tâches).
3-Lancer charger.bat et entrer le nom de processus que on a besoin de masquier (ici: notepad).
4-La processus et bien masqui.


------------------------Codes pour la re-compilation-----------------------

 Driver_KernelMode/
   - Code source du driver (Driver.c)
   - Script de compilation (build.bat)
   - Apres la compilation de Driver.c a partir de script build.bat : ProcessHider.sys (Dans le dossier bin)


Scripts_Installation/
   - charger.bat : Charge le driver (utilisé pour masqui la processus).




