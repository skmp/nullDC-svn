#include <stdio.h>
#include <stdarg.h>
#include "pcap.h"
#include "pcap_io.h"

#include <Iphlpapi.h>

mac_address virtual_mac   = { 0x76, 0x6D, 0x61, 0x63, 0x30, 0x31 };
//mac_address virtual_mac   = { 0x6D, 0x76, 0x63, 0x61, 0x31, 0x30 };
mac_address broadcast_mac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

ip_address virtual_ip = { 192, 168, 2, 4};

pcap_t *adhandle;
int pcap_io_running=0;

char errbuf[PCAP_ERRBUF_SIZE];

char namebuff[256];

FILE*packet_log;

pcap_dumper_t *dump_pcap;

mac_address host_mac = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Fetches the MAC address and prints it
int GetMACaddress(char *adapter, mac_address* addr)
{
	static IP_ADAPTER_INFO AdapterInfo[128];       // Allocate information
												 // for up to 128 NICs
	static PIP_ADAPTER_INFO pAdapterInfo;
	ULONG dwBufLen = sizeof(AdapterInfo);	// Save memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
	AdapterInfo,                 // [out] buffer to receive data
	&dwBufLen);                  // [in] size of receive data buffer
	if(dwStatus != ERROR_SUCCESS)    // Verify return value is
		return 0;                       // valid, no buffer overflow

	pAdapterInfo = AdapterInfo; // Contains pointer to
											   // current adapter info
	do {
		if(strcmp(pAdapterInfo->AdapterName,adapter+12)==0)
		{
			memcpy(addr,pAdapterInfo->Address,6);
			return 1;
		}

		pAdapterInfo = pAdapterInfo->Next;    // Progress through
	}
	while(pAdapterInfo);                    // Terminate if last adapter
	return 0;
}

int pcap_io_init(char *adapter)
{
	printf("WINPCAP: Opening adapter '%s'...",adapter);

	GetMACaddress(adapter,&host_mac);
		
	/* Open the adapter */
	if ((adhandle= pcap_open_live(adapter,	// name of the device
							 65536,			// portion of the packet to capture. 
											// 65536 grants that the whole packet will be captured on all the MACs.
							 1,				// promiscuous mode (nonzero means promiscuous)
							 1,			// read timeout
							 errbuf			// error buffer
							 )) == NULL)
	{
		printf("\nWINPCAP: Unable to open the adapter. %s is not supported by WinPcap\n", adapter);
		return -1;
	}

	if(pcap_setnonblock(adhandle,1,errbuf)==-1)
	{
		printf("WINPCAP: Error setting non-blocking mode. Default mode will be used.\n");
	}

	packet_log=fopen("logs/packet.log","w");

	dump_pcap = pcap_dump_open(adhandle,"logs/pkt_log.pcap");

	pcap_io_running=1;
	printf("WINPCAP: Ok.\n");
	return 0;
}

int gettimeofday (struct timeval *tv, void* tz)
{
  unsigned __int64 ns100; /*time since 1 Jan 1601 in 100ns units */

  GetSystemTimeAsFileTime((LPFILETIME)&ns100);
  tv->tv_usec = (long) ((ns100 / 10L) % 1000000L);
  tv->tv_sec = (long) ((ns100 - 116444736000000000L) / 10000000L);
  return (0);
} 

int pcap_io_send(void* packet, int plen)
{
	struct pcap_pkthdr ph;

	if(pcap_io_running<=0)
		return -1;
	printf("WINPCAP:  * pcap io: Sending %d byte packet.\n",plen);

	if(dump_pcap)
	{
		gettimeofday(&ph.ts,NULL);
		ph.caplen=plen;
		ph.len=plen;
		pcap_dump((u_char*)dump_pcap,&ph,(u_char*)packet);
	}

	if(packet_log)
	{
		int i=0;
		int n=0;

		fprintf(packet_log,"PACKET SEND: %d BYTES\n",plen);
		for(i=0,n=0;i<plen;i++)
		{
			fprintf(packet_log,"%02x",((unsigned char*)packet)[i]);
			n++;
			if(n==16)
			{
				fprintf(packet_log,"\n");
				n=0;
			}
			else
			fprintf(packet_log," ");
		}
		fprintf(packet_log,"\n");
	}

	if(mac_compare(((ethernet_header*)packet)->dst,broadcast_mac)==0)
	{
		static u_char pack[65536];
		memcpy(pack,packet,plen);

		((ethernet_header*)packet)->dst=host_mac;
		pcap_sendpacket(adhandle, pack, plen);
	}

	return pcap_sendpacket(adhandle, (const u_char*)packet, plen);
}

int pcap_io_recv(void* packet, int max_len)
{
	int res;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;

	if(pcap_io_running<=0)
		return -1;

	if((res = pcap_next_ex(adhandle, &header, &pkt_data)) > 0)
	{
		ethernet_header *ph=(ethernet_header*)pkt_data;
		if((mac_compare(ph->dst,virtual_mac)!=0)&&(mac_compare(ph->dst,broadcast_mac)!=0))
		{
			return 0;
		}

		memcpy(packet,pkt_data,header->len);

		if(dump_pcap)
			pcap_dump((u_char*)dump_pcap,header,(u_char*)packet);

		if(packet_log)
		{
			int i=0;
			int n=0;
			int plen=header->len;

			fprintf(packet_log,"PACKET RECV: %d BYTES\n",plen);
			for(i=0,n=0;i<plen;i++)
			{
				fprintf(packet_log,"%02x",((unsigned char*)packet)[i]);
				n++;
				if(n==16)
				{
					fprintf(packet_log,"\n");
					n=0;
				}
				else
				fprintf(packet_log," ");
			}
			fprintf(packet_log,"\n");
		}

		return header->len;
	}

	return -1;
}

void pcap_io_close()
{
	if(packet_log)
		fclose(packet_log);
	if(dump_pcap)
		pcap_dump_close(dump_pcap);
	pcap_close(adhandle);  
	pcap_io_running=0;
}

int pcap_io_get_dev_num()
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i=0;

	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		return 0;
	}
    
	d=alldevs;
    while(d!=NULL) {d=d->next; i++;}

	pcap_freealldevs(alldevs);

	return i;
}

char* pcap_io_get_dev_name(int num)
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i=0;

	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		return NULL;
	}
    
	d=alldevs;
    while(d!=NULL) {
		if(num==i)
		{
			strcpy(namebuff,d->name);
			pcap_freealldevs(alldevs);
			return namebuff;
		}
		d=d->next; i++;
	}

	pcap_freealldevs(alldevs);

	return NULL;
}

char* pcap_io_get_dev_desc(int num)
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i=0;

	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		return NULL;
	}
    
	d=alldevs;
    while(d!=NULL) {
		if(num==i)
		{
			strcpy(namebuff,d->description);
			pcap_freealldevs(alldevs);
			return namebuff;
		}
		d=d->next; i++;
	}

	pcap_freealldevs(alldevs);

	return NULL;
}
