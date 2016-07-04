##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Environs
ConfigurationName      :=Debug
WorkspacePath          := "/home/chi-tai/git/environs-dev/Linux/CodeLite"
ProjectPath            := "/home/chi-tai/git/environs-dev/Linux/CodeLite/Environs"
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
OutputFile             :=../../../bin64/libs/lib$(ProjectName).so
Preprocessors          :=$(PreprocessorSwitch)DEBUG $(PreprocessorSwitch)ENVIRONS_CORE_LIB $(PreprocessorSwitch)LINUX $(PreprocessorSwitch)DISPLAYDEVICE 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="Environs.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -rdynamic -lm -lpthread
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../../Common $(IncludeSwitch)../../../3rd $(IncludeSwitch)../../../3rd/inc $(IncludeSwitch)../../../Native 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)dl $(LibrarySwitch)uuid 
ArLibs                 :=  "dl" "uuid" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -std=c++11 -fPIC $(Preprocessors)
CFLAGS   :=  -g $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/Native_Environs.cpp$(ObjectSuffix) $(IntermediateDirectory)/Native_Environs.Cpp.cpp$(ObjectSuffix) $(IntermediateDirectory)/Native_Environs.Display.cpp$(ObjectSuffix) $(IntermediateDirectory)/Native_Environs.Lib.cpp$(ObjectSuffix) $(IntermediateDirectory)/Native_Environs.Lib.Display.cpp$(ObjectSuffix) $(IntermediateDirectory)/Native_Environs.Lib.Mobile.cpp$(ObjectSuffix) $(IntermediateDirectory)/Native_Environs.Linux.cpp$(ObjectSuffix) $(IntermediateDirectory)/Native_Environs.Mobile.cpp$(ObjectSuffix) $(IntermediateDirectory)/Native_Environs.Obj.cpp$(ObjectSuffix) $(IntermediateDirectory)/Native_Environs.Sensors.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/Common_Environs.Commit.cpp$(ObjectSuffix) $(IntermediateDirectory)/Common_Environs.Crypt.cpp$(ObjectSuffix) $(IntermediateDirectory)/Common_Environs.Crypt1.cpp$(ObjectSuffix) $(IntermediateDirectory)/Common_Environs.Types.cpp$(ObjectSuffix) $(IntermediateDirectory)/Common_Environs.Utils.cpp$(ObjectSuffix) $(IntermediateDirectory)/Common_Log.cpp$(ObjectSuffix) $(IntermediateDirectory)/Common_Mediator.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Array.List.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Async.Worker.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Byte.Buffer.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/Core_Callbacks.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Core.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_File.Instance.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Input.Handler.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Kernel.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Kernel.Display.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Kernel.Display.Linux.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Kernel.Mobile.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Kernel.Windows.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Mediator.Client.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/Core_Message.Instance.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Notifications.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Performance.Count.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Stunt.Request.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Utils.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Queue.List.cpp$(ObjectSuffix) $(IntermediateDirectory)/Core_Queue.Vector.cpp$(ObjectSuffix) $(IntermediateDirectory)/Device_Device.Android.cpp$(ObjectSuffix) $(IntermediateDirectory)/Device_Device.Base.cpp$(ObjectSuffix) $(IntermediateDirectory)/Device_Device.Display.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/Device_Device.Display.Win.cpp$(ObjectSuffix) 

Objects1=$(IntermediateDirectory)/Device_Device.Instance.cpp$(ObjectSuffix) $(IntermediateDirectory)/Device_Device.Linux.cpp$(ObjectSuffix) $(IntermediateDirectory)/Device_Device.List.cpp$(ObjectSuffix) $(IntermediateDirectory)/Device_Devices.cpp$(ObjectSuffix) $(IntermediateDirectory)/Portal_Portal.Device.cpp$(ObjectSuffix) $(IntermediateDirectory)/Portal_Portal.Generator.cpp$(ObjectSuffix) $(IntermediateDirectory)/Portal_Portal.Generator.Mobile.cpp$(ObjectSuffix) $(IntermediateDirectory)/Portal_Portal.Generator.Windows.cpp$(ObjectSuffix) $(IntermediateDirectory)/Portal_Portal.Info.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/Portal_Portal.Instance.cpp$(ObjectSuffix) $(IntermediateDirectory)/Portal_Portal.Receiver.Android.cpp$(ObjectSuffix) $(IntermediateDirectory)/Portal_Portal.Receiver.cpp$(ObjectSuffix) $(IntermediateDirectory)/Portal_Portal.Receiver.Windows.cpp$(ObjectSuffix) $(IntermediateDirectory)/Portal_Portal.Stream.cpp$(ObjectSuffix) $(IntermediateDirectory)/Interop_Export.cpp$(ObjectSuffix) $(IntermediateDirectory)/Interop_Threads.cpp$(ObjectSuffix) $(IntermediateDirectory)/DynLib_Dyn.OpenCL.cpp$(ObjectSuffix) $(IntermediateDirectory)/DynLib_Dyn.Lib.Crypto.cpp$(ObjectSuffix) $(IntermediateDirectory)/Renderer_Render.OpenCL.cpp$(ObjectSuffix) \
	



Objects=$(Objects0) $(Objects1) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	@echo $(Objects1) >> $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@$(MakeDirCommand) "/home/chi-tai/git/environs-dev/Linux/CodeLite/.build-debug"
	@echo rebuilt > "/home/chi-tai/git/environs-dev/Linux/CodeLite/.build-debug/Environs"

$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/Native_Environs.cpp$(ObjectSuffix): ../../../Native/Environs.cpp $(IntermediateDirectory)/Native_Environs.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.cpp$(DependSuffix): ../../../Native/Environs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.cpp$(DependSuffix) -MM "../../../Native/Environs.cpp"

$(IntermediateDirectory)/Native_Environs.cpp$(PreprocessSuffix): ../../../Native/Environs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.cpp$(PreprocessSuffix) "../../../Native/Environs.cpp"

