/*
**	zNullMaple.cpp,	David Miller (2007)
*/
#include <windows.h>
#include <dinput.h>
#include <stdio.h>

#include "zNullMaple.h"
#include "MapleBus.h"
#include "DInput.h"
#include "resource.h"


HINSTANCE hInst=NULL;

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD   ul_reason_for_call,
					  LPVOID  lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}









emu_info ei;

void FASTCALL Unload();
s32  FASTCALL Load(emu_info* emu);


s32  FASTCALL Create(maple_device_instance* inst,u32 id,u32 flags,u32 rootmenu);
s32  FASTCALL CreateSub(maple_subdevice_instance* inst,u32 id,u32 flags,u32 rootmenu);
void FASTCALL Destroy(void* inst,u32 id);
s32  FASTCALL Init(void* inst, u32 id, maple_init_params* params);
void FASTCALL Term(void* inst, u32 id);

u32 FASTCALL DeviceDMA(void* device_instance,u32 Command,u32* buffer_in,u32 buffer_in_len,u32* buffer_out,u32& buffer_out_len);






void EXPORT_CALL dcGetInterface(plugin_interface* info)
{
	maple_plugin_if	* pm	= &info->maple;
	common_info		* pci	= &info->common;
	
	info->InterfaceVersion	= PLUGIN_I_F_VERSION;

	pci->Load				= Load;
	pci->Unload				= Unload;
	pci->Type				= Plugin_Maple;
	pci->InterfaceVersion	= MAPLE_PLUGIN_I_F_VERSION;
	swprintf(pci->Name,		L"ZnullMaple v1.0 beta1");

	pm->Init			= Init;
	pm->Term			= Term;
	pm->CreateSub		= CreateSub;
	pm->CreateMain		= Create;
	pm->Destroy			= Destroy;

	swprintf(pm->devices[0].Name, L"Controller Device");
	pm->devices[1].Type		= MDT_EndOfList;
	pm->devices[0].Type		= MDT_Main;
	pm->devices[0].Flags	= MDF_Controller;
}





void WriteDefCfg()
{
	wchar cfg_key[512], cfg_sub[512];
	for(int p=0; p<4; p++) {
		swprintf(cfg_key, L"zNullMaple_port%02X", p);
		printf("Writing DefCfg for %s\n", cfg_key);

		ei.ConfigSaveInt(cfg_key, L"DGUID_Data1", 0);	// long
		ei.ConfigSaveInt(cfg_key, L"DGUID_Data2", 0);	// short
		ei.ConfigSaveInt(cfg_key, L"DGUID_Data3", 0);	// short
		ei.ConfigSaveInt(cfg_key, L"DGUID_Data4a", 0);	// char[0-3] -> long
		ei.ConfigSaveInt(cfg_key, L"DGUID_Data4b", 0);	// char[4-7] -> long

		ei.ConfigSaveInt(cfg_key, L"Connected", 0);
		ei.ConfigSaveInt(cfg_key, L"DevType", 0);

		for(size_t k=0; k<32; k++) {
			swprintf(cfg_sub, L"KeyMap[%02X]", k);
			ei.ConfigSaveInt(cfg_key, cfg_sub, 0);
		}
	}
}

void SaveCfg()
{
	GUID tguid;
	wchar cfg_key[512], cfg_sub[512];
	for(int p=0; p<4; p++) {
		tguid = InputDev[p].guidDev;
		swprintf(cfg_key, L"zNullMaple_port%02X", p);

		ei.ConfigSaveInt(cfg_key, L"DGUID_Data1", tguid.Data1);	// long
		ei.ConfigSaveInt(cfg_key, L"DGUID_Data2", tguid.Data2);	// short
		ei.ConfigSaveInt(cfg_key, L"DGUID_Data3", tguid.Data3);	// short
		ei.ConfigSaveInt(cfg_key, L"DGUID_Data4a", ((u32*)tguid.Data4)[0]);	// char[0-3] -> long
		ei.ConfigSaveInt(cfg_key, L"DGUID_Data4b", ((u32*)tguid.Data4)[1]);	// char[4-7] -> long

		ei.ConfigSaveInt(cfg_key, L"Connected", InputDev[p].Connected);
		ei.ConfigSaveInt(cfg_key, L"DevType", InputDev[p].DevType);

		for(size_t k=0; k<32; k++) {
			swprintf(cfg_sub, L"KeyMap[%02X]", k);
			ei.ConfigSaveInt(cfg_key, cfg_sub, InputDev[p].KeyMap[k]);
		}
	}
}

