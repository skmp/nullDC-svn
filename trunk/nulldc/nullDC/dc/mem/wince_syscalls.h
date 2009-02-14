//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

ppsyscall.h

Abstract:

Private portion of syscall.h

Notes:


--*/

#ifndef _PRIV_SYSCALL_H__
#define _PRIV_SYSCALL_H__

#define FIRST_METHOD            0xFFFFFE01
#define APICALL_SCALE           2

#define SYSCALL_RETURN          (FIRST_METHOD-APICALL_SCALE)

#define NUM_API_SETS            128             // must be 2^n

#define APISET_SHIFT            8
#define METHOD_MASK             0x00FF
#define METHOD_COUNT (METHOD_MASK+1)
#define APISET_MASK             (NUM_API_SETS-1)

//
// all the API encoding falls between the following range
//
//  (LAST_METHOD <= api_encoding <= MAX_METHOD_NUM)
//
#define MAX_METHOD_NUM          (FIRST_METHOD + (METHOD_MASK * APICALL_SCALE))
#define LAST_METHOD             (FIRST_METHOD - ((APISET_MASK)<<APISET_SHIFT | (METHOD_MASK))*APICALL_SCALE)

#define REGAPISET_TYPEONLY      0x80000000

#define SH_WIN32        0
#define SH_CURTHREAD    1
#define SH_CURPROC      2
#define SH_CURTOKEN     3
#define HT_EVENT        4 // Event handle type
#define HT_MUTEX        5 // Mutex handle type
#define HT_APISET       6 // kernel API set handle type
#define HT_FILE         7 // open file handle type
#define HT_FIND         8 // FindFirst handle type
#define HT_DBFILE       9 // open database handle type
#define HT_DBFIND       10 // database find handle type
#define HT_SOCKET       11 // WinSock open socket handle type
#define HT_CRITSEC      12 // Critical section
#define HT_SEMAPHORE    13 // Semaphore handle type
#define HT_FSMAP        14 // mapped files
#define HT_WNETENUM     15 // Net Resource Enumeration
#define HT_AFSVOLUME    16 // file system volume handle type
#define SH_GDI          80
#define SH_WMGR         81
#define SH_WNET         82 // WNet APIs for network redirector
#define SH_COMM         83 // Communications not COM
#define SH_FILESYS_APIS 84 // File system APIS
#define SH_SHELL        85
#define SH_DEVMGR_APIS  86 // File system device manager
#define SH_TAPI         87
#define SH_CPROG        88 // Handle to the specified API set
#define SH_SERVICES     90
#define SH_DDRAW        91
#define SH_D3DM         92