$(IntermediateDirectory)/Native_Environs.Cpp.cpp$(ObjectSuffix): ../../../Native/Environs.Cpp.cpp $(IntermediateDirectory)/Native_Environs.Cpp.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.Cpp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.Cpp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.Cpp.cpp$(DependSuffix): ../../../Native/Environs.Cpp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.Cpp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.Cpp.cpp$(DependSuffix) -MM "../../../Native/Environs.Cpp.cpp"

$(IntermediateDirectory)/Native_Environs.Cpp.cpp$(PreprocessSuffix): ../../../Native/Environs.Cpp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.Cpp.cpp$(PreprocessSuffix) "../../../Native/Environs.Cpp.cpp"

$(IntermediateDirectory)/Native_Environs.Display.cpp$(ObjectSuffix): ../../../Native/Environs.Display.cpp $(IntermediateDirectory)/Native_Environs.Display.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.Display.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.Display.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.Display.cpp$(DependSuffix): ../../../Native/Environs.Display.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.Display.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.Display.cpp$(DependSuffix) -MM "../../../Native/Environs.Display.cpp"

$(IntermediateDirectory)/Native_Environs.Display.cpp$(PreprocessSuffix): ../../../Native/Environs.Display.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.Display.cpp$(PreprocessSuffix) "../../../Native/Environs.Display.cpp"

$(IntermediateDirectory)/Native_Environs.Lib.cpp$(ObjectSuffix): ../../../Native/Environs.Lib.cpp $(IntermediateDirectory)/Native_Environs.Lib.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.Lib.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.Lib.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.Lib.cpp$(DependSuffix): ../../../Native/Environs.Lib.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.Lib.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.Lib.cpp$(DependSuffix) -MM "../../../Native/Environs.Lib.cpp"

$(IntermediateDirectory)/Native_Environs.Lib.cpp$(PreprocessSuffix): ../../../Native/Environs.Lib.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.Lib.cpp$(PreprocessSuffix) "../../../Native/Environs.Lib.cpp"

$(IntermediateDirectory)/Native_Environs.Lib.Display.cpp$(ObjectSuffix): ../../../Native/Environs.Lib.Display.cpp $(IntermediateDirectory)/Native_Environs.Lib.Display.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.Lib.Display.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.Lib.Display.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.Lib.Display.cpp$(DependSuffix): ../../../Native/Environs.Lib.Display.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.Lib.Display.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.Lib.Display.cpp$(DependSuffix) -MM "../../../Native/Environs.Lib.Display.cpp"

$(IntermediateDirectory)/Native_Environs.Lib.Display.cpp$(PreprocessSuffix): ../../../Native/Environs.Lib.Display.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.Lib.Display.cpp$(PreprocessSuffix) "../../../Native/Environs.Lib.Display.cpp"

$(IntermediateDirectory)/Native_Environs.Lib.Mobile.cpp$(ObjectSuffix): ../../../Native/Environs.Lib.Mobile.cpp $(IntermediateDirectory)/Native_Environs.Lib.Mobile.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.Lib.Mobile.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.Lib.Mobile.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.Lib.Mobile.cpp$(DependSuffix): ../../../Native/Environs.Lib.Mobile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.Lib.Mobile.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.Lib.Mobile.cpp$(DependSuffix) -MM "../../../Native/Environs.Lib.Mobile.cpp"

$(IntermediateDirectory)/Native_Environs.Lib.Mobile.cpp$(PreprocessSuffix): ../../../Native/Environs.Lib.Mobile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.Lib.Mobile.cpp$(PreprocessSuffix) "../../../Native/Environs.Lib.Mobile.cpp"

$(IntermediateDirectory)/Native_Environs.Linux.cpp$(ObjectSuffix): ../../../Native/Environs.Linux.cpp $(IntermediateDirectory)/Native_Environs.Linux.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.Linux.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.Linux.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.Linux.cpp$(DependSuffix): ../../../Native/Environs.Linux.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.Linux.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.Linux.cpp$(DependSuffix) -MM "../../../Native/Environs.Linux.cpp"

$(IntermediateDirectory)/Native_Environs.Linux.cpp$(PreprocessSuffix): ../../../Native/Environs.Linux.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.Linux.cpp$(PreprocessSuffix) "../../../Native/Environs.Linux.cpp"

$(IntermediateDirectory)/Native_Environs.Mobile.cpp$(ObjectSuffix): ../../../Native/Environs.Mobile.cpp $(IntermediateDirectory)/Native_Environs.Mobile.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.Mobile.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.Mobile.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.Mobile.cpp$(DependSuffix): ../../../Native/Environs.Mobile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.Mobile.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.Mobile.cpp$(DependSuffix) -MM "../../../Native/Environs.Mobile.cpp"

$(IntermediateDirectory)/Native_Environs.Mobile.cpp$(PreprocessSuffix): ../../../Native/Environs.Mobile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.Mobile.cpp$(PreprocessSuffix) "../../../Native/Environs.Mobile.cpp"

$(IntermediateDirectory)/Native_Environs.Obj.cpp$(ObjectSuffix): ../../../Native/Environs.Obj.cpp $(IntermediateDirectory)/Native_Environs.Obj.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.Obj.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.Obj.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.Obj.cpp$(DependSuffix): ../../../Native/Environs.Obj.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.Obj.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.Obj.cpp$(DependSuffix) -MM "../../../Native/Environs.Obj.cpp"

$(IntermediateDirectory)/Native_Environs.Obj.cpp$(PreprocessSuffix): ../../../Native/Environs.Obj.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.Obj.cpp$(PreprocessSuffix) "../../../Native/Environs.Obj.cpp"