void LoadCfg()
{
	GUID tguid;
	wchar cfg_key[512], cfg_sub[512];
	for(int p=0; p<4; p++) {
		swprintf(cfg_key, L"zNullMaple_port%02X", p);

		if(!ei.ConfigExists(cfg_key, L"Connected"))
			WriteDefCfg();

		tguid.Data1	= ei.ConfigLoadInt(cfg_key, L"DGUID_Data1", 0);	// long
		tguid.Data2	= ei.ConfigLoadInt(cfg_key, L"DGUID_Data2", 0);	// short
		tguid.Data3	= ei.ConfigLoadInt(cfg_key, L"DGUID_Data3", 0);	// short
		((u32*)tguid.Data4)[0]	= ei.ConfigLoadInt(cfg_key, L"DGUID_Data4a", 0);	// char[0-3] -> long
		((u32*)tguid.Data4)[1]	= ei.ConfigLoadInt(cfg_key, L"DGUID_Data4b", 0);	// char[4-7] -> long

		InputDev[p].guidDev		= tguid;
		InputDev[p].Connected	= ei.ConfigLoadInt(cfg_key, L"Connected", 0);
		InputDev[p].DevType		= ei.ConfigLoadInt(cfg_key, L"DevType", 0);

		for(size_t k=0; k<32; k++) {
			swprintf(cfg_sub, L"KeyMap[%02X]", k);
			InputDev[p].KeyMap[k] = ei.ConfigLoadInt(cfg_key, cfg_sub, 0);
		}
	}
}



s32  FASTCALL Load(emu_info* emu)
{
	/////////////////////
	ei = *(emu_info*)emu;
	/////////////////////

	// Load Config //
	LoadCfg();

	if(!InitDInput(hInst))
		printf("DInput is fucked !\n");

	return rv_ok;
}

void FASTCALL Unload()
{
//	ei.WindowHandle = NULL; // -> wtf do you mean by that z ? :P
	TermDInput();
}

s32  FASTCALL Init(void* inst, u32 id, maple_init_params* params)
{
	return rv_ok;
}
void FASTCALL Term(void* inst, u32 id)
{

}


	/////////////////////// TEMP //////////////////////



u32 curr_port=0, curr_sel=0;

DWORD WINAPI ThreadStart(LPVOID lpParameter)
{
	while(5 != curr_port)
	{
		volatile u32 csel = curr_sel;
		s32 rv = GetChInput(InputDev[curr_port]);

		if((rv > 0) && (420 != csel))
		{
			switch(InputDev[curr_port].DevType)
			{
			case DI8DEVCLASS_KEYBOARD:
				InputDev[curr_port].KeyMap[csel] = (rv - DINPUT_KB_FIRST);
				SetDlgItemText((HWND)lpParameter, IDC_EDIT1+csel, MapNames[rv]);
			break;

			case DI8DEVCLASS_GAMECTRL:
				InputDev[curr_port].KeyMap[csel] = (rv - DINPUT_GP_BUT1);
				SetDlgItemText((HWND)lpParameter, IDC_EDIT1+csel, MapNames[rv]);
			break;

			case DI8DEVCLASS_POINTER:
				printf("Mouse Input Type, Unsupported!\n");
			break;

			default:
				printf("Default Input Type, WTF?\n");
			return -3;
			}
		}
		Sleep(100);
	}
	return 0;
}

