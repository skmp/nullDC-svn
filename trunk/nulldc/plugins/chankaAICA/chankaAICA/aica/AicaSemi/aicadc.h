
void AICA_DoSamples(int nsamples);
void AICA_UpdateSlotReg(int s,int r);
void AICA_Init(int SampleRate);
void AICA_SetRAM(unsigned char *r,unsigned char *regs);
void AICA_SetBuffers(signed short *l,signed short *r);
unsigned int AICA_GetPlayPos(int slot);
unsigned int AICA_GetEnvState(int slot);
void AICA_UpdateDSP(unsigned int address);