$(IntermediateDirectory)/Native_Environs.Sensors.cpp$(ObjectSuffix): ../../../Native/Environs.Sensors.cpp $(IntermediateDirectory)/Native_Environs.Sensors.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Environs.Sensors.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Native_Environs.Sensors.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Native_Environs.Sensors.cpp$(DependSuffix): ../../../Native/Environs.Sensors.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Native_Environs.Sensors.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Native_Environs.Sensors.cpp$(DependSuffix) -MM "../../../Native/Environs.Sensors.cpp"

$(IntermediateDirectory)/Native_Environs.Sensors.cpp$(PreprocessSuffix): ../../../Native/Environs.Sensors.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Native_Environs.Sensors.cpp$(PreprocessSuffix) "../../../Native/Environs.Sensors.cpp"

$(IntermediateDirectory)/Common_Environs.Commit.cpp$(ObjectSuffix): ../../../Common/Environs.Commit.cpp $(IntermediateDirectory)/Common_Environs.Commit.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Environs.Commit.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Common_Environs.Commit.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Common_Environs.Commit.cpp$(DependSuffix): ../../../Common/Environs.Commit.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Common_Environs.Commit.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Common_Environs.Commit.cpp$(DependSuffix) -MM "../../../Common/Environs.Commit.cpp"

$(IntermediateDirectory)/Common_Environs.Commit.cpp$(PreprocessSuffix): ../../../Common/Environs.Commit.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Common_Environs.Commit.cpp$(PreprocessSuffix) "../../../Common/Environs.Commit.cpp"

$(IntermediateDirectory)/Common_Environs.Crypt.cpp$(ObjectSuffix): ../../../Common/Environs.Crypt.cpp $(IntermediateDirectory)/Common_Environs.Crypt.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Environs.Crypt.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Common_Environs.Crypt.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Common_Environs.Crypt.cpp$(DependSuffix): ../../../Common/Environs.Crypt.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Common_Environs.Crypt.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Common_Environs.Crypt.cpp$(DependSuffix) -MM "../../../Common/Environs.Crypt.cpp"

$(IntermediateDirectory)/Common_Environs.Crypt.cpp$(PreprocessSuffix): ../../../Common/Environs.Crypt.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Common_Environs.Crypt.cpp$(PreprocessSuffix) "../../../Common/Environs.Crypt.cpp"

$(IntermediateDirectory)/Common_Environs.Crypt1.cpp$(ObjectSuffix): ../../../Common/Environs.Crypt1.cpp $(IntermediateDirectory)/Common_Environs.Crypt1.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Environs.Crypt1.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Common_Environs.Crypt1.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Common_Environs.Crypt1.cpp$(DependSuffix): ../../../Common/Environs.Crypt1.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Common_Environs.Crypt1.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Common_Environs.Crypt1.cpp$(DependSuffix) -MM "../../../Common/Environs.Crypt1.cpp"

$(IntermediateDirectory)/Common_Environs.Crypt1.cpp$(PreprocessSuffix): ../../../Common/Environs.Crypt1.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Common_Environs.Crypt1.cpp$(PreprocessSuffix) "../../../Common/Environs.Crypt1.cpp"

$(IntermediateDirectory)/Common_Environs.Types.cpp$(ObjectSuffix): ../../../Common/Environs.Types.cpp $(IntermediateDirectory)/Common_Environs.Types.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Environs.Types.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Common_Environs.Types.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Common_Environs.Types.cpp$(DependSuffix): ../../../Common/Environs.Types.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Common_Environs.Types.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Common_Environs.Types.cpp$(DependSuffix) -MM "../../../Common/Environs.Types.cpp"

$(IntermediateDirectory)/Common_Environs.Types.cpp$(PreprocessSuffix): ../../../Common/Environs.Types.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Common_Environs.Types.cpp$(PreprocessSuffix) "../../../Common/Environs.Types.cpp"

$(IntermediateDirectory)/Common_Environs.Utils.cpp$(ObjectSuffix): ../../../Common/Environs.Utils.cpp $(IntermediateDirectory)/Common_Environs.Utils.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Environs.Utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Common_Environs.Utils.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Common_Environs.Utils.cpp$(DependSuffix): ../../../Common/Environs.Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Common_Environs.Utils.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Common_Environs.Utils.cpp$(DependSuffix) -MM "../../../Common/Environs.Utils.cpp"

$(IntermediateDirectory)/Common_Environs.Utils.cpp$(PreprocessSuffix): ../../../Common/Environs.Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Common_Environs.Utils.cpp$(PreprocessSuffix) "../../../Common/Environs.Utils.cpp"

$(IntermediateDirectory)/Common_Log.cpp$(ObjectSuffix): ../../../Common/Log.cpp $(IntermediateDirectory)/Common_Log.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Log.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Common_Log.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Common_Log.cpp$(DependSuffix): ../../../Common/Log.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Common_Log.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Common_Log.cpp$(DependSuffix) -MM "../../../Common/Log.cpp"

$(IntermediateDirectory)/Common_Log.cpp$(PreprocessSuffix): ../../../Common/Log.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Common_Log.cpp$(PreprocessSuffix) "../../../Common/Log.cpp"

$(IntermediateDirectory)/Common_Mediator.cpp$(ObjectSuffix): ../../../Common/Mediator.cpp $(IntermediateDirectory)/Common_Mediator.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Mediator.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Common_Mediator.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Common_Mediator.cpp$(DependSuffix): ../../../Common/Mediator.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Common_Mediator.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Common_Mediator.cpp$(DependSuffix) -MM "../../../Common/Mediator.cpp"

$(IntermediateDirectory)/Common_Mediator.cpp$(PreprocessSuffix): ../../../Common/Mediator.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Common_Mediator.cpp$(PreprocessSuffix) "../../../Common/Mediator.cpp"

$(IntermediateDirectory)/Core_Array.List.cpp$(ObjectSuffix): ../../../Native/Core/Array.List.cpp $(IntermediateDirectory)/Core_Array.List.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Array.List.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Array.List.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Array.List.cpp$(DependSuffix): ../../../Native/Core/Array.List.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Array.List.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Array.List.cpp$(DependSuffix) -MM "../../../Native/Core/Array.List.cpp"