void UpdateDlg(HWND hDlg, u32 port)
{	
	CheckDlgButton(hDlg, IDC_CONNECTED, InputDev[port].Connected);

	// Update ComboBox to prop. device
	for(size_t x=0; x<diDevInfoList.size(); x++)
		if(diDevInfoList[x].guid == InputDev[port].guidDev)
			SendMessage(GetDlgItem(hDlg,IDC_SELECT),CB_SETCURSEL, x,0);

	// Get proper mapstart offs. for dev type
	u32 mapstart = DINPUT_KB_FIRST;
	if(DI8DEVCLASS_GAMECTRL==InputDev[port].DevType) mapstart = DINPUT_GP_BUT1;
	if(DI8DEVCLASS_POINTER ==InputDev[port].DevType) mapstart = DINPUT_GP_BUT1;

	// Update each
	for(int i=0; i<13; i++) {
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT1+i), InputDev[port].Connected);
		SetDlgItemText(hDlg, IDC_EDIT1+i, MapNames[mapstart+InputDev[port].KeyMap[i]]);
	}
}


INT_PTR CALLBACK dlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static u32 ThreadID=0;
	static HANDLE hThread=NULL;

	switch(uMsg)
	{
	case WM_INITDIALOG:
	{
		// Create Thread for DInput Checking //
		hThread = CreateThread(NULL, 0, ThreadStart, (LPVOID)hDlg, 0, (LPDWORD)&ThreadID);
		if(NULL == hThread) {
			printf("CreateThread Failed in dlgProc for nullMaple\n");
			return false;
		}

		// Setup ComboBox Selection
		for(size_t x=0; x<diDevInfoList.size(); x++)
			SendMessage(GetDlgItem(hDlg,IDC_SELECT), CB_ADDSTRING, 0, (LPARAM)diDevInfoList[x].name);
		SendMessage(GetDlgItem(hDlg,IDC_SELECT), CB_SETCURSEL, 0, 0);


		// Setup Initial Mappings	*FIXME* port !
		UpdateDlg(hDlg,curr_port);

		return true;
	}

	case WM_COMMAND:
	{
		switch(LOWORD(wParam))
		{
		case IDC_EDIT1:		case IDC_EDIT2:		case IDC_EDIT3:		case IDC_EDIT4:
		case IDC_EDIT5:		case IDC_EDIT6:		case IDC_EDIT7:		case IDC_EDIT8:
		case IDC_EDIT9:		case IDC_EDIT10:	case IDC_EDIT11:	/*case IDC_EDIT12:
		case IDC_EDIT13:*/
		{
			// *FIXME* Note, specific to dc cont. UI only //
			switch(HIWORD(wParam))
			{
			case EN_SETFOCUS:
				curr_sel = 10 - (IDC_EDIT11 - LOWORD(wParam));
			return true;

			case EN_KILLFOCUS:
				curr_sel = 0x420;
			return true;

				// *FIXME* Use IF kboard not in-use, else you can type shit ....
			case EN_CHANGE:
				return true;	

			default: break;
			}

			return false;
		}

		case IDC_CONNECTED:
			if(BN_CLICKED == HIWORD(wParam)) {
				InputDev[curr_port].Connected = 
					(BST_CHECKED==IsDlgButtonChecked(hDlg,IDC_CONNECTED)) ? true : false ;
				UpdateDlg(hDlg,curr_port);
			}
			return true;

		case IDC_SELECT:
			if(CBN_SELCHANGE==HIWORD(wParam)) {
				int guidIdx = SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
				if(!InputDev[curr_port].ReAqcuire(guidIdx))
					printf("IDC_SELECT::InputDev[%02X].ReAqcuire(%X) Failed!\n", curr_port, guidIdx);
				UpdateDlg(hDlg,curr_port);
			}
			return true;

		case IDOK:
			SaveCfg();
			curr_port = 5;		// terminate input thread
			EndDialog(hDlg,0);
			return true;

		case IDCANCEL:
			curr_port = 5;		// terminate input thread
			EndDialog(hDlg,0);
			return true;

		default: break;
		}
		return false;
	}

	default: break;
	}
	return false;
}

void EXPORT_CALL menu_cb(u32 id, void* handle, void* p)
{
	curr_port = ((maple_device_instance*)p)->port>>6;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DICFG), (HWND)handle, dlgProc);
}
/////////////////////// TEMP //////////////////////



