#include "mvspti.h"
using namespace std;

#if defined(_X86_)
	#define PAGE_SIZE 0x1000
#elif defined(_AMD64_)
	#define PAGE_SIZE 0x1000
#elif defined(_IA64_)
	#define PAGE_SIZE 0x2000
#else
	#define PAGE_SIZE 0x1000
#endif

typedef struct {
	SCSI_PASS_THROUGH_DIRECT sptd;
	DWORD alignmentDummy;
	BYTE  senseBuf[mvSCSI_MAX_SENSE_LEN];
} mvSPTI_SPTDWB, *PmvSCSI_SPTDWB, FAR *LPmvSCSI_SPTDWB;

mvSPTI_Bus::mvSPTI_Bus(BYTE busNumber)
: busHandle_(busNumber) {
	DWORD bytesReturnedIO = 0;
	IO_SCSI_CAPABILITIES capabilities = {0};
	if(!DeviceIoControl(busHandle_, IOCTL_SCSI_GET_CAPABILITIES, NULL, 0, &capabilities, sizeof(IO_SCSI_CAPABILITIES), &bytesReturnedIO, NULL)) { throw mvSPTI_Error(); }

	busInfo_.busNumber      = busNumber;
	busInfo_.busMaxTransfer = min(
		(capabilities.MaximumPhysicalPages - 1) * PAGE_SIZE,
		capabilities.MaximumTransferLength > 0 ? capabilities.MaximumTransferLength : 0xFFFFFFFF);
	busInfo_.busAlignmentMask = capabilities.AlignmentMask;

	busDevMap_ = GetDeviceMap();
}

VOID mvSPTI_Bus::RescanBus(LPmvSCSI_Bus bus) {
	DWORD bytesReturnedIO = 0;
	if(!DeviceIoControl(busHandle_, IOCTL_SCSI_RESCAN_BUS, NULL, 0, NULL, 0, &bytesReturnedIO, NULL)) { throw mvSPTI_Error(); }

	busDevMap_ = GetDeviceMap();
	InquiryBus(bus);
}

LPmvSPTI_Dev mvSPTI_Bus::GetDevice(BYTE devPath, BYTE devTarget, BYTE devLun) {
	mvSPTI_DevMap::const_iterator dev = busDevMap_.find(mvSPTI_DevMapKey(devPath, devTarget, devLun));
	if(dev == busDevMap_.end()) throw out_of_range("");
	return (*dev).second;
}

mvSPTI_DevMap mvSPTI_Bus::GetDeviceMap() {
	DWORD inquiryLength = PAGE_SIZE;
	DWORD alignmentOffset = 0;
	vector<BYTE> inquiryData(inquiryLength);
	mvSPTI_DevMap devMap;

	for(;;) {
		if(busInfo_.busAlignmentMask) {
			UINT_PTR  align = (UINT_PTR)busInfo_.busAlignmentMask;
			alignmentOffset = (DWORD)((((UINT_PTR)&inquiryData[0] + align) & ~align) - (UINT_PTR)&inquiryData[0]);
		} else {
			alignmentOffset = 0;
		}

		DWORD bytesReturnedIO = 0;
		if(DeviceIoControl(busHandle_, IOCTL_SCSI_GET_INQUIRY_DATA, NULL, 0, &inquiryData[alignmentOffset], (DWORD)inquiryData.size() - alignmentOffset, &bytesReturnedIO, NULL)) { break; }

		DWORD lastError = GetLastError();
		if(lastError != ERROR_INSUFFICIENT_BUFFER && lastError != ERROR_MORE_DATA) { throw mvSPTI_Error(); }

		inquiryLength += PAGE_SIZE;
		inquiryData.resize(inquiryLength);
	}

	PSCSI_ADAPTER_BUS_INFO adapterInfo = (PSCSI_ADAPTER_BUS_INFO)&inquiryData[alignmentOffset];

	for(int busIndex = 0; busIndex < adapterInfo->NumberOfBuses; ++busIndex) {
		PSCSI_INQUIRY_DATA devInquiryData = (PSCSI_INQUIRY_DATA)(&inquiryData[alignmentOffset] + adapterInfo->BusData[busIndex].InquiryDataOffset);
		for(int lunIndex = 0; lunIndex < adapterInfo->BusData[busIndex].NumberOfLogicalUnits; ++lunIndex) {
			if(devInquiryData->DeviceClaimed) {
				devMap[mvSPTI_DevMapKey(devInquiryData->PathId, devInquiryData->TargetId, devInquiryData->Lun)] = LPmvSPTI_Dev(new mvSPTI_DevClaimed(
					busInfo_.busNumber, devInquiryData->PathId, devInquiryData->TargetId, devInquiryData->Lun,
					devInquiryData->InquiryData[0] & mvSCSI_UNKNOWN, mvSCSI_DEFAULT_TIMEOUT));
			} else {
				devMap[mvSPTI_DevMapKey(devInquiryData->PathId, devInquiryData->TargetId, devInquiryData->Lun)] = LPmvSPTI_Dev(new mvSPTI_DevNotClaimed(
					busInfo_.busNumber, devInquiryData->PathId, devInquiryData->TargetId, devInquiryData->Lun,
					devInquiryData->InquiryData[0] & mvSCSI_UNKNOWN, mvSCSI_DEFAULT_TIMEOUT, busHandle_));
			}
			devInquiryData = (PSCSI_INQUIRY_DATA)(&inquiryData[alignmentOffset] + devInquiryData->NextInquiryDataOffset);
		}
	}

	return devMap;
}

