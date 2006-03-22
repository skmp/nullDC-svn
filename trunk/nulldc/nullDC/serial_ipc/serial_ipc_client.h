#include "types.h"

void PrintSerialIPUsage(int argc, char *argv[]);
void WriteSerial(u8 data);
bool PendingSerialData();
s32 ReadSerial();
float GetRxSpeed();
float GetTxSpeed();
//this is a fake dma handler
//to initiate it write size to 0
//pointer to 4
//0xdeadc0de to 8
void WriteBlockSerial(u8* blk,u32 size,u8* sum);