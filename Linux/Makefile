CC	= g++
# This must be set to the common code directory of Environs
INCS	= ../Common/
LIBNAME	= libEnvirons.so

SRCNAT		= ../Native/
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
CFLAGS	= -fPIC -O3 -DNDEBUG -Wall -DLINUX -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -g -I$(INCS) -I../Native/ -I../3rd/inc -I. -I/opt/ssl/include/ -I/opt/local/include -Wno-deprecated
LDFLAGS	= -Wextra -nostartfiles -shared -rdynamic -lm -lpthread -lcurses -lcurl -L/opt/local/lib -lcrypto -ldl
OTHER_C_FLAGS = -isystem /opt/local/include 

SRC	= stdafx.cpp $(INCS)Mediator.cpp $(INCS)Environs.Crypt.cpp $(INCS)Environs.Crypt1.cpp $(INCS)Environs.Types.cpp $(INCS)Environs.Utils.cpp
SRC	+= $(INCS)Interfaces/IEnvironsBase.cpp $(INCS)/Interfaces/IPortalEncoder.cpp $(INCS)Interfaces/IPortalRenderer.cpp
SRC	+= $(INCS)Interfaces/ITouchRecognizer.cpp
SRC	+= $(INCS)Interop/Export.cpp $(INCS)Interop/Threads.cpp

SRC	+= $(SRCNAT)Environs.cpp $(SRCNAT)Environs.Lib.cpp

SRC	+= $(NATCORE)ByteBuffer.cpp $(NATCORE)Callbacks.cpp $(NATCORE)Utils.cpp
SRC	+= $(NATCORE)Core.cpp $(NATCORE)Kernel.cpp $(NATCORE)Kernel.Display.cpp $(NATCORE)Kernel.Display.Linux.cpp
SRC	+= $(NATCORE)Mediator.Client.cpp $(NATCORE)Notifications.cpp $(NATCORE)Performance.Count.cpp $(NATCORE)Stunt.Request.cpp

SRC	+= $(NATCAP)Cam.Base.cpp 

SRC	+= $(NATDEC)Decoder.Base.cpp 

SRC	+= $(NATDEV)Device.Base.cpp $(NATDEV)Devices.cpp $(NATDEV)Device.Display.cpp 

SRC	+= $(NATDYL)Dyn.OpenCL.cpp 

SRC	+= $(NATENC)Encoder.Pictures.cpp 

SRC	+= $(NATPORT)Portal.Receiver.cpp $(NATPORT)Portal.Generator.cpp $(NATPORT)Portal.Stream.cpp

SRC	+= $(NATREC)Gesture.Three.Finger.cpp $(NATREC)Mouse.Simulator.cpp $(NATREC)Recognizers.cpp

SRC	+= $(NATREN)Render.OpenCL.cpp $(NATCORE)Async.Worker.cpp 

OBJ   	= $(SRC:%.cpp=%.o)
     
%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(LIBNAME) $(OBJ) $(LDFLAGS)
	
clean:
	rm -f *.o $(LIBNAME) $(INCS)*.o $(SRCNAT)*.o $(NATCORE)*.o $(NATCAP)*.o $(NATDYL)*.o $(NATENC)*.o $(NATPORT)*.o $(NATREC)*.o $(NATREN)*.o

#prepare: 
#	@if [ -d "/opt/local/include/openssl" ]; then export CFLAGS += -I/opt/local/include/openssl; fi

	