mvSPTI_Dev::mvSPTI_Dev(BYTE devBus, BYTE devPath, BYTE devTarget, BYTE devLun, BYTE devType, DWORD devTimeOut) {
	devInfo_.devBus     = devBus;
	devInfo_.devPath    = devPath;
	devInfo_.devTarget  = devTarget;
	devInfo_.devLun     = devLun;
	devInfo_.devType    = devType;
	devInfo_.devTimeOut = devTimeOut;
}

VOID mvSPTI_Dev::ExecCmd(LPmvSCSI_Cmd cmd) {
	mvSPTI_SPTDWB sptdwb = {0};

	sptdwb.sptd.Length             = sizeof(SCSI_PASS_THROUGH_DIRECT);
	sptdwb.sptd.PathId             = devInfo_.devPath;
	sptdwb.sptd.TargetId           = devInfo_.devTarget;
	sptdwb.sptd.Lun                = devInfo_.devLun;
	sptdwb.sptd.TimeOutValue       = devInfo_.devTimeOut;
	sptdwb.sptd.CdbLength          = cmd->cmdCDBLen   > mvSCSI_MAX_CDB_LEN   ? mvSCSI_MAX_CDB_LEN   : cmd->cmdCDBLen;
	sptdwb.sptd.SenseInfoLength    = cmd->cmdSenseLen > mvSCSI_MAX_SENSE_LEN ? mvSCSI_MAX_SENSE_LEN : cmd->cmdSenseLen;
	sptdwb.sptd.SenseInfoOffset    = offsetof(mvSPTI_SPTDWB, senseBuf);
	sptdwb.sptd.DataIn             = cmd->cmdDataDir;
	sptdwb.sptd.DataTransferLength = cmd->cmdBufLen;
	sptdwb.sptd.DataBuffer         = cmd->cmdBufPtr;
	memcpy(sptdwb.sptd.Cdb, cmd->cmdCDB, cmd->cmdCDBLen > mvSCSI_MAX_CDB_LEN ? mvSCSI_MAX_CDB_LEN : cmd->cmdCDBLen);

	DWORD bytesReturnedIO = 0;
	if(!DeviceIoControl(GetHandle(), IOCTL_SCSI_PASS_THROUGH_DIRECT, &sptdwb, sizeof(mvSPTI_SPTDWB), &sptdwb, sizeof(mvSPTI_SPTDWB), &bytesReturnedIO, NULL)) { throw mvSPTI_Error(); }

	memcpy(cmd->cmdSense, sptdwb.senseBuf, cmd->cmdSenseLen > mvSCSI_MAX_SENSE_LEN ? mvSCSI_MAX_SENSE_LEN : cmd->cmdSenseLen);
	cmd->cmdStatus = sptdwb.sptd.ScsiStatus;

	if(sptdwb.sptd.ScsiStatus) { throw mvSPTI_Check(); }
}

mvSPTI_DevClaimed::mvSPTI_DevClaimed(BYTE devBus, BYTE devPath, BYTE devTarget, BYTE devLun, BYTE devType, DWORD devTimeOut)
: mvSPTI_Dev(devBus, devPath, devTarget, devLun, devType, devTimeOut) {
	DWORD length = PAGE_SIZE, offset = 0;
	vector<CHAR> devices(length);

	for(;;) {
		if(QueryDosDevice(NULL, &devices[0], (DWORD)devices.size())) { break; }

		DWORD lastError = GetLastError();
		if(lastError != ERROR_INSUFFICIENT_BUFFER && lastError != ERROR_MORE_DATA) { throw mvSPTI_Error(); }

		length += PAGE_SIZE;
		devices.resize(length);
	}

	while(length = (DWORD)strlen(&devices[offset])) {
		try {
            mvSPTI_Handle tempDevHandle((const CHAR*)&devices[offset]);

			DWORD bytesReturnedIO = 0;
			SCSI_ADDRESS scsiAddr = {0};
			BOOL resultIO = DeviceIoControl(tempDevHandle, IOCTL_SCSI_GET_ADDRESS, NULL, 0, &scsiAddr, sizeof(scsiAddr), &bytesReturnedIO, NULL);

			if(resultIO && bytesReturnedIO && devBus == scsiAddr.PortNumber && devPath == scsiAddr.PathId && devTarget == scsiAddr.TargetId && devLun == scsiAddr.Lun) {
				devHandle_ = tempDevHandle;
				break;
			}
		} catch(mvSPTI_InvalidHandle&) {
		}

		offset += length + 1;
	}

	if(devHandle_ == INVALID_HANDLE_VALUE) { throw mvSPTI_Error(); }
}
