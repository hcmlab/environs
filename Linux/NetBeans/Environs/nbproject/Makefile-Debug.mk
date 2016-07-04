#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/450dd325/Dyn.Lib.Crypto.o \
	${OBJECTDIR}/_ext/ff80179a/Environs.Commit.o \
	${OBJECTDIR}/_ext/ff80179a/Environs.Crypt.o \
	${OBJECTDIR}/_ext/ff80179a/Environs.Crypt1.o \
	${OBJECTDIR}/_ext/ff80179a/Environs.Types.o \
	${OBJECTDIR}/_ext/ff80179a/Environs.Utils.o \
	${OBJECTDIR}/_ext/54bfec0f/IEnvirons.Base.o \
	${OBJECTDIR}/_ext/54bfec0f/IPortal.Encoder.o \
	${OBJECTDIR}/_ext/54bfec0f/IPortal.Renderer.o \
	${OBJECTDIR}/_ext/c0e95e28/Export.o \
	${OBJECTDIR}/_ext/c0e95e28/Threads.o \
	${OBJECTDIR}/_ext/ff80179a/Log.o \
	${OBJECTDIR}/_ext/ff80179a/Mediator.o \
	${OBJECTDIR}/_ext/d490e268/Array.List.o \
	${OBJECTDIR}/_ext/d490e268/Async.Worker.o \
	${OBJECTDIR}/_ext/d490e268/Byte.Buffer.o \
	${OBJECTDIR}/_ext/d490e268/Callbacks.o \
	${OBJECTDIR}/_ext/d490e268/Core.o \
	${OBJECTDIR}/_ext/d490e268/File.Instance.o \
	${OBJECTDIR}/_ext/d490e268/Input.Handler.o \
	${OBJECTDIR}/_ext/d490e268/Kernel.Display.Linux.o \
	${OBJECTDIR}/_ext/d490e268/Kernel.Display.o \
	${OBJECTDIR}/_ext/d490e268/Kernel.Mobile.Platform.o \
	${OBJECTDIR}/_ext/d490e268/Kernel.Mobile.o \
	${OBJECTDIR}/_ext/d490e268/Kernel.Windows.o \
	${OBJECTDIR}/_ext/d490e268/Kernel.o \
	${OBJECTDIR}/_ext/d490e268/Mediator.Client.o \
	${OBJECTDIR}/_ext/d490e268/Message.Instance.o \
	${OBJECTDIR}/_ext/d490e268/Notifications.o \
	${OBJECTDIR}/_ext/d490e268/Performance.Count.o \
	${OBJECTDIR}/_ext/d490e268/Queue.List.o \
	${OBJECTDIR}/_ext/d490e268/Queue.Vector.o \
	${OBJECTDIR}/_ext/d490e268/Stunt.Request.o \
	${OBJECTDIR}/_ext/d490e268/Touch.Source.o \
	${OBJECTDIR}/_ext/d490e268/Utils.o \
	${OBJECTDIR}/_ext/f50bc2df/Device.Android.o \
	${OBJECTDIR}/_ext/f50bc2df/Device.Base.o \
	${OBJECTDIR}/_ext/f50bc2df/Device.Display.Win.o \
	${OBJECTDIR}/_ext/f50bc2df/Device.Display.o \
	${OBJECTDIR}/_ext/f50bc2df/Device.Instance.o \
	${OBJECTDIR}/_ext/f50bc2df/Device.Linux.o \
	${OBJECTDIR}/_ext/f50bc2df/Device.List.o \
	${OBJECTDIR}/_ext/f50bc2df/Device.Mobile.o \
	${OBJECTDIR}/_ext/f50bc2df/Devices.o \
	${OBJECTDIR}/_ext/f62189d5/Dyn.OpenCL.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.Cpp.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.Display.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.Lib.Display.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.Lib.Mobile.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.Lib.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.Linux.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.Mobile.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.Obj.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.Sensors.o \
	${OBJECTDIR}/_ext/11833dc6/Environs.o \
	${OBJECTDIR}/_ext/a112cf5/Portal.Device.o \
	${OBJECTDIR}/_ext/a112cf5/Portal.Generator.o \
	${OBJECTDIR}/_ext/a112cf5/Portal.Info.o \
	${OBJECTDIR}/_ext/a112cf5/Portal.Instance.o \
	${OBJECTDIR}/_ext/a112cf5/Portal.Receiver.o \
	${OBJECTDIR}/_ext/a112cf5/Portal.Stream.o \
	${OBJECTDIR}/_ext/818ae70c/Render.OpenCL.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-ldl -lpthread `pkg-config --libs uuid`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../../../bin64/libs/libEnvirons.${CND_DLIB_EXT}

