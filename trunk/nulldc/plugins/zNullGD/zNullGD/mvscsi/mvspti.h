#ifndef _MVSPTI_H_
#define _MVSPTI_H_

#include <windows.h>
#include <devioctl.h>
#include <ntddscsi.h>

#include <map>
#include <vector>
#include <exception>
#include <boost/shared_ptr.hpp>

#include "mvscsi.h"
#include "mvsptihandle.h"

class mvSPTI_DevMapKey;
class mvSPTI_Bus;
class mvSPTI_Dev;

typedef boost::shared_ptr<mvSPTI_Bus> LPmvSPTI_Bus;
typedef boost::shared_ptr<mvSPTI_Dev> LPmvSPTI_Dev;

typedef std::vector<LPmvSPTI_Bus>           mvSPTI_BusList;
typedef boost::shared_ptr<mvSPTI_BusList> LPmvSPTI_BusList;

typedef std::map<mvSPTI_DevMapKey, LPmvSPTI_Dev> mvSPTI_DevMap;
typedef boost::shared_ptr<mvSPTI_DevMap>       LPmvSPTI_DevMap;

class mvSPTI_Check : public std::exception {};
class mvSPTI_Error : public std::exception {};

class mvSPTI_DevMapKey {
public:
	mvSPTI_DevMapKey(BYTE devPath, BYTE devTarget, BYTE devLun)
	: devPath_(devPath), devTarget_(devTarget), devLun_(devLun) {}

	friend bool operator < (const mvSPTI_DevMapKey& lhs, const mvSPTI_DevMapKey& rhs) {
		DWORD lhsKey = (lhs.devPath_ << 16) | (lhs.devTarget_ << 8) | lhs.devLun_;
		DWORD rhsKey = (rhs.devPath_ << 16) | (rhs.devTarget_ << 8) | rhs.devLun_;
		return lhsKey < rhsKey;
	}

private:
	BYTE devPath_;
	BYTE devTarget_;
	BYTE devLun_;
};

class mvSPTI_Bus {
public:
	mvSPTI_Bus(BYTE busNumber);
	virtual ~mvSPTI_Bus() {}

	VOID RescanBus(LPmvSCSI_Bus bus);
	VOID InquiryBus(LPmvSCSI_Bus bus) { *bus = busInfo_; }
	LPmvSPTI_Dev GetDevice(BYTE devPath, BYTE devTarget, BYTE devLun);

private:
	mvSPTI_Bus(const mvSPTI_Bus&);
	mvSPTI_Bus& operator=(const mvSPTI_Bus&);

	mvSPTI_DevMap GetDeviceMap();

	mvSCSI_Bus    busInfo_;
	mvSPTI_Handle busHandle_;
	mvSPTI_DevMap busDevMap_;
};

class mvSPTI_Dev {
public:
	mvSPTI_Dev(BYTE devBus, BYTE devPath, BYTE devTarget, BYTE devLun, BYTE devType, DWORD devTimeOut);
	virtual ~mvSPTI_Dev() {}

	VOID InquiryDev(LPmvSCSI_Dev dev) { *dev = devInfo_; }
	VOID SetDevTimeOut(LPmvSCSI_Dev dev) { devInfo_.devTimeOut = dev->devTimeOut; }
	VOID ExecCmd(LPmvSCSI_Cmd cmd);

private:
	mvSPTI_Dev(const mvSPTI_Dev&);
	mvSPTI_Dev& operator=(const mvSPTI_Dev&);

	virtual HANDLE GetHandle() = 0;

	mvSCSI_Dev devInfo_;
};

class mvSPTI_DevClaimed : public mvSPTI_Dev {
public:
	mvSPTI_DevClaimed(BYTE devBus, BYTE devPath, BYTE devTarget, BYTE devLun, BYTE devType, DWORD devTimeOut);

private:
	virtual HANDLE GetHandle() { return devHandle_; }
	mvSPTI_Handle devHandle_;
};

class mvSPTI_DevNotClaimed : public mvSPTI_Dev {
public:
	mvSPTI_DevNotClaimed(BYTE devBus, BYTE devPath, BYTE devTarget, BYTE devLun, BYTE devType, DWORD devTimeOut, HANDLE handle)
	: mvSPTI_Dev(devBus, devPath, devTarget, devLun, devType, devTimeOut), devHandle_(handle) {}

private:
	virtual HANDLE GetHandle() { return devHandle_; }
	HANDLE devHandle_;
};

#endif