#define W32_HandleCall                  0
#define W32_CreateAPISet				2
#define W32_CreateProcess               3
#define W32_OpenProcess                 4
#define W32_CreateThread                5
#define W32_OpenThread                  6
#define W32_CreateEvent                 7
#define W32_OpenEvent                   8
#define W32_CreateMutex                 9
#define W32_OpenMutex                   10
#define W32_CreateSemaphore             11
#define W32_OpenSemaphore               12
#define W32_NKExitThread                13
#define W32_TlsCall                     14
#define W32_IsBadPtr                    15
#define W32_CeVirtualSharedAlloc        16
#define W32_CRITCreate                  17
#define W32_Sleep                       18
#define W32_SetLastError                19
#define W32_GetLastError                20
#define W32_LoadLibraryEx               21
#define W32_FreeLibrary                 22
#define W32_CreateWatchDog              23
#define W32_CeCreateToken               24
#define W32_DuplicateHandle             25
#define W32_WaitForMultiple             26
#define W32_OutputDebugString           27
#define W32_NKvDbgPrintfW               28
#define W32_RaiseException              29
#define W32_PerformCallBack             30
#define W32_GetTickCount                31
#define W32_GetProcessVersion           32     // GetProcessVersion
#define W32_GetRomFileBytes             33
#define W32_CeRevertToSelf              34
#define W32_CeImpersonateCurrProc       35
#define W32_GetRomFileInfo              36
#define W32_GetSystemInfo               37
#define W32_GetFSHeapInfo               38
#define W32_KernelIoControl             39
#define W32_KernelLibIoControl          40
#define W32_NKGetTime                   41
#define W32_GetDirectCallerProcessId    42
#define W32_GetOwnerProcessId           43
#define W32_GetIdleTime                 44
#define W32_CacheRangeFlush             45
#define W32_LoadExistingModule          46  // u-mode only
#define W32_GetCallerResInfo            46  // k-mode only
#define W32_WaitForDebugEvent           47
#define W32_ContinueDebugEvent          48
#define W32_DebugNotify                 49
#define W32_AttachDebugger              50
#define W32_BinaryCompress              51
#define W32_BinaryDecompress            52
#define W32_DecompressBinaryBlock       53
#define W32_StringCompress              54
#define W32_StringDecompress            55
#define W32_CeGetRandomSeed             56
#define W32_SleepTillTick               57
#define W32_QueryPerformanceFrequency   58
#define W32_QueryPerformanceCounter     59
#define W32_GetThreadTimes              60
#define W32_QueryAPISetID               61
#define W32_IsNamedEventSignaled        62
#define W32_CreateLocaleView            63
#define W32_RegisterDbgZones            64
#define W32_SetDbgZone                  65
#define W32_CreateMsgQueue              66
#define W32_WriteDebugLED               67
#define W32_IsProcessorFeaturePresent   68
#define W32_QueryInstructionSet         69
#define W32_SetKernelAlarm              70
#define W32_RefreshKernelAlarm          71
#define W32_SetOOMEvent                 72
#define W32_NKSetTime                   73
#define W32_GetKPhys                    74
#define W32_GiveKPhys                   75
#define W32_InputDebugCharW             76
#define W32_SetLowestScheduledPriority  77
#define W32_UpdateNLSInfoEx             78
#define W32_ReadRegistryFromOEM         79
#define W32_WriteRegistryToOEM          80
#define W32_GetStdioPathW               81
#define W32_SetStdioPathW               82
#define W32_SetHardwareWatch            83
#define W32_PrepareThreadExit           84
#define W32_InterruptInitialize         85
#define W32_InterruptDisable            86
#define W32_InterruptDone               87
#define W32_InterruptMask               88
#define W32_SetPowerHandler             89
#define W32_PowerOffSystem              90
#define W32_LoadIntChainHandler         91
#define W32_FreeIntChainHandler         92
#define W32_GetExitCodeProcess          93
#define W32_GetExitCodeThread           94
#define W32_CreateStaticMapping         95
#define W32_SetDaylightTime             96
#define W32_SetTimeZoneBias             97
#define W32_THCreateSnapshot            98
#define W32_RemoteLocalAlloc            99
#define W32_RemoteLocalReAlloc          100
#define W32_RemoteLocalSize             101
#define W32_RemoteLocalFree             102
#define W32_PSLNotify                   103
#define W32_SystemMemoryLow             104
#define W32_LoadKernelLibrary           105
#define W32_CeLogData                   106
#define W32_CeLogSetZones               107
#define W32_CeLogGetZones               108
#define W32_CeLogReSync                 109
#define W32_WaitForAPIReady             110
#define W32_SetRAMMode                  111
#define W32_SetStoreQueueBase           112
#define W32_GetHandleServerId           113
#define W32_CeAccessCheck               114
#define W32_CePrivilegeCheck            115
#define W32_SetDirectCall               116
#define W32_GetCallerVMProcessId        117
#define W32_ProfileSyscall              118
#define W32_DebugSetProcessKillOnExit   119
#define W32_DebugActiveProcessStop      120
#define W32_DebugActiveProcess          121
#define W32_GetCallerProcess            122     // old behavior GetCallerProcess
#define W32_ReadKPEHeader               123
#define W32_ForcePageout                124
#define W32_DeleteStaticMapping         125
#define W32_CeGetRawTime                126     // CeGetRawTime
#define W32_FileTimeToSystemTime        127     // FileTimeToSystemTime
#define W32_SystemTimeToFileTime        128     // SystemTimeToFileTime
#define W32_NKGetTimeAsFileTime         129     // NKGetTimeAsFileTime

const char* W32_mids[METHOD_COUNT];

const char* ID_PROC_mids [METHOD_COUNT];
const char* ID_THREAD_mids [METHOD_COUNT];
const char* ID_TOKEN_mids [METHOD_COUNT];
const char* ID_EVENT_mids [METHOD_COUNT];
const char* ID_APISET_mids [METHOD_COUNT];
const char* ID_FSMAP_mids [METHOD_COUNT];
const char* ID_MUTEX_mids [METHOD_COUNT];
const char* ID_SEMAPHORE_mids [METHOD_COUNT];


/* Common syscalls */

#define ID_HCALLBASE             2

#define ID_HCALL(i)                 (i+ID_HCALLBASE)

/* Process syscalls */