s32  FASTCALL Create(maple_device_instance* inst,u32 id,u32 flags,u32 rootmenu)
{
	wchar menu_str[512];
	curr_port = (inst->port>>6);
	swprintf(menu_str, L"Maple Device Config, Port: %d",curr_port);
	u32 mID = ei.AddMenuItem(rootmenu, -1, menu_str, menu_cb, 0);

	MenuItem mi;
	mi.PUser=inst;
	ei.SetMenuItem(mID, &mi, MIM_PUser);

	inst->dma = DeviceDMA;
	inst->data= inst;
	return rv_ok;
}

void FASTCALL Destroy(void* inst,u32 id)
{

}




u32 FASTCALL SubDeviceDMA(maple_subdevice_instance* dev_inst, u32 Command, u32* buffer_in,
						   u32 in_len, u32* buffer_out, u32& out_len)
{
	printf("Unhandled MapleSubDMA to %X: Command: %0Xh\n", dev_inst->port, Command);
	return 0xFF;
}
u32 FASTCALL DeviceDMA(void* data, u32 Command, u32* buffer_in,
						u32 in_len, u32* buffer_out, u32& out_len)
{
	maple_device_instance* dev_inst=(maple_device_instance*)data;
	curr_port = dev_inst->port>>6;

	switch(Command)
	{
	case MAPLE_DEV_REQ:
	{
		FixedDevStatus * fds = (FixedDevStatus*)buffer_out;

		if(0 != in_len)
			printf("MAPLE_DEV_REQ, in_len != 0\n");

		out_len	= 0x1C<<2;	// sizeof(FixedDevStatus)


		// This is for a controller only atm
		fds->DevID.FD[2]	= fds->DevID.FD[1] = 0;
		fds->DevID.FD[0]	= FD_CONTROLLER;		// *FIXME*
		fds->DevID.FT		= FT_CONTROLLER;		// FT_STORAGE
		fds->DestCode		= DEST_WORLDWIDE;

		fds->Direction		= DIR_TOP;
		fds->StandbyCurrent	= 0x01AE;	// 1AE = 43 mA |(O) 0x0069 = 10.5mA
		fds->MaximumCurrent	= 0x01F4;	// 1F4 = 50 mA |(O) 0x04FF = 127.9mA

		strcpy((char*)fds->ProductName,"Dreamcast Controller         ");
		strcpy((char*)fds->License,"Produced By or Under License From SEGA ENTERPRISES,LTD.    ");

		return MAPLE_DEV_STATUS;
	}

	case MAPLE_DEV_REQALL:
	case MAPLE_DEV_RESET:
	case MAPLE_DEV_KILL:
		break;


	case MAPLE_GET_CONDITION:
	{
		if(1 != (in_len>>2))
			printf("MAPLE_GET_CONDITION, in_len(%d) != 1\n", in_len);

		if(FT_CONTROLLER != buffer_in[0])
			printf("MAPLE_GET_CONDITION, !AP_DE \n");

		out_len = 0x03<<2;

		buffer_out[0] = FT_CONTROLLER;
		Controller_ReadFormat * crf = 
			(Controller_ReadFormat *)&buffer_out[1];

		crf->Buttons = ~0;
		crf->LT = crf->RT = 0;
		crf->Ax1 = crf->Ax2 = 128;
		crf->Ax3 = crf->Ax4 = 0;
		if(!GetDInput(curr_port,crf))
			printf("Failed To Get DInput State!\n");

		return MAPLE_DATA_TRANSFER;
	}


	case MAPLE_GET_MEDIAINFO:
	case MAPLE_BLOCK_READ:
	case MAPLE_BLOCK_WRITE:
	case MAPLE_GET_LAST_ERR:
	case MAPLE_SET_CONDITION:
	case MAPLE_FT_CONTROL:
	case MAPLE_AR_CONTROL:
		break;
	}
	printf("Unhandled MapleDMA to %X: Command: %0Xh\n", dev_inst->port, Command);
	return MAPLE_ERR_CMD_UNK;
}



s32  FASTCALL CreateSub(maple_subdevice_instance* inst,u32 id,u32 flags,u32 rootmenu) { return rv_ok; }