../../../bin64/libs/libEnvirons.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ../../../bin64/libs
	${LINK.cc} -o ../../../bin64/libs/libEnvirons.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/_ext/450dd325/Dyn.Lib.Crypto.o: ../../../3rd/DynLib/Dyn.Lib.Crypto.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/450dd325
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/450dd325/Dyn.Lib.Crypto.o ../../../3rd/DynLib/Dyn.Lib.Crypto.cpp

${OBJECTDIR}/_ext/ff80179a/Environs.Commit.o: ../../../Common/Environs.Commit.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/ff80179a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/ff80179a/Environs.Commit.o ../../../Common/Environs.Commit.cpp

${OBJECTDIR}/_ext/ff80179a/Environs.Crypt.o: ../../../Common/Environs.Crypt.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/ff80179a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/ff80179a/Environs.Crypt.o ../../../Common/Environs.Crypt.cpp

${OBJECTDIR}/_ext/ff80179a/Environs.Crypt1.o: ../../../Common/Environs.Crypt1.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/ff80179a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/ff80179a/Environs.Crypt1.o ../../../Common/Environs.Crypt1.cpp

${OBJECTDIR}/_ext/ff80179a/Environs.Types.o: ../../../Common/Environs.Types.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/ff80179a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/ff80179a/Environs.Types.o ../../../Common/Environs.Types.cpp

${OBJECTDIR}/_ext/ff80179a/Environs.Utils.o: ../../../Common/Environs.Utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/ff80179a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/ff80179a/Environs.Utils.o ../../../Common/Environs.Utils.cpp

${OBJECTDIR}/_ext/54bfec0f/IEnvirons.Base.o: ../../../Common/Interfaces/IEnvirons.Base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/54bfec0f
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/54bfec0f/IEnvirons.Base.o ../../../Common/Interfaces/IEnvirons.Base.cpp

${OBJECTDIR}/_ext/54bfec0f/IPortal.Encoder.o: ../../../Common/Interfaces/IPortal.Encoder.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/54bfec0f
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/54bfec0f/IPortal.Encoder.o ../../../Common/Interfaces/IPortal.Encoder.cpp

${OBJECTDIR}/_ext/54bfec0f/IPortal.Renderer.o: ../../../Common/Interfaces/IPortal.Renderer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/54bfec0f
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/54bfec0f/IPortal.Renderer.o ../../../Common/Interfaces/IPortal.Renderer.cpp

${OBJECTDIR}/_ext/c0e95e28/Export.o: ../../../Common/Interop/Export.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/c0e95e28
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/c0e95e28/Export.o ../../../Common/Interop/Export.cpp

${OBJECTDIR}/_ext/c0e95e28/Threads.o: ../../../Common/Interop/Threads.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/c0e95e28
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/c0e95e28/Threads.o ../../../Common/Interop/Threads.cpp

${OBJECTDIR}/_ext/ff80179a/Log.o: ../../../Common/Log.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/ff80179a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/ff80179a/Log.o ../../../Common/Log.cpp

${OBJECTDIR}/_ext/ff80179a/Mediator.o: ../../../Common/Mediator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/ff80179a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/ff80179a/Mediator.o ../../../Common/Mediator.cpp

${OBJECTDIR}/_ext/d490e268/Array.List.o: ../../../Native/Core/Array.List.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Array.List.o ../../../Native/Core/Array.List.cpp

${OBJECTDIR}/_ext/d490e268/Async.Worker.o: ../../../Native/Core/Async.Worker.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Async.Worker.o ../../../Native/Core/Async.Worker.cpp

${OBJECTDIR}/_ext/d490e268/Byte.Buffer.o: ../../../Native/Core/Byte.Buffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Byte.Buffer.o ../../../Native/Core/Byte.Buffer.cpp

${OBJECTDIR}/_ext/d490e268/Callbacks.o: ../../../Native/Core/Callbacks.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Callbacks.o ../../../Native/Core/Callbacks.cpp

${OBJECTDIR}/_ext/d490e268/Core.o: ../../../Native/Core/Core.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Core.o ../../../Native/Core/Core.cpp

