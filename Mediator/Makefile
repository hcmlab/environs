PDETECT := ../Tools/platform.detect.sh

EXLDF := 

ifneq (,$(wildcard $(PDETECT)))
	CC := $(shell $(PDETECT) 1)
	EXLDF := $(shell $(PDETECT) 2)	
else
	CC	= g++
endif

ifeq ($(CC),)
	CC	= g++
endif

# This must be set to the common code directory of Environs
INCS	= ../Common/
3RD	= ../3rd/

ACFLAGS =
OUTF	= Mediator
ifeq ($(OS),Windows_NT)
    OUTF = Mediator.Win.exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
	OUTF = Mediator.Linux
    endif
    ifeq ($(UNAME_S),Darwin)
	OUTF = Mediator.OSX
	ACFLAGS = -I.
    endif
endif

export CC
export ENVCC
export PLATFORMDETECTED

# For release builds add -DNDEBUG to suppress debug messages

CFLAGS	= -std=gnu++11 -Wall -Wno-unused -DMEDIATORDAEMON -DLINUX -DMEDIATOR -g -I$(INCS) -I$(3RD) 
CFLAGS	+= -I/opt/ssl/include/ -I/opt/local/include -I$(3RD)inc -Wno-deprecated

LDFLAGS	= -lm -lpthread -lcurses -lcurl -ldl -L/opt/local/lib $(EXLDF)
# -lrt
#-lcrypto
OTHER_C_FLAGS = -isystem /opt/local/include

ifdef ENVCC
   CC = $(ENVCC)
endif

ifndef DEBUG
   CFLAGS += -Os -DNDEBUG
endif

CFLAGS += $(ACFLAGS)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
   CFLAGS += -Wno-unused-but-set-variable
endif

SRC	= main.cpp Mediator.Daemon.cpp stdafx.cpp $(INCS)Mediator.cpp $(INCS)Interop/Threads.cpp 
SRC	+= $(INCS)Environs.Types.cpp $(INCS)Environs.Utils.cpp $(INCS)Environs.Crypt.Lib.cpp $(INCS)Tracer.cpp
SRC	+= $(INCS)Environs.Crypt.cpp $(INCS)Queue.Vector.cpp $(INCS)Interop/Export.cpp $(3RD)DynLib/Dyn.Lib.Crypto.cpp

OBJ = $(SRC:%.cpp=%.o)

all: install

new: clean install
	
install: pre-build $(OUTF)

pre-build:
	@$(PDETECT)
	
$(OUTF): $(OBJ)
	$(CC) $(CFLAGS) -o $(OUTF) $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean: pre-build
	-rm *.o $(OUTF) $(INCS)*.o $(INCS)Interop/*.o $(3RD)DynLib/*.o

#prepare: 
#	@if [ -d "/opt/local/include/openssl" ]; then export CFLAGS += -I/opt/local/include/openssl; fi

	