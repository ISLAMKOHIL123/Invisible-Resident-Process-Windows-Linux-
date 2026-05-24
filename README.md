# Invisible Resident Process – Windows & Linux

A kernel-level project implementing a hidden, persistent process on both Windows and Linux.
Built for educational purposes in a controlled test environment.

## What it does

- Creates a process and reads its PCB (Process Control Block) attributes
- Hides the process from Task Manager / `ps` / `top`
- Survives reboots via persistence mechanisms
- Dual implementation: Windows (kernel driver) + Linux (kernel module)

## How it works

### Windows
- **Hiding**: DKOM (Direct Kernel Object Manipulation) — removes the process from the `ActiveProcessLinks` linked list inside the `EPROCESS` kernel structure
- **PCB access**: Win32 API (`OpenProcess`, `GetProcessMemoryInfo`, `GetProcessTimes`...)
- **Persistence**: Registry key `HKCU\Run` + scheduled task via `schtasks`
- **Driver**: `ProcessHider.sys` loaded in Test Signing Mode

### Linux
- **Hiding**: Ftrace syscall hooking — intercepts `getdents64` and filters the target PID from the output of `ls`, `ps`, `top`, and `/proc`
- **PCB access**: `task_struct` fields (PID, PPID, UID, nice value, scheduler priority)
- **Persistence**: `systemd` service (auto-restart) + module loaded via `/etc/modules`
- **Daemon**: `fork()`-based resident process with elevated priority via `setpriority()`

## Test Environment

| | Windows | Linux |
|---|---|---|
| OS | Windows 10 x64 (Build 19041) | Ubuntu (VirtualBox) |
| Tools | Visual Studio 2022, WDK 10.0.26100 | gcc, make, Ftrace |
| Mode | Test Signing Mode | Root |

## Results

- Process hidden from Task Manager / `top` / `ps` ✓
- PCB attributes displayed successfully ✓
- Auto-restart after reboot confirmed ✓
- No system crashes ✓

## ⚠️ Disclaimer

This project is strictly for **educational purposes** as part of an Operating Systems course
at USTHB – Faculté d'Informatique. Do not use on production systems.

## Authors

Kohil Islam · 
Fegas Lokman Abdelhakim · 
Hamek Nouh · Haloui Moussa · 
Cheriet Mahieddine Idris · 

*USTHB – Faculté d'Informatique, Novembre 2025*
