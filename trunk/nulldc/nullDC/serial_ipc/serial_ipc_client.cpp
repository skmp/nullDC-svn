
#include "..\types.h"
#include <windows.h>
#include <assert.h>
#include <iostream>

#define sipcver "0.3 -spdma -buffered"

HANDLE readp=NULL;
HANDLE writep=NULL;
#define buff_size (512*1024)
u8 ReadBuff[buff_size];

u32 readind=0;
u32 ReadSz=0;
u32 serial_RxBytes=0;
u32 serial_TxBytes=0;
u32 last_serial_RxBytes=0;
u32 last_serial_TxBytes=0;
u32 last_serial_Rx_tick=0;
u32 last_serial_Tx_tick=0;

void StartGDBSession();

void PrintSerialIPUsage(int argc, char *argv[])
{
	if (argc==1)
	{
		printf("If you want to use serial port ipc redirection use -slave piperead pipewrite \n");
		//StartGDBSession();
		return;
	}

	if (argc!=4)
	{
		printf("Serial port pipe redirection version %s\n",sipcver);
		printf("Wrong number of parameters , expecting  nulldc -slave piperead pipewrite \n");
		printf("redirection disabled");
		//StartGDBSession();
		return;
	}
	
	if (strcmp(argv[1],"-slave")!=0)
	{
		printf("Serial port pipe redirection version %s\n",sipcver);
		printf("Wrong type of parameters , expecting  nulldc -slave piperead pipewrite \n");
		printf("redirection disabled");
		//StartGDBSession();
		return;
	}

	//TODO : how to do on 64b compatable code ?
	HANDLE laddWrSlave = (HANDLE)(u64)atoi(argv[2]);
	HANDLE laddRdSlave = (HANDLE)(u64)atoi(argv[3]);
	printf("Value of write handle to pipe1: %p\n",laddWrSlave);
	printf("Value of read handle to pipe2 : %p\n",laddRdSlave);

	//this warning can't be fixed can it ?
	writep = laddWrSlave;
	readp = laddRdSlave;

}
void WriteBlockSerial(u8* blk,u32 size,u8* sum)
{
	serial_TxBytes+=size;
	*sum=0; 
	for (u32 i=0;i<size;i++)
			*sum^=blk[i];
	if (!writep)
	{
		for (u32 i=0;i<size;i++)
			putc(blk[i],stdout);
		return;
	}

	//printf("Write IPC not implemented");
	DWORD dwWritten=0;
	
	if (!WriteFile(writep,blk,size,&dwWritten,NULL))
		printf("IPC error");

	if (dwWritten!=size)
		printf("IPC error");
}
void WriteSerial(u8 data)
{
	serial_TxBytes++;
	if (!writep)
	{
		putc(data,stdout);
		return;
	}

	//printf("Write IPC not implemented");
	DWORD dwWritten=0;
	
	if (!WriteFile(writep,&data,1,&dwWritten,NULL))
		printf("IPC error");

	if (dwWritten!=1)
		printf("IPC error");
}

int i=0;
bool PendingSerialData()
{
	if(!readp)
		return false;

	if (readind==0)
	{
		DWORD tba=0;
		if (((i++)%256)==0)
			PeekNamedPipe(readp,NULL,NULL,NULL,&tba,NULL);
		if (tba!=0)
		{
			readind=tba;
			if (readind>(buff_size)) readind=buff_size;
			ReadFile(readp,ReadBuff,readind,&tba,NULL);
			ReadSz=tba;
			if (readind!=tba)
				printf("IPC ERROR \n");
		}
		return tba!=0;
	}
	else
	{
		return true;
	}
}
s32 ReadSerial()
{
	serial_RxBytes++;
	if (!readp)
		return -1;

	//u8 read_data;
	//DWORD dwRead;
	if (readind<=0)
	{
		printf("IPC error");
		return -1;
	}
	
	u8 rv= ReadBuff[ReadSz-readind];
	readind--;
	return rv;
/*
	if (!ReadFile(readp,&read_data,1,&dwRead,NULL))
		printf("IPC error");
	if (dwRead!=1)
		printf("IPC error");
	//printf("Read IPC not implemented");
	return read_data;*/
	
}



float GetRxSpeed()
{
	double tp=((double)timeGetTime()-(double)last_serial_Rx_tick)/1000.0;
	float rv=(float)((serial_RxBytes-last_serial_RxBytes)/tp);
	last_serial_RxBytes=serial_RxBytes;
	last_serial_Rx_tick=timeGetTime();
	return rv;
}

float GetTxSpeed()
{
	double tp=((double)timeGetTime()-(double)last_serial_Tx_tick)/1000.0;
	float rv=(float)((serial_TxBytes-last_serial_TxBytes)/tp);
	last_serial_TxBytes=serial_TxBytes;
	last_serial_Tx_tick=timeGetTime();
	return rv;
}