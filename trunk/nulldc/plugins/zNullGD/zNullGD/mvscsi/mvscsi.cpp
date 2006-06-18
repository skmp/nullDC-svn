#include <windows.h>
#include "mvscsi.h"
#include "mvspti.h"
using namespace std;

namespace { LPmvSPTI_BusList mvSPTI_GlobalBusList; }

//BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
//	switch(reason) {
//		case DLL_PROCESS_ATTACH: /*MessageBox(NULL, "Process attach", "mvSCSI DLL", MB_OK);*/ break;
//		case DLL_THREAD_ATTACH:  /*MessageBox(NULL, "Thread attach",  "mvSCSI DLL", MB_OK);*/ break;
//		case DLL_THREAD_DETACH:  /*MessageBox(NULL, "Thread detach",  "mvSCSI DLL", MB_OK);*/ break;
//		case DLL_PROCESS_DETACH: /*MessageBox(NULL, "Process detach", "mvSCSI DLL", MB_OK);*/ break;
//	}
//	return TRUE;
//}

//__declspec(dllexport) 
DWORD mvSCSI_Init(VOID) {
	if(!mvSPTI_GlobalBusList) {
		try {
			mvSPTI_GlobalBusList = LPmvSPTI_BusList(new mvSPTI_BusList);
			for(BYTE busNumber = 0; busNumber < mvSCSI_MAX_BUS; ++busNumber) 
			{
				try 
				{
					mvSPTI_GlobalBusList->push_back(LPmvSPTI_Bus(new mvSPTI_Bus(busNumber)));
				}
				catch(...) 
				{
					//LPmvSPTI_BusList* s;
					mvSPTI_GlobalBusList->push_back(LPmvSPTI_Bus());
				}
			}
		} catch(mvSPTI_InvalidHandle&) {
		} catch(...) {
			mvSPTI_GlobalBusList.reset();
			return (mvSCSI_ERROR << 8);
		}
	}

	return ((mvSPTI_GlobalBusList->size() > 0 ? mvSCSI_OK : mvSCSI_NO_BUS) << 8) | (BYTE)mvSPTI_GlobalBusList->size();
}

//__declspec(dllexport) 
DWORD mvSCSI_RescanBus(LPmvSCSI_Bus bus) {
	if(!mvSPTI_GlobalBusList) { return mvSCSI_NO_INIT; }

	try {
		mvSPTI_GlobalBusList->at(bus->busNumber)->RescanBus(bus);
	} catch(out_of_range&) {
		return mvSCSI_NO_BUS;
	} catch(...) {
		return mvSCSI_ERROR;
	}

	return mvSCSI_OK;
}

//__declspec(dllexport) 
DWORD mvSCSI_InquiryBus(LPmvSCSI_Bus bus) {
	if(!mvSPTI_GlobalBusList) { return mvSCSI_NO_INIT; }

	try {
		mvSPTI_GlobalBusList->at(bus->busNumber)->InquiryBus(bus);
	} catch(out_of_range&) {
		return mvSCSI_NO_BUS;
	}

	return mvSCSI_OK;
}

//__declspec(dllexport) 
DWORD mvSCSI_InquiryDev(LPmvSCSI_Dev dev) {
	if(!mvSPTI_GlobalBusList) { return mvSCSI_NO_INIT; }

	try {
		mvSPTI_GlobalBusList->at(dev->devBus)->GetDevice(dev->devPath, dev->devTarget, dev->devLun)->InquiryDev(dev);
	} catch(out_of_range&) {
		return mvSCSI_NO_DEV;
	}

	return mvSCSI_OK;
}

//__declspec(dllexport) 
DWORD mvSCSI_SetDevTimeOut(LPmvSCSI_Dev dev) {
	if(!mvSPTI_GlobalBusList) { return mvSCSI_NO_INIT; }

	try {
		mvSPTI_GlobalBusList->at(dev->devBus)->GetDevice(dev->devPath, dev->devTarget, dev->devLun)->SetDevTimeOut(dev);
	} catch(out_of_range&) {
		return mvSCSI_NO_DEV;
	}

	return mvSCSI_OK;
}

//__declspec(dllexport) 
DWORD mvSCSI_ExecCmd(LPmvSCSI_Cmd cmd) {
	if(!mvSPTI_GlobalBusList) { return mvSCSI_NO_INIT; }

	try {
		mvSPTI_GlobalBusList->at(cmd->devBus)->GetDevice(cmd->devPath, cmd->devTarget, cmd->devLun)->ExecCmd(cmd);
	} catch(out_of_range&) {
		return mvSCSI_NO_DEV;
	} catch(mvSPTI_Check&) {
		return mvSCSI_CHECK;
	} catch(mvSPTI_Error&) {
		return mvSCSI_ERROR;
	}

	return mvSCSI_OK;
}
