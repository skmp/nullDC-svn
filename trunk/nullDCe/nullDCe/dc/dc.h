#include "nulldce.h"

//Inits emulator into paused state.NOTE : this should NOT reset the state, you must call dcReset to do that
void dcInit();
//Resets emulator, only valid when emulator is paused
void dcReset(bool phys);
//Resumes emulation
void dcResume();
//Pauses emulation
void dcPause();
//Terminates emulation, will pause first if needed
void dcTerm();
