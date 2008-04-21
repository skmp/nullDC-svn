TARGET = ndcepsp
OBJS = ./nullDCe/dc/dc.o ./nullDCe/dc/mem/mmap_arm7.o ./nullDCe/dc/mem/mmap_sh4.o ./nullDCe/dc/sh4/sh4.o ./nullDCe/Debug/ioif.o ./nullDCe/main.o ./nullDCe/types.o callbacks.o vram.o
INCDIR = ./nullDCe/
CFLAGS = -G0 -Wall -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lpspgu -lm -lpsprtc

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = nullDCe/psp v0

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
