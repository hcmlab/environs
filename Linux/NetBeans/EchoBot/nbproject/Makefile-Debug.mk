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
	${OBJECTDIR}/_ext/ff80179a/Environs.Loader.o \
	${OBJECTDIR}/_ext/83386d8e/Echo.Bot.CPP.o


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
LDLIBSOPTIONS=-ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../../../bin64/EchoBot-Linux

../../../bin64/EchoBot-Linux: ${OBJECTFILES}
	${MKDIR} -p ../../../bin64
	${LINK.cc} -o ../../../bin64/EchoBot-Linux ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/ff80179a/Environs.Loader.o: ../../../Common/Environs.Loader.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/ff80179a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLINUX -I../../../Common -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/ff80179a/Environs.Loader.o ../../../Common/Environs.Loader.cpp

${OBJECTDIR}/_ext/83386d8e/Echo.Bot.CPP.o: ../../../Windows/Echo.Bot.CPP/Echo.Bot.CPP.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/83386d8e
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLINUX -I../../../Common -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/83386d8e/Echo.Bot.CPP.o ../../../Windows/Echo.Bot.CPP/Echo.Bot.CPP.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../../../bin64/EchoBot-Linux

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