$(IntermediateDirectory)/Core_Array.List.cpp$(PreprocessSuffix): ../../../Native/Core/Array.List.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Array.List.cpp$(PreprocessSuffix) "../../../Native/Core/Array.List.cpp"

$(IntermediateDirectory)/Core_Async.Worker.cpp$(ObjectSuffix): ../../../Native/Core/Async.Worker.cpp $(IntermediateDirectory)/Core_Async.Worker.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Async.Worker.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Async.Worker.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Async.Worker.cpp$(DependSuffix): ../../../Native/Core/Async.Worker.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Async.Worker.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Async.Worker.cpp$(DependSuffix) -MM "../../../Native/Core/Async.Worker.cpp"

$(IntermediateDirectory)/Core_Async.Worker.cpp$(PreprocessSuffix): ../../../Native/Core/Async.Worker.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Async.Worker.cpp$(PreprocessSuffix) "../../../Native/Core/Async.Worker.cpp"

$(IntermediateDirectory)/Core_Byte.Buffer.cpp$(ObjectSuffix): ../../../Native/Core/Byte.Buffer.cpp $(IntermediateDirectory)/Core_Byte.Buffer.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Byte.Buffer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Byte.Buffer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Byte.Buffer.cpp$(DependSuffix): ../../../Native/Core/Byte.Buffer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Byte.Buffer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Byte.Buffer.cpp$(DependSuffix) -MM "../../../Native/Core/Byte.Buffer.cpp"

$(IntermediateDirectory)/Core_Byte.Buffer.cpp$(PreprocessSuffix): ../../../Native/Core/Byte.Buffer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Byte.Buffer.cpp$(PreprocessSuffix) "../../../Native/Core/Byte.Buffer.cpp"

$(IntermediateDirectory)/Core_Callbacks.cpp$(ObjectSuffix): ../../../Native/Core/Callbacks.cpp $(IntermediateDirectory)/Core_Callbacks.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Callbacks.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Callbacks.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Callbacks.cpp$(DependSuffix): ../../../Native/Core/Callbacks.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Callbacks.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Callbacks.cpp$(DependSuffix) -MM "../../../Native/Core/Callbacks.cpp"

$(IntermediateDirectory)/Core_Callbacks.cpp$(PreprocessSuffix): ../../../Native/Core/Callbacks.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Callbacks.cpp$(PreprocessSuffix) "../../../Native/Core/Callbacks.cpp"

$(IntermediateDirectory)/Core_Core.cpp$(ObjectSuffix): ../../../Native/Core/Core.cpp $(IntermediateDirectory)/Core_Core.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Core.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Core.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Core.cpp$(DependSuffix): ../../../Native/Core/Core.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Core.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Core.cpp$(DependSuffix) -MM "../../../Native/Core/Core.cpp"

$(IntermediateDirectory)/Core_Core.cpp$(PreprocessSuffix): ../../../Native/Core/Core.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Core.cpp$(PreprocessSuffix) "../../../Native/Core/Core.cpp"

$(IntermediateDirectory)/Core_File.Instance.cpp$(ObjectSuffix): ../../../Native/Core/File.Instance.cpp $(IntermediateDirectory)/Core_File.Instance.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/File.Instance.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_File.Instance.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_File.Instance.cpp$(DependSuffix): ../../../Native/Core/File.Instance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_File.Instance.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_File.Instance.cpp$(DependSuffix) -MM "../../../Native/Core/File.Instance.cpp"

$(IntermediateDirectory)/Core_File.Instance.cpp$(PreprocessSuffix): ../../../Native/Core/File.Instance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_File.Instance.cpp$(PreprocessSuffix) "../../../Native/Core/File.Instance.cpp"

$(IntermediateDirectory)/Core_Input.Handler.cpp$(ObjectSuffix): ../../../Native/Core/Input.Handler.cpp $(IntermediateDirectory)/Core_Input.Handler.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Input.Handler.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Input.Handler.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Input.Handler.cpp$(DependSuffix): ../../../Native/Core/Input.Handler.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Input.Handler.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Input.Handler.cpp$(DependSuffix) -MM "../../../Native/Core/Input.Handler.cpp"

$(IntermediateDirectory)/Core_Input.Handler.cpp$(PreprocessSuffix): ../../../Native/Core/Input.Handler.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Input.Handler.cpp$(PreprocessSuffix) "../../../Native/Core/Input.Handler.cpp"

$(IntermediateDirectory)/Core_Kernel.cpp$(ObjectSuffix): ../../../Native/Core/Kernel.cpp $(IntermediateDirectory)/Core_Kernel.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Kernel.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Kernel.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Kernel.cpp$(DependSuffix): ../../../Native/Core/Kernel.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Kernel.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Kernel.cpp$(DependSuffix) -MM "../../../Native/Core/Kernel.cpp"

$(IntermediateDirectory)/Core_Kernel.cpp$(PreprocessSuffix): ../../../Native/Core/Kernel.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Kernel.cpp$(PreprocessSuffix) "../../../Native/Core/Kernel.cpp"

$(IntermediateDirectory)/Core_Kernel.Display.cpp$(ObjectSuffix): ../../../Native/Core/Kernel.Display.cpp $(IntermediateDirectory)/Core_Kernel.Display.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Kernel.Display.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Kernel.Display.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Kernel.Display.cpp$(DependSuffix): ../../../Native/Core/Kernel.Display.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Kernel.Display.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Kernel.Display.cpp$(DependSuffix) -MM "../../../Native/Core/Kernel.Display.cpp"

$(IntermediateDirectory)/Core_Kernel.Display.cpp$(PreprocessSuffix): ../../../Native/Core/Kernel.Display.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Kernel.Display.cpp$(PreprocessSuffix) "../../../Native/Core/Kernel.Display.cpp"

