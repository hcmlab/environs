##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=EchoBot
ConfigurationName      :=Debug
WorkspacePath          := "/home/chi-tai/git/environs-dev/Linux/CodeLite"
ProjectPath            := "/home/chi-tai/git/environs-dev/Linux/CodeLite/EchoBot"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=chi-tai
Date                   :=08/03/16
CodeLitePath           :="/home/chi-tai/.codelite"
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=../../../bin64/$(ProjectName)-Linux
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="EchoBot.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -ldl
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../../Common 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -std=c++11 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/Echo.Bot.CPP_Echo.Bot.CPP.cpp$(ObjectSuffix) $(IntermediateDirectory)/Common_Environs.Loader.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/Echo.Bot.CPP_Echo.Bot.CPP.cpp$(ObjectSuffix): ../../../Windows/Echo.Bot.CPP/Echo.Bot.CPP.cpp $(IntermediateDirectory)/Echo.Bot.CPP_Echo.Bot.CPP.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Windows/Echo.Bot.CPP/Echo.Bot.CPP.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Echo.Bot.CPP_Echo.Bot.CPP.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Echo.Bot.CPP_Echo.Bot.CPP.cpp$(DependSuffix): ../../../Windows/Echo.Bot.CPP/Echo.Bot.CPP.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Echo.Bot.CPP_Echo.Bot.CPP.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Echo.Bot.CPP_Echo.Bot.CPP.cpp$(DependSuffix) -MM "../../../Windows/Echo.Bot.CPP/Echo.Bot.CPP.cpp"

$(IntermediateDirectory)/Echo.Bot.CPP_Echo.Bot.CPP.cpp$(PreprocessSuffix): ../../../Windows/Echo.Bot.CPP/Echo.Bot.CPP.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Echo.Bot.CPP_Echo.Bot.CPP.cpp$(PreprocessSuffix) "../../../Windows/Echo.Bot.CPP/Echo.Bot.CPP.cpp"

$(IntermediateDirectory)/Common_Environs.Loader.cpp$(ObjectSuffix): ../../../Common/Environs.Loader.cpp $(IntermediateDirectory)/Common_Environs.Loader.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Environs.Loader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Common_Environs.Loader.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Common_Environs.Loader.cpp$(DependSuffix): ../../../Common/Environs.Loader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Common_Environs.Loader.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Common_Environs.Loader.cpp$(DependSuffix) -MM "../../../Common/Environs.Loader.cpp"

$(IntermediateDirectory)/Common_Environs.Loader.cpp$(PreprocessSuffix): ../../../Common/Environs.Loader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Common_Environs.Loader.cpp$(PreprocessSuffix) "../../../Common/Environs.Loader.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


