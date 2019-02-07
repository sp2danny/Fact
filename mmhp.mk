##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=mmhp
ConfigurationName      :=Release
WorkspacePath          := "/home/sp2danny/extra/Fact"
ProjectPath            := "/home/sp2danny/extra/Fact"
IntermediateDirectory  :=./bin/Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Daniel Nyström
Date                   :=02/07/19
CodeLitePath           :="/home/sp2danny/.codelite"
LinkerName             :=/usr/bin/clang++ 
SharedObjectLinkerName :=/usr/bin/clang++ -shared -fPIC
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
OutputFile             :=$(ProjectName).out
Preprocessors          :=$(PreprocessorSwitch)NDEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="mmhp.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)boost_system $(LibrarySwitch)boost_filesystem $(LibrarySwitch)boost_thread $(LibrarySwitch)boost_chrono $(LibrarySwitch)pthread $(LibrarySwitch)gmp $(LibrarySwitch)gmpxx 
ArLibs                 :=  "boost_system" "boost_filesystem" "boost_thread" "boost_chrono" "pthread" "gmp" "gmpxx" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)/usr/lib/x86_64-linux-gnu/ 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/clang++ 
CC       := /usr/bin/clang 
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Werror -O3 $(Preprocessors)
CFLAGS   := -Wall -O3 $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as 


##
## User defined environment variables
##
Objects0=$(IntermediateDirectory)/src_mmhp.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_graph.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_mandel_hp.cpp$(ObjectSuffix) 



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
	@test -d ./bin/Release || $(MakeDirCommand) ./bin/Release

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_mmhp.cpp$(ObjectSuffix): src/mmhp.cpp $(IntermediateDirectory)/src_mmhp.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sp2danny/extra/Fact/src/mmhp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_mmhp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_mmhp.cpp$(DependSuffix): src/mmhp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_mmhp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_mmhp.cpp$(DependSuffix) -MM "src/mmhp.cpp"

$(IntermediateDirectory)/src_mmhp.cpp$(PreprocessSuffix): src/mmhp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_mmhp.cpp$(PreprocessSuffix) "src/mmhp.cpp"

$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix): src/cmdline.cpp $(IntermediateDirectory)/src_cmdline.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sp2danny/extra/Fact/src/cmdline.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_cmdline.cpp$(DependSuffix): src/cmdline.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_cmdline.cpp$(DependSuffix) -MM "src/cmdline.cpp"

$(IntermediateDirectory)/src_cmdline.cpp$(PreprocessSuffix): src/cmdline.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_cmdline.cpp$(PreprocessSuffix) "src/cmdline.cpp"

$(IntermediateDirectory)/src_graph.cpp$(ObjectSuffix): src/graph.cpp $(IntermediateDirectory)/src_graph.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sp2danny/extra/Fact/src/graph.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_graph.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_graph.cpp$(DependSuffix): src/graph.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_graph.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_graph.cpp$(DependSuffix) -MM "src/graph.cpp"

$(IntermediateDirectory)/src_graph.cpp$(PreprocessSuffix): src/graph.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_graph.cpp$(PreprocessSuffix) "src/graph.cpp"

$(IntermediateDirectory)/src_mandel_hp.cpp$(ObjectSuffix): src/mandel_hp.cpp $(IntermediateDirectory)/src_mandel_hp.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sp2danny/extra/Fact/src/mandel_hp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_mandel_hp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_mandel_hp.cpp$(DependSuffix): src/mandel_hp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_mandel_hp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_mandel_hp.cpp$(DependSuffix) -MM "src/mandel_hp.cpp"

$(IntermediateDirectory)/src_mandel_hp.cpp$(PreprocessSuffix): src/mandel_hp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_mandel_hp.cpp$(PreprocessSuffix) "src/mandel_hp.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) ./bin/Release/*$(ObjectSuffix)
	$(RM) ./bin/Release/*$(DependSuffix)
	$(RM) $(OutputFile)
	$(RM) ".build-release/mmhp"


