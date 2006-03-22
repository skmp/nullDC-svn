#include "plugin_types.h"

INLINE bool operator ==(VersionNumber& x,  const u32 y)
{
	return x.full==y;
}
INLINE void VersionNumber::operator= ( const u32 y)
{
	full=y;
	//return this;
}