$(IntermediateDirectory)/Core_Kernel.Display.Linux.cpp$(ObjectSuffix): ../../../Native/Core/Kernel.Display.Linux.cpp $(IntermediateDirectory)/Core_Kernel.Display.Linux.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Kernel.Display.Linux.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Kernel.Display.Linux.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Kernel.Display.Linux.cpp$(DependSuffix): ../../../Native/Core/Kernel.Display.Linux.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Kernel.Display.Linux.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Kernel.Display.Linux.cpp$(DependSuffix) -MM "../../../Native/Core/Kernel.Display.Linux.cpp"

$(IntermediateDirectory)/Core_Kernel.Display.Linux.cpp$(PreprocessSuffix): ../../../Native/Core/Kernel.Display.Linux.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Kernel.Display.Linux.cpp$(PreprocessSuffix) "../../../Native/Core/Kernel.Display.Linux.cpp"

$(IntermediateDirectory)/Core_Kernel.Mobile.cpp$(ObjectSuffix): ../../../Native/Core/Kernel.Mobile.cpp $(IntermediateDirectory)/Core_Kernel.Mobile.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Kernel.Mobile.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Kernel.Mobile.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Kernel.Mobile.cpp$(DependSuffix): ../../../Native/Core/Kernel.Mobile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Kernel.Mobile.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Kernel.Mobile.cpp$(DependSuffix) -MM "../../../Native/Core/Kernel.Mobile.cpp"

$(IntermediateDirectory)/Core_Kernel.Mobile.cpp$(PreprocessSuffix): ../../../Native/Core/Kernel.Mobile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Kernel.Mobile.cpp$(PreprocessSuffix) "../../../Native/Core/Kernel.Mobile.cpp"

$(IntermediateDirectory)/Core_Kernel.Windows.cpp$(ObjectSuffix): ../../../Native/Core/Kernel.Windows.cpp $(IntermediateDirectory)/Core_Kernel.Windows.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Kernel.Windows.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Kernel.Windows.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Kernel.Windows.cpp$(DependSuffix): ../../../Native/Core/Kernel.Windows.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Kernel.Windows.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Kernel.Windows.cpp$(DependSuffix) -MM "../../../Native/Core/Kernel.Windows.cpp"

$(IntermediateDirectory)/Core_Kernel.Windows.cpp$(PreprocessSuffix): ../../../Native/Core/Kernel.Windows.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Kernel.Windows.cpp$(PreprocessSuffix) "../../../Native/Core/Kernel.Windows.cpp"

$(IntermediateDirectory)/Core_Mediator.Client.cpp$(ObjectSuffix): ../../../Native/Core/Mediator.Client.cpp $(IntermediateDirectory)/Core_Mediator.Client.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Mediator.Client.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Mediator.Client.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Mediator.Client.cpp$(DependSuffix): ../../../Native/Core/Mediator.Client.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Mediator.Client.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Mediator.Client.cpp$(DependSuffix) -MM "../../../Native/Core/Mediator.Client.cpp"

$(IntermediateDirectory)/Core_Mediator.Client.cpp$(PreprocessSuffix): ../../../Native/Core/Mediator.Client.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Mediator.Client.cpp$(PreprocessSuffix) "../../../Native/Core/Mediator.Client.cpp"

$(IntermediateDirectory)/Core_Message.Instance.cpp$(ObjectSuffix): ../../../Native/Core/Message.Instance.cpp $(IntermediateDirectory)/Core_Message.Instance.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Message.Instance.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Message.Instance.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Message.Instance.cpp$(DependSuffix): ../../../Native/Core/Message.Instance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Message.Instance.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Message.Instance.cpp$(DependSuffix) -MM "../../../Native/Core/Message.Instance.cpp"

$(IntermediateDirectory)/Core_Message.Instance.cpp$(PreprocessSuffix): ../../../Native/Core/Message.Instance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Message.Instance.cpp$(PreprocessSuffix) "../../../Native/Core/Message.Instance.cpp"

$(IntermediateDirectory)/Core_Notifications.cpp$(ObjectSuffix): ../../../Native/Core/Notifications.cpp $(IntermediateDirectory)/Core_Notifications.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Notifications.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Notifications.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Notifications.cpp$(DependSuffix): ../../../Native/Core/Notifications.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Notifications.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Notifications.cpp$(DependSuffix) -MM "../../../Native/Core/Notifications.cpp"

$(IntermediateDirectory)/Core_Notifications.cpp$(PreprocessSuffix): ../../../Native/Core/Notifications.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Notifications.cpp$(PreprocessSuffix) "../../../Native/Core/Notifications.cpp"

$(IntermediateDirectory)/Core_Performance.Count.cpp$(ObjectSuffix): ../../../Native/Core/Performance.Count.cpp $(IntermediateDirectory)/Core_Performance.Count.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Performance.Count.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Performance.Count.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Performance.Count.cpp$(DependSuffix): ../../../Native/Core/Performance.Count.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Performance.Count.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Performance.Count.cpp$(DependSuffix) -MM "../../../Native/Core/Performance.Count.cpp"

$(IntermediateDirectory)/Core_Performance.Count.cpp$(PreprocessSuffix): ../../../Native/Core/Performance.Count.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Performance.Count.cpp$(PreprocessSuffix) "../../../Native/Core/Performance.Count.cpp"

$(IntermediateDirectory)/Core_Stunt.Request.cpp$(ObjectSuffix): ../../../Native/Core/Stunt.Request.cpp $(IntermediateDirectory)/Core_Stunt.Request.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Stunt.Request.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Stunt.Request.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Stunt.Request.cpp$(DependSuffix): ../../../Native/Core/Stunt.Request.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Stunt.Request.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Stunt.Request.cpp$(DependSuffix) -MM "../../../Native/Core/Stunt.Request.cpp"