#define ID_PROC_TERMINATE           ID_HCALL(0)
#define ID_PROC_VMSETATTR           ID_HCALL(1)
#define ID_PROC_FLUSHICACHE   ID_HCALL(2)
#define ID_PROC_READMEMORY   ID_HCALL(3)
#define ID_PROC_WRITEMEMORY   ID_HCALL(4)
// ID_HCALL(5) unused
#define ID_PROC_GETMODINFO          ID_HCALL(6)
#define ID_PROC_SETVER              ID_HCALL(7)
#define ID_PROC_VMALLOC             ID_HCALL(8)
#define ID_PROC_VMFREE              ID_HCALL(9)
#define ID_PROC_VMQUERY             ID_HCALL(10)
#define ID_PROC_VMPROTECT           ID_HCALL(11)
#define ID_PROC_VMCOPY              ID_HCALL(12)
#define ID_PROC_VMLOCK              ID_HCALL(13)
#define ID_PROC_VMUNLOCK            ID_HCALL(14)
#define ID_PROC_HNDLCLOSE           ID_HCALL(15)
#define ID_PROC_UNMAPVIEW           ID_HCALL(16)
#define ID_PROC_FLUSHVIEW           ID_HCALL(17)
#define ID_PROC_MAPVIEW             ID_HCALL(18)
#define ID_PROC_MAPCREATE           ID_HCALL(19)
// ID_HCALL(20) unused
#define ID_PROC_GETMODNAME          ID_HCALL(21)
#define ID_PROC_GETMODHNDL          ID_HCALL(22)
#define ID_PROC_VMALLOCPHYS         ID_HCALL(23)
#define ID_PROC_READPEHEADER        ID_HCALL(24)
#define ID_PROC_CHECKREMOTEDEBUGGERPRESENT ID_HCALL(25)
#define ID_PROC_OPENMSGQEX          ID_HCALL(26)
#define ID_PROC_GETPROCID           ID_HCALL(27)
#define ID_PROC_VMALLOCCOPY         ID_HCALL(28)
#define ID_PROC_PAGEOUTMODULE       ID_HCALL(29)

/* Thread syscalls */

#define ID_THREAD_SUSPEND           ID_HCALL(0)
#define ID_THREAD_RESUME            ID_HCALL(1)
#define ID_THREAD_SETTHREADPRIO     ID_HCALL(2)
#define ID_THREAD_GETTHREADPRIO     ID_HCALL(3)
#define ID_THREAD_GETCALLSTACK      ID_HCALL(4)
#define ID_THREAD_GETCONTEXT  ID_HCALL(5)
#define ID_THREAD_SETCONTEXT  ID_HCALL(6)
#define ID_THREAD_TERMINATE   ID_HCALL(7)
#define ID_THREAD_CEGETPRIO   ID_HCALL(8)
#define ID_THREAD_CESETPRIO   ID_HCALL(9)
#define ID_THREAD_CEGETQUANT  ID_HCALL(10)
#define ID_THREAD_CESETQUANT  ID_HCALL(11)
#define ID_THREAD_GETID             ID_HCALL(12)
#define ID_THREAD_GETPROCID         ID_HCALL(13)

/* Token syscalls */
#define ID_TOKEN_IMPERSONATE        ID_HCALL(0)
#define ID_TOKEN_GETOWNERSID        ID_HCALL(1)
#define ID_TOKEN_GETGROUPSID        ID_HCALL(2)

/* Event syscalls */

#define ID_EVENT_MODIFY             ID_HCALL(0)
#define ID_EVENT_GETDATA            ID_HCALL(1)
#define ID_EVENT_SETDATA            ID_HCALL(2)
#define ID_EVENT_READMSGQ           ID_HCALL(3)
#define ID_EVENT_WRITEMSGQ          ID_HCALL(4)
#define ID_EVENT_GETMSGQINFO        ID_HCALL(5)
#define ID_EVENT_WDSTART            ID_HCALL(6)
#define ID_EVENT_WDSTOP             ID_HCALL(7)
#define ID_EVENT_WDREFRESH          ID_HCALL(8)
#define ID_EVENT_RESUMEMAINTH       ID_HCALL(9)

/* Apiset syscalls */

#define ID_APISET_REGISTER            ID_HCALL(0)
#define ID_APISET_CREATEHANDLE        ID_HCALL(1)
#define ID_APISET_VERIFY              ID_HCALL(2)
#define ID_APISET_REGDIRECTMTHD       ID_HCALL(3)
#define ID_APISET_LOCKAPIHANDLE       ID_HCALL(4)
#define ID_APISET_UNLOCKAPIHANDLE     ID_HCALL(5)
#define ID_APISET_SETAPIERRORHANDLER  ID_HCALL(6)

/* mapped file syscalls */

#define ID_FSMAP_MAPVIEWOFFILE  ID_HCALL(0)

/* mutex syscalls */

#define ID_MUTEX_RELEASEMUTEX  ID_HCALL(0)

/* semaphore syscalls */

#define ID_SEMAPHORE_RELEASESEMAPHORE ID_HCALL(0)

#endif // _PRIV_SYSCALL_H__
struct
{
	const char* name;
	const char** mids;//const char** 
} hid_table[NUM_API_SETS];

