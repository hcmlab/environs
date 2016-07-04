PDETECT := ../../Tools/platform.detect.sh

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
INCS	= ../../Common
SRCNAT	= ../../Windows/Echo.Bot.CPP

ACFLAGS =
OUTF	= Echo.Bot
ifeq ($(OS),Windows_NT)
    OUTF = Echo.Bot.exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		OUTF = EchoBot-Linux
    endif
    ifeq ($(UNAME_S),Darwin)
		OUTF = Echo.Bot.OSX
		ACFLAGS = -I.
    endif
endif

# For release builds add -DNDEBUG to suppress debug messages
# -Wno-ignored-attributes 

CFLAGS	= -std=gnu++11 -Wall -DLINUX -DDISPLAYDEVICE -Wno-unused -I$(INCS) -Wno-deprecated -Wno-unused-variable -fuse-cxa-atexit
#-fno-use-cxa-atexit

ifndef DEBUG
   CFLAGS += -O3 -DNDEBUG
else
   CFLAGS += -g
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
   CFLAGS += -Wno-unused-but-set-variable
endif

LDFLAGS	= -lm -lpthread -ldl $(EXLDF)
# -lcurl -lcrypto  -lcurses -L/opt/local/lib 

OTHER_C_FLAGS = -isystem /opt/local/include 

SRC	= $(SRCNAT)/stdafx.cpp $(SRCNAT)/Echo.Bot.CPP.cpp $(INCS)/Environs.Loader.cpp $(INCS)/Interop/Threads.cpp

OBJ   	= $(SRC:%.cpp=%.o)

all: install

new: clean install
	
install: pre-build $(OUTF)
	cp $(OUTF) ../../bin64/.
	@echo ""
	@echo "Hint: cd ../../bin64 and start: $(OUTF) \n"

pre-build:
	@$(PDETECT)
	
all: install
	     
%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUTF): $(OBJ)
	$(CC) -o $(OUTF) $(OBJ) $(LDFLAGS) 
	
clean: pre-build
	-rm -f *.o $(OUTF) $(INCS)/*.o $(INCS)/Interop/*.o $(SRCNAT)/*.o $(INCS)/../bin64/$(OUTF)

	
	


	