$(IntermediateDirectory)/Core_Stunt.Request.cpp$(PreprocessSuffix): ../../../Native/Core/Stunt.Request.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Stunt.Request.cpp$(PreprocessSuffix) "../../../Native/Core/Stunt.Request.cpp"

$(IntermediateDirectory)/Core_Utils.cpp$(ObjectSuffix): ../../../Native/Core/Utils.cpp $(IntermediateDirectory)/Core_Utils.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Utils.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Utils.cpp$(DependSuffix): ../../../Native/Core/Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Utils.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Utils.cpp$(DependSuffix) -MM "../../../Native/Core/Utils.cpp"

$(IntermediateDirectory)/Core_Utils.cpp$(PreprocessSuffix): ../../../Native/Core/Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Utils.cpp$(PreprocessSuffix) "../../../Native/Core/Utils.cpp"

$(IntermediateDirectory)/Core_Queue.List.cpp$(ObjectSuffix): ../../../Native/Core/Queue.List.cpp $(IntermediateDirectory)/Core_Queue.List.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Core/Queue.List.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Queue.List.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Queue.List.cpp$(DependSuffix): ../../../Native/Core/Queue.List.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Queue.List.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Queue.List.cpp$(DependSuffix) -MM "../../../Native/Core/Queue.List.cpp"

$(IntermediateDirectory)/Core_Queue.List.cpp$(PreprocessSuffix): ../../../Native/Core/Queue.List.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Queue.List.cpp$(PreprocessSuffix) "../../../Native/Core/Queue.List.cpp"

$(IntermediateDirectory)/Core_Queue.Vector.cpp$(ObjectSuffix): ../../../Common/Queue.Vector.cpp $(IntermediateDirectory)/Core_Queue.Vector.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Queue.Vector.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_Queue.Vector.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_Queue.Vector.cpp$(DependSuffix): ../../../Common/Queue.Vector.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_Queue.Vector.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_Queue.Vector.cpp$(DependSuffix) -MM "../../../Common/Queue.Vector.cpp"

$(IntermediateDirectory)/Core_Queue.Vector.cpp$(PreprocessSuffix): ../../../Common/Queue.Vector.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_Queue.Vector.cpp$(PreprocessSuffix) "../../../Common/Queue.Vector.cpp"

$(IntermediateDirectory)/Device_Device.Android.cpp$(ObjectSuffix): ../../../Native/Device/Device.Android.cpp $(IntermediateDirectory)/Device_Device.Android.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Device/Device.Android.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Device_Device.Android.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Device_Device.Android.cpp$(DependSuffix): ../../../Native/Device/Device.Android.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Device_Device.Android.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Device_Device.Android.cpp$(DependSuffix) -MM "../../../Native/Device/Device.Android.cpp"

$(IntermediateDirectory)/Device_Device.Android.cpp$(PreprocessSuffix): ../../../Native/Device/Device.Android.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Device_Device.Android.cpp$(PreprocessSuffix) "../../../Native/Device/Device.Android.cpp"

$(IntermediateDirectory)/Device_Device.Base.cpp$(ObjectSuffix): ../../../Native/Device/Device.Base.cpp $(IntermediateDirectory)/Device_Device.Base.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Device/Device.Base.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Device_Device.Base.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Device_Device.Base.cpp$(DependSuffix): ../../../Native/Device/Device.Base.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Device_Device.Base.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Device_Device.Base.cpp$(DependSuffix) -MM "../../../Native/Device/Device.Base.cpp"

$(IntermediateDirectory)/Device_Device.Base.cpp$(PreprocessSuffix): ../../../Native/Device/Device.Base.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Device_Device.Base.cpp$(PreprocessSuffix) "../../../Native/Device/Device.Base.cpp"

$(IntermediateDirectory)/Device_Device.Display.cpp$(ObjectSuffix): ../../../Native/Device/Device.Display.cpp $(IntermediateDirectory)/Device_Device.Display.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Device/Device.Display.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Device_Device.Display.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Device_Device.Display.cpp$(DependSuffix): ../../../Native/Device/Device.Display.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Device_Device.Display.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Device_Device.Display.cpp$(DependSuffix) -MM "../../../Native/Device/Device.Display.cpp"

$(IntermediateDirectory)/Device_Device.Display.cpp$(PreprocessSuffix): ../../../Native/Device/Device.Display.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Device_Device.Display.cpp$(PreprocessSuffix) "../../../Native/Device/Device.Display.cpp"

$(IntermediateDirectory)/Device_Device.Display.Win.cpp$(ObjectSuffix): ../../../Native/Device/Device.Display.Win.cpp $(IntermediateDirectory)/Device_Device.Display.Win.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Device/Device.Display.Win.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Device_Device.Display.Win.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Device_Device.Display.Win.cpp$(DependSuffix): ../../../Native/Device/Device.Display.Win.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Device_Device.Display.Win.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Device_Device.Display.Win.cpp$(DependSuffix) -MM "../../../Native/Device/Device.Display.Win.cpp"

$(IntermediateDirectory)/Device_Device.Display.Win.cpp$(PreprocessSuffix): ../../../Native/Device/Device.Display.Win.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Device_Device.Display.Win.cpp$(PreprocessSuffix) "../../../Native/Device/Device.Display.Win.cpp"

$(IntermediateDirectory)/Device_Device.Instance.cpp$(ObjectSuffix): ../../../Native/Device/Device.Instance.cpp $(IntermediateDirectory)/Device_Device.Instance.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Device/Device.Instance.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Device_Device.Instance.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Device_Device.Instance.cpp$(DependSuffix): ../../../Native/Device/Device.Instance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Device_Device.Instance.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Device_Device.Instance.cpp$(DependSuffix) -MM "../../../Native/Device/Device.Instance.cpp"

$(IntermediateDirectory)/Device_Device.Instance.cpp$(PreprocessSuffix): ../../../Native/Device/Device.Instance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Device_Device.Instance.cpp$(PreprocessSuffix) "../../../Native/Device/Device.Instance.cpp"

