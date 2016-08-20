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
LIBNAME	= libEnvirons.so

SRCNAT		= ../Native/
SRC3RD		= ../3rd/
NATCORE		= $(SRCNAT)Core/
NATCAP		= $(SRCNAT)Capture/
NATDEC		= $(SRCNAT)Decoder/
NATDEV		= $(SRCNAT)Device/
NATDYL		= $(SRCNAT)DynLib/
NATENC		= $(SRCNAT)Encoder/
NATPORT		= $(SRCNAT)Portal/
NATREC		= $(SRCNAT)Recognizer/
NATREN		= $(SRCNAT)Renderer/

# For release builds add -DNDEBUG to suppress debug messages
# -Wno-ignored-attributes 



CFLAGS	= -std=gnu++11 -fPIC -Wall -DLINUX -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -I$(INCS) -I../Native/ -I../3rd -I../3rd/inc -I. -I/opt/ssl/include/ -I/opt/local/include -Wno-deprecated -Wno-unused-variable -fuse-cxa-atexit

ifndef DEBUG
   CFLAGS += -O3 -DNDEBUG
else
   CFLAGS += -g
endif

ifneq (,$(wildcard $(ISRASPBERYY)))
	CFLAGS += -DRASPBERRY
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
   CFLAGS += -Wno-unused-but-set-variable
endif

#LDFLAGS	= -Wextra -nostartfiles -shared -rdynamic -lm -lpthread -L/opt/local/lib -ldl $(EXLDF)
LDFLAGS	= -shared -rdynamic -lm -lpthread -ldl $(EXLDF)
#-llibstdc++ -lcurl -lcrypto -L/opt/local/lib 


OTHER_C_FLAGS = -isystem /opt/local/include 

SRC	= stdafx.cpp $(INCS)Mediator.cpp $(INCS)Environs.Crypt.cpp $(INCS)Environs.Crypt.Lib.cpp $(INCS)Environs.Crypt1.cpp 
SRC	+= $(INCS)Environs.Types.cpp $(INCS)Environs.Utils.cpp $(INCS)Tracer.cpp
SRC	+= $(INCS)Interfaces/IEnvirons.Base.cpp $(INCS)/Interfaces/IPortal.Encoder.cpp $(INCS)Interfaces/IPortal.Renderer.cpp
SRC	+= $(INCS)Interop/Export.cpp $(INCS)Interop/Threads.cpp $(INCS)Log.cpp $(INCS)Environs.Commit.cpp

SRC	+= $(SRCNAT)Environs.cpp $(SRCNAT)Environs.Cpp.cpp $(SRCNAT)Environs.Obj.cpp $(SRCNAT)Environs.Lib.cpp $(SRCNAT)Environs.Linux.cpp $(SRCNAT)Environs.Sensors.cpp
SRC	+= $(SRCNAT)Environs.Display.cpp $(SRCNAT)Environs.Lib.Display.cpp
SRC	+= $(NATDEV)Device.List.cpp $(NATDEV)Device.Instance.cpp

SRC	+= $(NATCORE)Byte.Buffer.cpp $(NATCORE)Callbacks.cpp $(NATCORE)Utils.cpp $(NATCORE)Array.List.cpp
SRC	+= $(NATCORE)Core.cpp $(NATCORE)Kernel.cpp $(NATCORE)Kernel.Display.cpp $(NATCORE)Kernel.Display.Linux.cpp
SRC	+= $(NATCORE)Mediator.Client.cpp $(NATCORE)Notifications.cpp $(NATCORE)Performance.Count.cpp $(NATCORE)Stunt.Request.cpp

SRC	+= $(SRC3RD)DynLib/Dyn.Lib.Crypto.cpp 

SRC	+= $(NATCAP)Cam.Base.cpp $(NATCORE)File.Instance.cpp $(NATCORE)Message.Instance.cpp $(INCS)Queue.Vector.cpp $(NATCORE)Queue.List.cpp

SRC	+= $(NATDEC)Decoder.Base.cpp $(NATDEV)Device.Linux.cpp $(NATCORE)Input.Handler.cpp 

SRC	+= $(NATDEV)Device.Base.cpp $(NATDEV)Devices.cpp $(NATDEV)Device.Display.cpp 

SRC	+= $(NATDYL)Dyn.OpenCL.cpp $(NATDYL)Dyn.Lib.UUID.cpp $(NATCORE)Wifi.Observer.cpp
SRC	+= $(NATCORE)Wifi.List.cpp $(NATCORE)Bt.Observer.cpp $(NATCORE)Bt.List.cpp

SRC	+= $(NATENC)Encoder.Pictures.cpp 

SRC	+= $(NATPORT)Portal.Device.cpp $(NATPORT)Portal.Instance.cpp $(NATPORT)Portal.Info.cpp

SRC	+= $(NATPORT)Portal.Receiver.cpp $(NATPORT)Portal.Generator.cpp $(NATPORT)Portal.Stream.cpp

SRC	+= $(NATREC)Gesture.Three.Finger.cpp $(NATREC)Mouse.Simulator.cpp $(NATREC)Recognizers.cpp

SRC	+= $(NATREN)Render.OpenCL.cpp $(NATCORE)Async.Worker.cpp 

OBJ   	= $(SRC:%.cpp=%.o)

all: install

new: clean all

pre-build:
	@../Tools/build.commit.sh
	@$(PDETECT)
	     
%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIBNAME): $(OBJ)
	$(CC) -o $(LIBNAME) $(OBJ) $(LDFLAGS) 
	
clean: pre-build
	-rm -f *.o $(LIBNAME) $(INCS)*.o $(SRC3RD)DynLib/*.o $(INCS)Interfaces/*.o $(INCS)Interop/*.o $(SRCNAT)*.o $(NATCORE)*.o $(NATDEV)*.o $(NATCAP)*.o $(NATDYL)*.o $(NATENC)*.o $(NATPORT)*.o $(NATREC)*.o $(NATDEC)*.o $(NATREN)*.o

install: pre-build $(LIBNAME)
	cp $(LIBNAME) ../bin64/libs/.
	

#prepare: 
#	@if [ -d "/opt/local/include/openssl" ]; then export CFLAGS += -I/opt/local/include/openssl; fi

	