#ifndef _MVSPTIHANDLE_
#define _MVSPTIHANDLE_

#include <windows.h>
#include <exception>

class mvSPTI_InvalidHandle : public std::exception {};

class mvSPTI_Handle {
public:
	mvSPTI_Handle(HANDLE handle = INVALID_HANDLE_VALUE) : handle_(handle) {}
	mvSPTI_Handle(BYTE busNumber);
	mvSPTI_Handle(const CHAR* devName);
	mvSPTI_Handle(mvSPTI_Handle& rhs);
	~mvSPTI_Handle();

	mvSPTI_Handle& operator=(mvSPTI_Handle& rhs);
	operator HANDLE() { return handle_; }

private:
	HANDLE handle_;
};

#endif