$(IntermediateDirectory)/Device_Device.Linux.cpp$(ObjectSuffix): ../../../Native/Device/Device.Linux.cpp $(IntermediateDirectory)/Device_Device.Linux.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Device/Device.Linux.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Device_Device.Linux.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Device_Device.Linux.cpp$(DependSuffix): ../../../Native/Device/Device.Linux.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Device_Device.Linux.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Device_Device.Linux.cpp$(DependSuffix) -MM "../../../Native/Device/Device.Linux.cpp"

$(IntermediateDirectory)/Device_Device.Linux.cpp$(PreprocessSuffix): ../../../Native/Device/Device.Linux.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Device_Device.Linux.cpp$(PreprocessSuffix) "../../../Native/Device/Device.Linux.cpp"

$(IntermediateDirectory)/Device_Device.List.cpp$(ObjectSuffix): ../../../Native/Device/Device.List.cpp $(IntermediateDirectory)/Device_Device.List.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Device/Device.List.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Device_Device.List.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Device_Device.List.cpp$(DependSuffix): ../../../Native/Device/Device.List.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Device_Device.List.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Device_Device.List.cpp$(DependSuffix) -MM "../../../Native/Device/Device.List.cpp"

$(IntermediateDirectory)/Device_Device.List.cpp$(PreprocessSuffix): ../../../Native/Device/Device.List.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Device_Device.List.cpp$(PreprocessSuffix) "../../../Native/Device/Device.List.cpp"

$(IntermediateDirectory)/Device_Devices.cpp$(ObjectSuffix): ../../../Native/Device/Devices.cpp $(IntermediateDirectory)/Device_Devices.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Device/Devices.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Device_Devices.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Device_Devices.cpp$(DependSuffix): ../../../Native/Device/Devices.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Device_Devices.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Device_Devices.cpp$(DependSuffix) -MM "../../../Native/Device/Devices.cpp"

$(IntermediateDirectory)/Device_Devices.cpp$(PreprocessSuffix): ../../../Native/Device/Devices.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Device_Devices.cpp$(PreprocessSuffix) "../../../Native/Device/Devices.cpp"

$(IntermediateDirectory)/Portal_Portal.Device.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Device.cpp $(IntermediateDirectory)/Portal_Portal.Device.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Device.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Device.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Device.cpp$(DependSuffix): ../../../Native/Portal/Portal.Device.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Device.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Device.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Device.cpp"

$(IntermediateDirectory)/Portal_Portal.Device.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Device.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Device.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Device.cpp"

$(IntermediateDirectory)/Portal_Portal.Generator.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Generator.cpp $(IntermediateDirectory)/Portal_Portal.Generator.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Generator.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Generator.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Generator.cpp$(DependSuffix): ../../../Native/Portal/Portal.Generator.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Generator.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Generator.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Generator.cpp"

$(IntermediateDirectory)/Portal_Portal.Generator.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Generator.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Generator.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Generator.cpp"

$(IntermediateDirectory)/Portal_Portal.Generator.Mobile.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Generator.Mobile.cpp $(IntermediateDirectory)/Portal_Portal.Generator.Mobile.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Generator.Mobile.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Generator.Mobile.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Generator.Mobile.cpp$(DependSuffix): ../../../Native/Portal/Portal.Generator.Mobile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Generator.Mobile.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Generator.Mobile.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Generator.Mobile.cpp"

$(IntermediateDirectory)/Portal_Portal.Generator.Mobile.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Generator.Mobile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Generator.Mobile.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Generator.Mobile.cpp"

$(IntermediateDirectory)/Portal_Portal.Generator.Windows.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Generator.Windows.cpp $(IntermediateDirectory)/Portal_Portal.Generator.Windows.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Generator.Windows.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Generator.Windows.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Generator.Windows.cpp$(DependSuffix): ../../../Native/Portal/Portal.Generator.Windows.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Generator.Windows.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Generator.Windows.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Generator.Windows.cpp"

$(IntermediateDirectory)/Portal_Portal.Generator.Windows.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Generator.Windows.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Generator.Windows.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Generator.Windows.cpp"

$(IntermediateDirectory)/Portal_Portal.Info.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Info.cpp $(IntermediateDirectory)/Portal_Portal.Info.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Info.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Info.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Info.cpp$(DependSuffix): ../../../Native/Portal/Portal.Info.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Info.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Info.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Info.cpp"

$(IntermediateDirectory)/Portal_Portal.Info.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Info.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Info.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Info.cpp"

$(IntermediateDirectory)/Portal_Portal.Instance.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Instance.cpp $(IntermediateDirectory)/Portal_Portal.Instance.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Instance.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Instance.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Instance.cpp$(DependSuffix): ../../../Native/Portal/Portal.Instance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Instance.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Instance.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Instance.cpp"

$(IntermediateDirectory)/Portal_Portal.Instance.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Instance.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Instance.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Instance.cpp"

$(IntermediateDirectory)/Portal_Portal.Receiver.Android.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Receiver.Android.cpp $(IntermediateDirectory)/Portal_Portal.Receiver.Android.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Receiver.Android.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Receiver.Android.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Receiver.Android.cpp$(DependSuffix): ../../../Native/Portal/Portal.Receiver.Android.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Receiver.Android.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Receiver.Android.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Receiver.Android.cpp"

$(IntermediateDirectory)/Portal_Portal.Receiver.Android.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Receiver.Android.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Receiver.Android.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Receiver.Android.cpp"

$(IntermediateDirectory)/Portal_Portal.Receiver.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Receiver.cpp $(IntermediateDirectory)/Portal_Portal.Receiver.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Receiver.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Receiver.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Receiver.cpp$(DependSuffix): ../../../Native/Portal/Portal.Receiver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Receiver.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Receiver.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Receiver.cpp"

