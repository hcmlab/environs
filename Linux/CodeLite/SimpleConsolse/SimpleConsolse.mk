##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=SimpleConsolse
ConfigurationName      :=Debug
WorkspacePath          := "/home/chi-tai/git/environs-dev/Linux/CodeLite"
ProjectPath            := "/home/chi-tai/git/environs-dev/Linux/CodeLite/SimpleConsolse"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=chi-tai
Date                   :=07/03/16
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
Preprocessors          :=$(PreprocessorSwitch)LINUX 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="SimpleConsolse.txt"
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
Objects0=$(IntermediateDirectory)/Common_Environs.Loader.cpp$(ObjectSuffix) $(IntermediateDirectory)/Simple.Console.CPP_Simple.Console.CPP.cpp$(ObjectSuffix) 



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
$(IntermediateDirectory)/Common_Environs.Loader.cpp$(ObjectSuffix): ../../../Common/Environs.Loader.cpp $(IntermediateDirectory)/Common_Environs.Loader.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Environs.Loader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Common_Environs.Loader.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Common_Environs.Loader.cpp$(DependSuffix): ../../../Common/Environs.Loader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Common_Environs.Loader.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Common_Environs.Loader.cpp$(DependSuffix) -MM "../../../Common/Environs.Loader.cpp"

$(IntermediateDirectory)/Common_Environs.Loader.cpp$(PreprocessSuffix): ../../../Common/Environs.Loader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Common_Environs.Loader.cpp$(PreprocessSuffix) "../../../Common/Environs.Loader.cpp"

$(IntermediateDirectory)/Simple.Console.CPP_Simple.Console.CPP.cpp$(ObjectSuffix): ../../../Windows/Simple.Console.CPP/Simple.Console.CPP.cpp $(IntermediateDirectory)/Simple.Console.CPP_Simple.Console.CPP.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Windows/Simple.Console.CPP/Simple.Console.CPP.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Simple.Console.CPP_Simple.Console.CPP.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Simple.Console.CPP_Simple.Console.CPP.cpp$(DependSuffix): ../../../Windows/Simple.Console.CPP/Simple.Console.CPP.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Simple.Console.CPP_Simple.Console.CPP.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Simple.Console.CPP_Simple.Console.CPP.cpp$(DependSuffix) -MM "../../../Windows/Simple.Console.CPP/Simple.Console.CPP.cpp"

$(IntermediateDirectory)/Simple.Console.CPP_Simple.Console.CPP.cpp$(PreprocessSuffix): ../../../Windows/Simple.Console.CPP/Simple.Console.CPP.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Simple.Console.CPP_Simple.Console.CPP.cpp$(PreprocessSuffix) "../../../Windows/Simple.Console.CPP/Simple.Console.CPP.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