${OBJECTDIR}/_ext/d490e268/File.Instance.o: ../../../Native/Core/File.Instance.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/File.Instance.o ../../../Native/Core/File.Instance.cpp

${OBJECTDIR}/_ext/d490e268/Input.Handler.o: ../../../Native/Core/Input.Handler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Input.Handler.o ../../../Native/Core/Input.Handler.cpp

${OBJECTDIR}/_ext/d490e268/Kernel.Display.Linux.o: ../../../Native/Core/Kernel.Display.Linux.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Kernel.Display.Linux.o ../../../Native/Core/Kernel.Display.Linux.cpp

${OBJECTDIR}/_ext/d490e268/Kernel.Display.o: ../../../Native/Core/Kernel.Display.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Kernel.Display.o ../../../Native/Core/Kernel.Display.cpp

${OBJECTDIR}/_ext/d490e268/Kernel.Mobile.Platform.o: ../../../Native/Core/Kernel.Mobile.Platform.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Kernel.Mobile.Platform.o ../../../Native/Core/Kernel.Mobile.Platform.cpp

${OBJECTDIR}/_ext/d490e268/Kernel.Mobile.o: ../../../Native/Core/Kernel.Mobile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Kernel.Mobile.o ../../../Native/Core/Kernel.Mobile.cpp

${OBJECTDIR}/_ext/d490e268/Kernel.Windows.o: ../../../Native/Core/Kernel.Windows.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Kernel.Windows.o ../../../Native/Core/Kernel.Windows.cpp

${OBJECTDIR}/_ext/d490e268/Kernel.o: ../../../Native/Core/Kernel.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Kernel.o ../../../Native/Core/Kernel.cpp

${OBJECTDIR}/_ext/d490e268/Mediator.Client.o: ../../../Native/Core/Mediator.Client.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Mediator.Client.o ../../../Native/Core/Mediator.Client.cpp

${OBJECTDIR}/_ext/d490e268/Message.Instance.o: ../../../Native/Core/Message.Instance.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Message.Instance.o ../../../Native/Core/Message.Instance.cpp

${OBJECTDIR}/_ext/d490e268/Notifications.o: ../../../Native/Core/Notifications.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Notifications.o ../../../Native/Core/Notifications.cpp

${OBJECTDIR}/_ext/d490e268/Performance.Count.o: ../../../Native/Core/Performance.Count.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Performance.Count.o ../../../Native/Core/Performance.Count.cpp

${OBJECTDIR}/_ext/d490e268/Queue.List.o: ../../../Native/Core/Queue.List.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Queue.List.o ../../../Native/Core/Queue.List.cpp

${OBJECTDIR}/_ext/d490e268/Queue.Vector.o: ../../../Native/Core/Queue.Vector.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Queue.Vector.o ../../../Native/Core/Queue.Vector.cpp

${OBJECTDIR}/_ext/d490e268/Stunt.Request.o: ../../../Native/Core/Stunt.Request.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Stunt.Request.o ../../../Native/Core/Stunt.Request.cpp

${OBJECTDIR}/_ext/d490e268/Touch.Source.o: ../../../Native/Core/Touch.Source.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Touch.Source.o ../../../Native/Core/Touch.Source.cpp

${OBJECTDIR}/_ext/d490e268/Utils.o: ../../../Native/Core/Utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/d490e268
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d490e268/Utils.o ../../../Native/Core/Utils.cpp

${OBJECTDIR}/_ext/f50bc2df/Device.Android.o: ../../../Native/Device/Device.Android.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f50bc2df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f50bc2df/Device.Android.o ../../../Native/Device/Device.Android.cpp

${OBJECTDIR}/_ext/f50bc2df/Device.Base.o: ../../../Native/Device/Device.Base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f50bc2df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f50bc2df/Device.Base.o ../../../Native/Device/Device.Base.cpp

${OBJECTDIR}/_ext/f50bc2df/Device.Display.Win.o: ../../../Native/Device/Device.Display.Win.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f50bc2df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f50bc2df/Device.Display.Win.o ../../../Native/Device/Device.Display.Win.cpp

${OBJECTDIR}/_ext/f50bc2df/Device.Display.o: ../../../Native/Device/Device.Display.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f50bc2df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f50bc2df/Device.Display.o ../../../Native/Device/Device.Display.cpp

${OBJECTDIR}/_ext/f50bc2df/Device.Instance.o: ../../../Native/Device/Device.Instance.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f50bc2df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f50bc2df/Device.Instance.o ../../../Native/Device/Device.Instance.cpp