$(IntermediateDirectory)/Portal_Portal.Receiver.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Receiver.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Receiver.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Receiver.cpp"

$(IntermediateDirectory)/Portal_Portal.Receiver.Windows.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Receiver.Windows.cpp $(IntermediateDirectory)/Portal_Portal.Receiver.Windows.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Receiver.Windows.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Receiver.Windows.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Receiver.Windows.cpp$(DependSuffix): ../../../Native/Portal/Portal.Receiver.Windows.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Receiver.Windows.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Receiver.Windows.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Receiver.Windows.cpp"

$(IntermediateDirectory)/Portal_Portal.Receiver.Windows.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Receiver.Windows.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Receiver.Windows.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Receiver.Windows.cpp"

$(IntermediateDirectory)/Portal_Portal.Stream.cpp$(ObjectSuffix): ../../../Native/Portal/Portal.Stream.cpp $(IntermediateDirectory)/Portal_Portal.Stream.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Portal/Portal.Stream.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Portal_Portal.Stream.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Portal_Portal.Stream.cpp$(DependSuffix): ../../../Native/Portal/Portal.Stream.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Portal_Portal.Stream.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Portal_Portal.Stream.cpp$(DependSuffix) -MM "../../../Native/Portal/Portal.Stream.cpp"

$(IntermediateDirectory)/Portal_Portal.Stream.cpp$(PreprocessSuffix): ../../../Native/Portal/Portal.Stream.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Portal_Portal.Stream.cpp$(PreprocessSuffix) "../../../Native/Portal/Portal.Stream.cpp"

$(IntermediateDirectory)/Interop_Export.cpp$(ObjectSuffix): ../../../Common/Interop/Export.cpp $(IntermediateDirectory)/Interop_Export.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Interop/Export.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Interop_Export.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Interop_Export.cpp$(DependSuffix): ../../../Common/Interop/Export.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Interop_Export.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Interop_Export.cpp$(DependSuffix) -MM "../../../Common/Interop/Export.cpp"

$(IntermediateDirectory)/Interop_Export.cpp$(PreprocessSuffix): ../../../Common/Interop/Export.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Interop_Export.cpp$(PreprocessSuffix) "../../../Common/Interop/Export.cpp"

$(IntermediateDirectory)/Interop_Threads.cpp$(ObjectSuffix): ../../../Common/Interop/Threads.cpp $(IntermediateDirectory)/Interop_Threads.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Common/Interop/Threads.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Interop_Threads.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Interop_Threads.cpp$(DependSuffix): ../../../Common/Interop/Threads.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Interop_Threads.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Interop_Threads.cpp$(DependSuffix) -MM "../../../Common/Interop/Threads.cpp"

$(IntermediateDirectory)/Interop_Threads.cpp$(PreprocessSuffix): ../../../Common/Interop/Threads.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Interop_Threads.cpp$(PreprocessSuffix) "../../../Common/Interop/Threads.cpp"

$(IntermediateDirectory)/DynLib_Dyn.OpenCL.cpp$(ObjectSuffix): ../../../Native/DynLib/Dyn.OpenCL.cpp $(IntermediateDirectory)/DynLib_Dyn.OpenCL.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/DynLib/Dyn.OpenCL.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/DynLib_Dyn.OpenCL.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/DynLib_Dyn.OpenCL.cpp$(DependSuffix): ../../../Native/DynLib/Dyn.OpenCL.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/DynLib_Dyn.OpenCL.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/DynLib_Dyn.OpenCL.cpp$(DependSuffix) -MM "../../../Native/DynLib/Dyn.OpenCL.cpp"

$(IntermediateDirectory)/DynLib_Dyn.OpenCL.cpp$(PreprocessSuffix): ../../../Native/DynLib/Dyn.OpenCL.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/DynLib_Dyn.OpenCL.cpp$(PreprocessSuffix) "../../../Native/DynLib/Dyn.OpenCL.cpp"

$(IntermediateDirectory)/DynLib_Dyn.Lib.Crypto.cpp$(ObjectSuffix): ../../../3rd/DynLib/Dyn.Lib.Crypto.cpp $(IntermediateDirectory)/DynLib_Dyn.Lib.Crypto.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/3rd/DynLib/Dyn.Lib.Crypto.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/DynLib_Dyn.Lib.Crypto.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/DynLib_Dyn.Lib.Crypto.cpp$(DependSuffix): ../../../3rd/DynLib/Dyn.Lib.Crypto.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/DynLib_Dyn.Lib.Crypto.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/DynLib_Dyn.Lib.Crypto.cpp$(DependSuffix) -MM "../../../3rd/DynLib/Dyn.Lib.Crypto.cpp"

$(IntermediateDirectory)/DynLib_Dyn.Lib.Crypto.cpp$(PreprocessSuffix): ../../../3rd/DynLib/Dyn.Lib.Crypto.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/DynLib_Dyn.Lib.Crypto.cpp$(PreprocessSuffix) "../../../3rd/DynLib/Dyn.Lib.Crypto.cpp"

$(IntermediateDirectory)/Renderer_Render.OpenCL.cpp$(ObjectSuffix): ../../../Native/Renderer/Render.OpenCL.cpp $(IntermediateDirectory)/Renderer_Render.OpenCL.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/chi-tai/git/environs-dev/Native/Renderer/Render.OpenCL.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Renderer_Render.OpenCL.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Renderer_Render.OpenCL.cpp$(DependSuffix): ../../../Native/Renderer/Render.OpenCL.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Renderer_Render.OpenCL.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Renderer_Render.OpenCL.cpp$(DependSuffix) -MM "../../../Native/Renderer/Render.OpenCL.cpp"

$(IntermediateDirectory)/Renderer_Render.OpenCL.cpp$(PreprocessSuffix): ../../../Native/Renderer/Render.OpenCL.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Renderer_Render.OpenCL.cpp$(PreprocessSuffix) "../../../Native/Renderer/Render.OpenCL.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


