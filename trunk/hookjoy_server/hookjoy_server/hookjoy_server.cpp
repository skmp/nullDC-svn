// hookjoy_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#define MSG_WAITALL 0
typedef signed __int8  s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef float f32;
typedef double f64;


/*
	how the code works :

	Client Connects
	Client Sends Init param (Version , port)
	Server sends Init repsonse (Config , status)
	if status is not ok , Client gets disconnected

	Client sends key data asap
	when server gets all the key data , it sends em to all Clients
	when client gets server's data ,  if the previus data is applied , it sends the new ones
	else , it waits for previus packed to be applied and sends current data( max 1 buffer)

*/
#pragma pack(1)
struct joy_init_resp
{
	u32 ratio;
	u32 mode;
	u32 players;
};

struct joy_init
{
	u32 Version;
	char Name[512];
	u32 port;
};

struct joy_substate
{
	u16 state;
	s8 jy;
	s8 jx;
	u8 r;
	u8 l;
};
struct joy_state
{
	u32 id;
	joy_substate  substates[8];
};
u32 state_size=0;
u32 ratio,port,players,mode;
struct connection_info
{
	joy_state info;
	u32 connected;
	bool has_data;
	//bool waiting;

	SOCKET connection;
};
/*volatile*/ connection_info states[4];
u32 c_status=0;
u32 p_status=0;
CRITICAL_SECTION lock_and_sync;
bool locked_cs=false;
void LOCK()
{
	if (!locked_cs)
	{
		locked_cs=true;
		InitializeCriticalSection(&lock_and_sync);
	}
	EnterCriticalSection(&lock_and_sync);
}
void UNLOCK()
{
	LeaveCriticalSection(&lock_and_sync);
}
void setups(SOCKET s)
{
	int flag = 1;
	u_long t;
	int result = setsockopt(s,            /* socket affected */
		IPPROTO_TCP,     /* set option at TCP level */
		TCP_NODELAY,     /* name of option */
		(char *) &flag,  /* the cast is historical
						 cruft */
						 sizeof(int));    /* length of option value */
	t=0;
	ioctlsocket (s,FIONBIO ,&t);
}

u32 __stdcall client_thread(SOCKET sock)
{
	volatile connection_info* con=0;
	joy_init in_init;
	joy_init_resp out_init;

	setups(sock);

	u32 rva=recv(sock,(char*)&in_init,sizeof(in_init),MSG_WAITALL);
	printf("Client version %X",in_init.Version);

	if (in_init.port<4 && states[in_init.port].connected==0)
	{
		
	}
	else
	{
		printf("Invalid id , or used %d\n",in_init.port);
		closesocket(sock);
		return -1;
	}
	con = &states[in_init.port];
	con->connected=1;
	con->connection=sock;

	out_init.ratio=ratio;
	out_init.mode=mode;//ingored atm
	out_init.players=players;
	send(con->connection,(char*)&out_init,sizeof(out_init),0);
	
	in_init.Name[511]=0;//make sure its terminated =P
	printf("Client connected using %s @ %d\n",in_init.Name,in_init.port);

	while(con->connected)
	{
		
		joy_state t;
		int rv=recv(con->connection,(char*)&t,state_size,MSG_WAITALL);
		
		LOCK();
		memcpy((void*)&con->info,&t,state_size);
		if (rv==-1)
		{
			con->has_data=0;
			con->connected=0;
			printf("%d dissconected\n",in_init.port);
			UNLOCK();
			return -1;
		}
		
		if (con->has_data)
			printf("con->has_data -> ONLY single buffer allowed \n");
		con->has_data=1;
		UNLOCK();
	}
	return 0;
}

u32 __stdcall send_thread(SOCKET sock)
{
	char temp_buffer[sizeof(joy_state)*4];

	while(	states[0].connected==0 || 
		states[1].connected==0 || 
		states[2].connected==0 || 
		states[3].connected==0)
	{
		Sleep(1);
	}

	while(1)
	{
		LOCK();
		while(	states[0].has_data==0 || 
			states[1].has_data==0 || 
			states[2].has_data==0 || 
			states[3].has_data==0)
		{
			UNLOCK();
			Sleep(1);
			LOCK();
		}

		u32 temp_buffer_sz=0;
		memset(temp_buffer,0,sizeof(temp_buffer));
		for(int i=0;i<players;i++)
		{
			if (states[i].connected==1)
			{
				memcpy(&temp_buffer[temp_buffer_sz],&states[i].info,state_size);
				states[i].has_data=0;
				temp_buffer_sz+=state_size;
			}
			else
			{
				//whatever
			}
		}
		//broadcast message !
		for(int i=0;i<players;i++)
		{
			u32 rvva=send(states[i].connection,temp_buffer,temp_buffer_sz,0);
		}
		UNLOCK();
	}
}
void server_main()
{
	
    WSADATA wsaData;
    SOCKET ListenSocket = INVALID_SOCKET,
           ClientSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    hints;
    int iResult, iSendResult;
    

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return ;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
	char ttt[8];
	sprintf(ttt,"%d",port);
    iResult = getaddrinfo(NULL, ttt, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return ;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return ;
    }
	setups(ListenSocket);

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return ;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return ;
    }
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)send_thread,(void*)ClientSocket,0,0);
	while(1)
	{
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());

			continue;
		}
		CreateThread(0,0,(LPTHREAD_START_ROUTINE)client_thread,(void*)ClientSocket,0,0);
	}
	closesocket(ListenSocket);
	WSACleanup();
}


int _tmain(int argc, _TCHAR* argv[])
{
	printf("hookjoy server v1.0.0\n");
	printf("Give init params in port,ratio,mode,players format :");
	fflush(stdout);
	scanf("%d,%d,%d,%d",&port,&ratio,&mode,&players);
	mode=mode?1:0;
	printf("\nIniting w/ port %d , ratio %d , mode %d and player count=%d\n",port,ratio,mode,players);

	if(mode==0)
		state_size=sizeof(joy_substate)+4;
	else
		state_size=ratio*sizeof(joy_substate)+4;

	if (state_size>sizeof(joy_state))
	{
		printf("invalid state size , use smaller ratio or mode0 (%d)\n",state_size);
		return -2;
	}
	printf("server -> client : %d b/frame , %d/sec@60 \nclient -> server : %d b/frame , %d/sec@60 \n",state_size*players,state_size*players*60/ratio,state_size,state_size*60/ratio);
	printf("Masked lag : %d @ 60fps;%d @30fps\n",ratio*16,ratio*32);
	for (int i=players;i<4;i++)
	{
		states[i].connected=2;
		states[i].has_data=1;
	}
	server_main();
	
	return 0;
}
