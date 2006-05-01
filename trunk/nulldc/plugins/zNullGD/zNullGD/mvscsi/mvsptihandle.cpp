#include <sstream>
#include "mvsptihandle.h"
using namespace std;

mvSPTI_Handle::mvSPTI_Handle(BYTE busNumber) {
	ostringstream tempBusName;
	tempBusName << "\\\\.\\Scsi" << (DWORD)busNumber << ":";

	handle_ = CreateFile(tempBusName.str().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(handle_ == INVALID_HANDLE_VALUE) { throw mvSPTI_InvalidHandle(); }
}

mvSPTI_Handle::mvSPTI_Handle(const CHAR* devName) {
	ostringstream tempDevName;
	tempDevName << "\\\\.\\" << devName;

	handle_ = CreateFile(tempDevName.str().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(handle_ == INVALID_HANDLE_VALUE) {
		handle_ = CreateFile(tempDevName.str().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(handle_ == INVALID_HANDLE_VALUE) { throw mvSPTI_InvalidHandle(); }
	}
}

mvSPTI_Handle& mvSPTI_Handle::operator=(mvSPTI_Handle& rhs) {
	if(handle_ != INVALID_HANDLE_VALUE) { CloseHandle(handle_); }
	handle_ = rhs.handle_;
	rhs.handle_ = INVALID_HANDLE_VALUE;
	return *this;
}

mvSPTI_Handle::mvSPTI_Handle(mvSPTI_Handle& rhs) {
	handle_ = rhs.handle_;
	rhs.handle_ = INVALID_HANDLE_VALUE;
}

mvSPTI_Handle::~mvSPTI_Handle() {
	if(handle_ != INVALID_HANDLE_VALUE) { CloseHandle(handle_); }
}