/*
*/
void init_hid_table()
{
	W32_mids [W32_HandleCall]="HandleCall";	//                  0
	W32_mids [W32_CreateAPISet]="CreateAPISet";	//				2
	W32_mids [W32_CreateProcess]="CreateProcess";	//               3
	W32_mids [W32_OpenProcess]="OpenProcess";	//                 4
	W32_mids [W32_CreateThread]="CreateThread";	//                5
	W32_mids [W32_OpenThread]="OpenThread";	//                  6
	W32_mids [W32_CreateEvent]="CreateEvent";	//                 7
	W32_mids [W32_OpenEvent]="OpenEvent";	//                   8
	W32_mids [W32_CreateMutex]="CreateMutex";	//                 9
	W32_mids [W32_OpenMutex]="OpenMutex";	//                   10
	W32_mids [W32_CreateSemaphore]="CreateSemaphore";	//             11
	W32_mids [W32_OpenSemaphore]="OpenSemaphore";	//               12
	W32_mids [W32_NKExitThread]="NKExitThread";	//                13
	W32_mids [W32_TlsCall]="TlsCall";	//                     14
	W32_mids [W32_IsBadPtr]="IsBadPtr";	//                    15
	W32_mids [W32_CeVirtualSharedAlloc]="CeVirtualSharedAlloc";	//        16
	W32_mids [W32_CRITCreate]="CRITCreate";	//                  17
	W32_mids [W32_Sleep]="Sleep";	//                       18
	W32_mids [W32_SetLastError]="SetLastError";	//                19
	W32_mids [W32_GetLastError]="GetLastError";	//                20
	W32_mids [W32_LoadLibraryEx]="LoadLibraryEx";	//               21
	W32_mids [W32_FreeLibrary]="FreeLibrary";	//                 22
	W32_mids [W32_CreateWatchDog]="CreateWatchDog";	//              23
	W32_mids [W32_CeCreateToken]="CeCreateToken";	//               24
	W32_mids [W32_DuplicateHandle]="DuplicateHandle";	//             25
	W32_mids [W32_WaitForMultiple]="WaitForMultiple";	//             26
	W32_mids [W32_OutputDebugString]="OutputDebugString";	//           27
	W32_mids [W32_NKvDbgPrintfW]="NKvDbgPrintfW";	//               28
	W32_mids [W32_RaiseException]="RaiseException";	//              29
	W32_mids [W32_PerformCallBack]="PerformCallBack";	//             30
	W32_mids [W32_GetTickCount]="GetTickCount";	//                31
	W32_mids [W32_GetProcessVersion]="GetProcessVersion";	//           32     // GetProcessVersion
	W32_mids [W32_GetRomFileBytes]="GetRomFileBytes";	//             33
	W32_mids [W32_CeRevertToSelf]="CeRevertToSelf";	//              34
	W32_mids [W32_CeImpersonateCurrProc]="CeImpersonateCurrProc";	//       35
	W32_mids [W32_GetRomFileInfo]="GetRomFileInfo";	//              36
	W32_mids [W32_GetSystemInfo]="GetSystemInfo";	//               37
	W32_mids [W32_GetFSHeapInfo]="GetFSHeapInfo";	//               38
	W32_mids [W32_KernelIoControl]="KernelIoControl";	//             39
	W32_mids [W32_KernelLibIoControl]="KernelLibIoControl";	//          40
	W32_mids [W32_NKGetTime]="NKGetTime";	//                   41
	W32_mids [W32_GetDirectCallerProcessId]="GetDirectCallerProcessId";	//    42
	W32_mids [W32_GetOwnerProcessId]="GetOwnerProcessId";	//           43
	W32_mids [W32_GetIdleTime]="GetIdleTime";	//                 44
	W32_mids [W32_CacheRangeFlush]="CacheRangeFlush";	//             45
	W32_mids [W32_LoadExistingModule]="LoadExistingModule";	//          46  // u-mode only
	W32_mids [W32_GetCallerResInfo]="GetCallerResInfo";	//            46  // k-mode only
	W32_mids [W32_WaitForDebugEvent]="WaitForDebugEvent";	//           47
	W32_mids [W32_ContinueDebugEvent]="ContinueDebugEvent";	//          48
	W32_mids [W32_DebugNotify]="DebugNotify";	//                 49
	W32_mids [W32_AttachDebugger]="AttachDebugger";	//              50
	W32_mids [W32_BinaryCompress]="BinaryCompress";	//              51
	W32_mids [W32_BinaryDecompress]="BinaryDecompress";	//            52
	W32_mids [W32_DecompressBinaryBlock]="DecompressBinaryBlock";	//       53
	W32_mids [W32_StringCompress]="StringCompress";	//              54
	W32_mids [W32_StringDecompress]="StringDecompress";	//            55
	W32_mids [W32_CeGetRandomSeed]="CeGetRandomSeed";	//             56
	W32_mids [W32_SleepTillTick]="SleepTillTick";	//               57
	W32_mids [W32_QueryPerformanceFrequency]="QueryPerformanceFrequency";	//   58
	W32_mids [W32_QueryPerformanceCounter]="QueryPerformanceCounter";	//     59
	W32_mids [W32_GetThreadTimes]="GetThreadTimes";	//              60
	W32_mids [W32_QueryAPISetID]="QueryAPISetID";	//               61
	W32_mids [W32_IsNamedEventSignaled]="IsNamedEventSignaled";	//        62
	W32_mids [W32_CreateLocaleView]="CreateLocaleView";	//            63
	W32_mids [W32_RegisterDbgZones]="RegisterDbgZones";	//            64
	W32_mids [W32_SetDbgZone]="SetDbgZone";	//                  65
	W32_mids [W32_CreateMsgQueue]="CreateMsgQueue";	//              66
	W32_mids [W32_WriteDebugLED]="WriteDebugLED";	//               67
	W32_mids [W32_IsProcessorFeaturePresent]="IsProcessorFeaturePresent";	//   68
	W32_mids [W32_QueryInstructionSet]="QueryInstructionSet";	//         69
	W32_mids [W32_SetKernelAlarm]="SetKernelAlarm";	//              70
	W32_mids [W32_RefreshKernelAlarm]="RefreshKernelAlarm";	//          71
	W32_mids [W32_SetOOMEvent]="SetOOMEvent";	//                 72
	W32_mids [W32_NKSetTime]="NKSetTime";	//                   73
	W32_mids [W32_GetKPhys]="GetKPhys";	//                    74
	W32_mids [W32_GiveKPhys]="GiveKPhys";	//                   75
	W32_mids [W32_InputDebugCharW]="InputDebugCharW";	//             76
	W32_mids [W32_SetLowestScheduledPriority]="SetLowestScheduledPriority";	//  77
	W32_mids [W32_UpdateNLSInfoEx]="UpdateNLSInfoEx";	//             78
	W32_mids [W32_ReadRegistryFromOEM]="ReadRegistryFromOEM";	//         79
	W32_mids [W32_WriteRegistryToOEM]="WriteRegistryToOEM";	//          80
	W32_mids [W32_GetStdioPathW]="GetStdioPathW";	//               81
	W32_mids [W32_SetStdioPathW]="SetStdioPathW";	//               82
	W32_mids [W32_SetHardwareWatch]="SetHardwareWatch";	//            83
	W32_mids [W32_PrepareThreadExit]="PrepareThreadExit";	//           84
	W32_mids [W32_InterruptInitialize]="InterruptInitialize";	//         85
	W32_mids [W32_InterruptDisable]="InterruptDisable";	//            86
	W32_mids [W32_InterruptDone]="InterruptDone";	//               87
	W32_mids [W32_InterruptMask]="InterruptMask";	//               88
	W32_mids [W32_SetPowerHandler]="SetPowerHandler";	//             89
	W32_mids [W32_PowerOffSystem]="PowerOffSystem";	//              90
	W32_mids [W32_LoadIntChainHandler]="LoadIntChainHandler";	//         91
	W32_mids [W32_FreeIntChainHandler]="FreeIntChainHandler";	//         92
	W32_mids [W32_GetExitCodeProcess]="GetExitCodeProcess";	//          93
	W32_mids [W32_GetExitCodeThread]="GetExitCodeThread";	//           94
	W32_mids [W32_CreateStaticMapping]="CreateStaticMapping";	//         95
	W32_mids [W32_SetDaylightTime]="SetDaylightTime";	//             96
	W32_mids [W32_SetTimeZoneBias]="SetTimeZoneBias";	//             97
	W32_mids [W32_THCreateSnapshot]="THCreateSnapshot";	//            98
	W32_mids [W32_RemoteLocalAlloc]="RemoteLocalAlloc";	//            99
	W32_mids [W32_RemoteLocalReAlloc]="RemoteLocalReAlloc";	//          100
	W32_mids [W32_RemoteLocalSize]="RemoteLocalSize";	//             101
	W32_mids [W32_RemoteLocalFree]="RemoteLocalFree";	//             102
	W32_mids [W32_PSLNotify]="PSLNotify";	//                   103
	W32_mids [W32_SystemMemoryLow]="SystemMemoryLow";	//             104
	W32_mids [W32_LoadKernelLibrary]="LoadKernelLibrary";	//           105
	W32_mids [W32_CeLogData]="CeLogData";	//                   106
	W32_mids [W32_CeLogSetZones]="CeLogSetZones";	//               107
	W32_mids [W32_CeLogGetZones]="CeLogGetZones";	//               108
	W32_mids [W32_CeLogReSync]="CeLogReSync";	//                 109
	W32_mids [W32_WaitForAPIReady]="WaitForAPIReady";	//             110
	W32_mids [W32_SetRAMMode]="SetRAMMode";	//                  111
	W32_mids [W32_SetStoreQueueBase]="SetStoreQueueBase";	//           112
	W32_mids [W32_GetHandleServerId]="GetHandleServerId";	//           113
	W32_mids [W32_CeAccessCheck]="CeAccessCheck";	//               114
	W32_mids [W32_CePrivilegeCheck]="CePrivilegeCheck";	//            115
	W32_mids [W32_SetDirectCall]="SetDirectCall";	//               116
	W32_mids [W32_GetCallerVMProcessId]="GetCallerVMProcessId";	//        117
	W32_mids [W32_ProfileSyscall]="ProfileSyscall";	//              118
	W32_mids [W32_DebugSetProcessKillOnExit]="DebugSetProcessKillOnExit";	//   119
	W32_mids [W32_DebugActiveProcessStop]="DebugActiveProcessStop";	//      120
	W32_mids [W32_DebugActiveProcess]="DebugActiveProcess";	//          121
	W32_mids [W32_GetCallerProcess]="GetCallerProcess";	//            122     // old behavior GetCallerProcess
	W32_mids [W32_ReadKPEHeader]="ReadKPEHeader";	//               123
	W32_mids [W32_ForcePageout]="ForcePageout";	//                124
	W32_mids [W32_DeleteStaticMapping]="DeleteStaticMapping";	//         125
	W32_mids [W32_CeGetRawTime]="CeGetRawTime";	//                126     // CeGetRawTime
	W32_mids [W32_FileTimeToSystemTime]="FileTimeToSystemTime";	//        127     // FileTimeToSystemTime
	W32_mids [W32_SystemTimeToFileTime]="SystemTimeToFileTime";	//        128     // SystemTimeToFileTime
	W32_mids [W32_NKGetTimeAsFileTime]="NKGetTimeAsFileTime";	//         129     // NKGetTimeAsFileTime


	ID_PROC_mids [ID_PROC_TERMINATE]="TERMINATE"; //           ID_HCALL(0)
	ID_PROC_mids [ID_PROC_VMSETATTR]="VMSETATTR"; //           ID_HCALL(1)
	ID_PROC_mids [ID_PROC_FLUSHICACHE]="FLUSHICACHE"; //   ID_HCALL(2)
	ID_PROC_mids [ID_PROC_READMEMORY]="READMEMORY"; //   ID_HCALL(3)
	ID_PROC_mids [ID_PROC_WRITEMEMORY]="WRITEMEMORY"; //   ID_HCALL(4)
		// ID_HCALL(5) unused
	ID_PROC_mids [ID_PROC_GETMODINFO]="GETMODINFO"; //          ID_HCALL(6)
	ID_PROC_mids [ID_PROC_SETVER]="SETVER"; //              ID_HCALL(7)
	ID_PROC_mids [ID_PROC_VMALLOC]="VMALLOC"; //             ID_HCALL(8)
	ID_PROC_mids [ID_PROC_VMFREE]="VMFREE"; //              ID_HCALL(9)
	ID_PROC_mids [ID_PROC_VMQUERY]="VMQUERY"; //             ID_HCALL(10)
	ID_PROC_mids [ID_PROC_VMPROTECT]="VMPROTECT"; //           ID_HCALL(11)
	ID_PROC_mids [ID_PROC_VMCOPY]="VMCOPY"; //              ID_HCALL(12)
	ID_PROC_mids [ID_PROC_VMLOCK]="VMLOCK"; //              ID_HCALL(13)
	ID_PROC_mids [ID_PROC_VMUNLOCK]="VMUNLOCK"; //            ID_HCALL(14)
	ID_PROC_mids [ID_PROC_HNDLCLOSE]="HNDLCLOSE"; //           ID_HCALL(15)
	ID_PROC_mids [ID_PROC_UNMAPVIEW]="UNMAPVIEW"; //           ID_HCALL(16)
	ID_PROC_mids [ID_PROC_FLUSHVIEW]="FLUSHVIEW"; //           ID_HCALL(17)
	ID_PROC_mids [ID_PROC_MAPVIEW]="MAPVIEW"; //             ID_HCALL(18)
	ID_PROC_mids [ID_PROC_MAPCREATE]="MAPCREATE"; //           ID_HCALL(19)
		// ID_HCALL(20) unused
	ID_PROC_mids [ID_PROC_GETMODNAME]="GETMODNAME"; //          ID_HCALL(21)
	ID_PROC_mids [ID_PROC_GETMODHNDL]="GETMODHNDL"; //          ID_HCALL(22)
	ID_PROC_mids [ID_PROC_VMALLOCPHYS]="VMALLOCPHYS"; //         ID_HCALL(23)
	ID_PROC_mids [ID_PROC_READPEHEADER]="READPEHEADER"; //        ID_HCALL(24)
	ID_PROC_mids [ID_PROC_CHECKREMOTEDEBUGGERPRESENT]="CHECKREMOTEDEBUGGERPRESENT"; // ID_HCALL(25)
	ID_PROC_mids [ID_PROC_OPENMSGQEX]="OPENMSGQEX"; //          ID_HCALL(26)
	ID_PROC_mids [ID_PROC_GETPROCID]="GETPROCID"; //           ID_HCALL(27)
	ID_PROC_mids [ID_PROC_VMALLOCCOPY]="VMALLOCCOPY"; //         ID_HCALL(28)
	ID_PROC_mids [ID_PROC_PAGEOUTMODULE]="PAGEOUTMODULE"; //       ID_HCALL(29)

		/* Thread syscalls */

	ID_THREAD_mids [ID_THREAD_SUSPEND]="SUSPEND"; //           ID_HCALL(0)
	ID_THREAD_mids [ID_THREAD_RESUME]="RESUME"; //            ID_HCALL(1)
	ID_THREAD_mids [ID_THREAD_SETTHREADPRIO]="SETTHREADPRIO"; //     ID_HCALL(2)
	ID_THREAD_mids [ID_THREAD_GETTHREADPRIO]="GETTHREADPRIO"; //     ID_HCALL(3)
	ID_THREAD_mids [ID_THREAD_GETCALLSTACK]="GETCALLSTACK"; //      ID_HCALL(4)
	ID_THREAD_mids [ID_THREAD_GETCONTEXT]="GETCONTEXT"; //  ID_HCALL(5)
	ID_THREAD_mids [ID_THREAD_SETCONTEXT]="SETCONTEXT"; //  ID_HCALL(6)
	ID_THREAD_mids [ID_THREAD_TERMINATE]="TERMINATE"; //   ID_HCALL(7)
	ID_THREAD_mids [ID_THREAD_CEGETPRIO]="CEGETPRIO"; //   ID_HCALL(8)
	ID_THREAD_mids [ID_THREAD_CESETPRIO]="CESETPRIO"; //   ID_HCALL(9)
	ID_THREAD_mids [ID_THREAD_CEGETQUANT]="CEGETQUANT"; //  ID_HCALL(10)
	ID_THREAD_mids [ID_THREAD_CESETQUANT]="CESETQUANT"; //  ID_HCALL(11)
	ID_THREAD_mids [ID_THREAD_GETID]="GETID"; //             ID_HCALL(12)
	ID_THREAD_mids [ID_THREAD_GETPROCID]="GETPROCID"; //         ID_HCALL(13)

		/* Token syscalls */
	ID_TOKEN_mids [ID_TOKEN_IMPERSONATE]="IMPERSONATE"; //        ID_HCALL(0)
	ID_TOKEN_mids [ID_TOKEN_GETOWNERSID]="GETOWNERSID"; //        ID_HCALL(1)
	ID_TOKEN_mids [ID_TOKEN_GETGROUPSID]="GETGROUPSID"; //        ID_HCALL(2)

		/* Event syscalls */

	ID_EVENT_mids [ID_EVENT_MODIFY]="MODIFY"; //             ID_HCALL(0)
	ID_EVENT_mids [ID_EVENT_GETDATA]="GETDATA"; //            ID_HCALL(1)
	ID_EVENT_mids [ID_EVENT_SETDATA]="SETDATA"; //            ID_HCALL(2)
	ID_EVENT_mids [ID_EVENT_READMSGQ]="READMSGQ"; //           ID_HCALL(3)
	ID_EVENT_mids [ID_EVENT_WRITEMSGQ]="WRITEMSGQ"; //          ID_HCALL(4)
	ID_EVENT_mids [ID_EVENT_GETMSGQINFO]="GETMSGQINFO"; //        ID_HCALL(5)
	ID_EVENT_mids [ID_EVENT_WDSTART]="WDSTART"; //            ID_HCALL(6)
	ID_EVENT_mids [ID_EVENT_WDSTOP]="WDSTOP"; //             ID_HCALL(7)
	ID_EVENT_mids [ID_EVENT_WDREFRESH]="WDREFRESH"; //          ID_HCALL(8)
	ID_EVENT_mids [ID_EVENT_RESUMEMAINTH]="RESUMEMAINTH"; //       ID_HCALL(9)

		/* Apiset syscalls */

	ID_APISET_mids [ID_APISET_REGISTER]="REGISTER"; //            ID_HCALL(0)
	ID_APISET_mids [ID_APISET_CREATEHANDLE]="CREATEHANDLE"; //        ID_HCALL(1)
	ID_APISET_mids [ID_APISET_VERIFY]="VERIFY"; //              ID_HCALL(2)
	ID_APISET_mids [ID_APISET_REGDIRECTMTHD]="REGDIRECTMTHD"; //       ID_HCALL(3)
	ID_APISET_mids [ID_APISET_LOCKAPIHANDLE]="LOCKAPIHANDLE"; //       ID_HCALL(4)
	ID_APISET_mids [ID_APISET_UNLOCKAPIHANDLE]="UNLOCKAPIHANDLE"; //     ID_HCALL(5)
	ID_APISET_mids [ID_APISET_SETAPIERRORHANDLER]="SETAPIERRORHANDLER"; //  ID_HCALL(6)

		/* mapped file syscalls */

	ID_FSMAP_mids [ID_FSMAP_MAPVIEWOFFILE]="MAPVIEWOFFILE"; //  ID_HCALL(0)

		/* mutex syscalls */

	ID_MUTEX_mids [ID_MUTEX_RELEASEMUTEX]="RELEASEMUTEX"; //  ID_HCALL(0)

		/* semaphore syscalls */

	ID_SEMAPHORE_mids [ID_SEMAPHORE_RELEASESEMAPHORE]="RELEASESEMAPHORE"; // ID_HCALL(0)


	hid_table[ SH_WIN32 ].name="Win32";
	hid_table[ SH_WIN32 ].mids=W32_mids;

	hid_table[ SH_CURTHREAD  ].name="CURTHREAD";
	hid_table[ SH_CURTHREAD ].mids=ID_THREAD_mids;

	hid_table[ SH_CURPROC ].name="CURPROC";
	hid_table[ SH_CURPROC ].mids=ID_PROC_mids;

	hid_table[ SH_CURTOKEN ].name="CURTOKEN";
	hid_table[ SH_CURTOKEN ].mids=ID_TOKEN_mids;

	hid_table[ HT_EVENT ].name="EVENT";			// Event handle type
	hid_table[ HT_EVENT ].mids=ID_EVENT_mids;

	hid_table[ HT_MUTEX ].name="MUTEX";			// Mutex handle type
	hid_table[ HT_MUTEX ].mids=ID_MUTEX_mids;

	hid_table[ HT_APISET ].name="APISET";		// kernel API set handle type
	hid_table[ HT_APISET ].mids=ID_APISET_mids;

	hid_table[ HT_FILE ].name="FILE";			// open file handle type
	hid_table[ HT_FIND ].name="FIND";			// FindFirst handle type
	hid_table[ HT_DBFILE ].name="DBFILE";		// open database handle type
	hid_table[ HT_DBFIND ].name="DBFIND";		// database find handle type
	hid_table[ HT_SOCKET ].name="SOCKET";		// WinSock open socket handle type
	hid_table[ HT_CRITSEC ].name="CRITSEC";		// Critical section
	hid_table[ HT_SEMAPHORE ].name="SEMAPHORE";		// Semaphore handle type
	hid_table[ HT_SEMAPHORE ].mids=ID_SEMAPHORE_mids;

	hid_table[ HT_FSMAP ].name="FSMAP";			// mapped files
	hid_table[ HT_FSMAP ].mids=ID_FSMAP_mids;

	hid_table[ HT_WNETENUM ].name="WNETENUM ";		// Net Resource Enumeration
	hid_table[ HT_AFSVOLUME ].name="AFSVOLUME";		// file system volume handle type
	hid_table[ SH_GDI ].name="GDI";
	hid_table[ SH_WMGR ].name="WMGR";
	hid_table[ SH_WNET ].name="WNET";			// WNet APIs for network redirector
	hid_table[ SH_COMM ].name="COMM";			// Communications not COM
	hid_table[ SH_FILESYS_APIS ].name="FILESYS";	// File system APIS
	hid_table[ SH_SHELL ].name="SHELL";
	hid_table[ SH_DEVMGR_APIS ].name="DEVMGR";	// File system device manager
	hid_table[ SH_TAPI ].name="TAPI";
	hid_table[ SH_CPROG ].name="CPROG";			// Handle to the specified API set
	hid_table[ SH_SERVICES ].name="SERVICES";
	hid_table[ SH_DDRAW ].name="DDRAW";
	hid_table[ SH_D3DM ].name="D3DM";
}

char GetApiNameTemp[128];
const char* GetApiName(u32 v)
{
	if (v>=LAST_METHOD && v<=FIRST_METHOD)
	{
		//(FIRST_METHOD - ((hid)<<APISET_SHIFT | (mid))*APICALL_SCALE)
		u32 uid=(-(v-FIRST_METHOD))/APICALL_SCALE;
		u32 hid=(uid>>APISET_SHIFT)&APISET_MASK;
		u32 mid=(uid&METHOD_MASK);
		if (hid==SH_WIN32 && mid==W32_OutputDebugString)
		{
			printf("OutputDebugString:%08X\n",r[4]);
		}
		if (hid_table[hid].mids==0 || hid_table[hid].mids[mid]==0)
		{
			if (hid_table[hid].name)
				sprintf(GetApiNameTemp,"%s:%d",hid_table[hid].name,mid);
			else
				sprintf(GetApiNameTemp,"%d:%d",hid,mid);
		}
		else
		{
			sprintf(GetApiNameTemp,"%s:%s",hid_table[hid].name,hid_table[hid].mids[mid]);
		}

		return GetApiNameTemp;
	}
	else
		return 0;
}