${OBJECTDIR}/_ext/f50bc2df/Device.Linux.o: ../../../Native/Device/Device.Linux.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f50bc2df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f50bc2df/Device.Linux.o ../../../Native/Device/Device.Linux.cpp

${OBJECTDIR}/_ext/f50bc2df/Device.List.o: ../../../Native/Device/Device.List.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f50bc2df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f50bc2df/Device.List.o ../../../Native/Device/Device.List.cpp

${OBJECTDIR}/_ext/f50bc2df/Device.Mobile.o: ../../../Native/Device/Device.Mobile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f50bc2df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f50bc2df/Device.Mobile.o ../../../Native/Device/Device.Mobile.cpp

${OBJECTDIR}/_ext/f50bc2df/Devices.o: ../../../Native/Device/Devices.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f50bc2df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f50bc2df/Devices.o ../../../Native/Device/Devices.cpp

${OBJECTDIR}/_ext/f62189d5/Dyn.OpenCL.o: ../../../Native/DynLib/Dyn.OpenCL.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/f62189d5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/f62189d5/Dyn.OpenCL.o ../../../Native/DynLib/Dyn.OpenCL.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.Cpp.o: ../../../Native/Environs.Cpp.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.Cpp.o ../../../Native/Environs.Cpp.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.Display.o: ../../../Native/Environs.Display.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.Display.o ../../../Native/Environs.Display.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.Lib.Display.o: ../../../Native/Environs.Lib.Display.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.Lib.Display.o ../../../Native/Environs.Lib.Display.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.Lib.Mobile.o: ../../../Native/Environs.Lib.Mobile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.Lib.Mobile.o ../../../Native/Environs.Lib.Mobile.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.Lib.o: ../../../Native/Environs.Lib.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.Lib.o ../../../Native/Environs.Lib.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.Linux.o: ../../../Native/Environs.Linux.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.Linux.o ../../../Native/Environs.Linux.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.Mobile.o: ../../../Native/Environs.Mobile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.Mobile.o ../../../Native/Environs.Mobile.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.Obj.o: ../../../Native/Environs.Obj.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.Obj.o ../../../Native/Environs.Obj.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.Sensors.o: ../../../Native/Environs.Sensors.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.Sensors.o ../../../Native/Environs.Sensors.cpp

${OBJECTDIR}/_ext/11833dc6/Environs.o: ../../../Native/Environs.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/11833dc6
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/11833dc6/Environs.o ../../../Native/Environs.cpp

${OBJECTDIR}/_ext/a112cf5/Portal.Device.o: ../../../Native/Portal/Portal.Device.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/a112cf5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a112cf5/Portal.Device.o ../../../Native/Portal/Portal.Device.cpp

${OBJECTDIR}/_ext/a112cf5/Portal.Generator.o: ../../../Native/Portal/Portal.Generator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/a112cf5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a112cf5/Portal.Generator.o ../../../Native/Portal/Portal.Generator.cpp

${OBJECTDIR}/_ext/a112cf5/Portal.Info.o: ../../../Native/Portal/Portal.Info.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/a112cf5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a112cf5/Portal.Info.o ../../../Native/Portal/Portal.Info.cpp

${OBJECTDIR}/_ext/a112cf5/Portal.Instance.o: ../../../Native/Portal/Portal.Instance.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/a112cf5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a112cf5/Portal.Instance.o ../../../Native/Portal/Portal.Instance.cpp

${OBJECTDIR}/_ext/a112cf5/Portal.Receiver.o: ../../../Native/Portal/Portal.Receiver.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/a112cf5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a112cf5/Portal.Receiver.o ../../../Native/Portal/Portal.Receiver.cpp

${OBJECTDIR}/_ext/a112cf5/Portal.Stream.o: ../../../Native/Portal/Portal.Stream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/a112cf5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a112cf5/Portal.Stream.o ../../../Native/Portal/Portal.Stream.cpp

${OBJECTDIR}/_ext/818ae70c/Render.OpenCL.o: ../../../Native/Renderer/Render.OpenCL.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/818ae70c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DDISPLAYDEVICE -DENVIRONS_CORE_LIB -DLINUX -I../../../Common -I../../../3rd/inc -I../../../3rd -I../../../Native `pkg-config --cflags uuid` -std=c++11  -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/818ae70c/Render.OpenCL.o ../../../Native/Renderer/Render.OpenCL.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../../../bin64/libs/libEnvirons.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
