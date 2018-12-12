##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=factmm
ConfigurationName      :=Release
WorkspacePath          :=/home/daniel/project/Fact
ProjectPath            :=/home/daniel/project/Fact
IntermediateDirectory  :=./bin/Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=daniel
Date                   :=12/12/18
CodeLitePath           :=/home/daniel/.codelite
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
ObjectsFileList        :="factmm.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch)/usr/include/gtk-2.0 $(IncludeSwitch)/usr/include/glib-2.0 $(IncludeSwitch)/usr/lib/x86_64-linux-gnu/glib-2.0/include $(IncludeSwitch)/usr/include/cairo $(IncludeSwitch)/usr/include/pango-1.0 $(IncludeSwitch)/usr/lib/x86_64-linux-gnu/gtk-2.0/include $(IncludeSwitch)/usr/include/gdk-pixbuf-2.0 $(IncludeSwitch)/usr/include/atk-1.0 $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)gtk-x11-2.0 $(LibrarySwitch)gdk-x11-2.0 $(LibrarySwitch)pangocairo-1.0 $(LibrarySwitch)atk-1.0 $(LibrarySwitch)cairo $(LibrarySwitch)gdk_pixbuf-2.0 $(LibrarySwitch)gio-2.0 $(LibrarySwitch)pangoft2-1.0 $(LibrarySwitch)pango-1.0 $(LibrarySwitch)gobject-2.0 $(LibrarySwitch)glib-2.0 $(LibrarySwitch)fontconfig $(LibrarySwitch)freetype $(LibrarySwitch)boost_system $(LibrarySwitch)boost_filesystem $(LibrarySwitch)pthread $(LibrarySwitch)gmp $(LibrarySwitch)gmpxx 
ArLibs                 :=  "gtk-x11-2.0" "gdk-x11-2.0" "pangocairo-1.0" "atk-1.0" "cairo" "gdk_pixbuf-2.0" "gio-2.0" "pangoft2-1.0" "pango-1.0" "gobject-2.0" "glib-2.0" "fontconfig" "freetype" "boost_system" "boost_filesystem" "pthread" "gmp" "gmpxx" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)/usr/lib/x86_64-linux-gnu/ 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/clang++
CC       := /usr/bin/clang
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pedantic -O3 $(Preprocessors)
CFLAGS   :=  -O2 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/src_factmm.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_graph.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_mandel_hp.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./bin/Release || $(MakeDirCommand) ./bin/Release


$(IntermediateDirectory)/.d:
	@test -d ./bin/Release || $(MakeDirCommand) ./bin/Release

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/src_factmm.cpp$(ObjectSuffix): src/factmm.cpp $(IntermediateDirectory)/src_factmm.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/daniel/project/Fact/src/factmm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_factmm.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_factmm.cpp$(DependSuffix): src/factmm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_factmm.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_factmm.cpp$(DependSuffix) -MM src/factmm.cpp

$(IntermediateDirectory)/src_factmm.cpp$(PreprocessSuffix): src/factmm.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_factmm.cpp$(PreprocessSuffix) src/factmm.cpp

$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix): src/cmdline.cpp $(IntermediateDirectory)/src_cmdline.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/daniel/project/Fact/src/cmdline.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_cmdline.cpp$(DependSuffix): src/cmdline.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_cmdline.cpp$(DependSuffix) -MM src/cmdline.cpp

$(IntermediateDirectory)/src_cmdline.cpp$(PreprocessSuffix): src/cmdline.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_cmdline.cpp$(PreprocessSuffix) src/cmdline.cpp

$(IntermediateDirectory)/src_graph.cpp$(ObjectSuffix): src/graph.cpp $(IntermediateDirectory)/src_graph.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/daniel/project/Fact/src/graph.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_graph.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_graph.cpp$(DependSuffix): src/graph.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_graph.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_graph.cpp$(DependSuffix) -MM src/graph.cpp

$(IntermediateDirectory)/src_graph.cpp$(PreprocessSuffix): src/graph.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_graph.cpp$(PreprocessSuffix) src/graph.cpp

$(IntermediateDirectory)/src_mandel_hp.cpp$(ObjectSuffix): src/mandel_hp.cpp $(IntermediateDirectory)/src_mandel_hp.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/daniel/project/Fact/src/mandel_hp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_mandel_hp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_mandel_hp.cpp$(DependSuffix): src/mandel_hp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_mandel_hp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_mandel_hp.cpp$(DependSuffix) -MM src/mandel_hp.cpp

$(IntermediateDirectory)/src_mandel_hp.cpp$(PreprocessSuffix): src/mandel_hp.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_mandel_hp.cpp$(PreprocessSuffix) src/mandel_hp.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./bin/Release/


