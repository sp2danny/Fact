##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=Client
ConfigurationName      :=Release
WorkspacePath          := "/home/sp2danny/extra/Fact"
ProjectPath            := "/home/sp2danny/extra/Fact"
IntermediateDirectory  :=./bin/Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Daniel Nyström
Date                   :=01/17/19
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
OutputFile             :=./$(ProjectName).out
Preprocessors          :=$(PreprocessorSwitch)NDEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="Client.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            := -std=c++17 -stdlib=libc++ 
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)/usr/include/gtk-2.0 $(IncludeSwitch)/usr/include/glib-2.0 $(IncludeSwitch)/usr/lib/x86_64-linux-gnu/glib-2.0/include $(IncludeSwitch)/usr/include/cairo $(IncludeSwitch)/usr/include/pango-1.0 $(IncludeSwitch)/usr/lib/x86_64-linux-gnu/gtk-2.0/include $(IncludeSwitch)/usr/include/gdk-pixbuf-2.0 $(IncludeSwitch)/usr/include/atk-1.0 $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)gtk-x11-2.0 $(LibrarySwitch)gdk-x11-2.0 $(LibrarySwitch)pangocairo-1.0 $(LibrarySwitch)atk-1.0 $(LibrarySwitch)cairo $(LibrarySwitch)gdk_pixbuf-2.0 $(LibrarySwitch)gio-2.0 $(LibrarySwitch)pangoft2-1.0 $(LibrarySwitch)pango-1.0 $(LibrarySwitch)gobject-2.0 $(LibrarySwitch)glib-2.0 $(LibrarySwitch)fontconfig $(LibrarySwitch)freetype $(LibrarySwitch)boost_system $(LibrarySwitch)pthread $(LibrarySwitch)boost_system $(LibrarySwitch)boost_filesystem $(LibrarySwitch)boost_thread $(LibrarySwitch)boost_chrono $(LibrarySwitch)gmp $(LibrarySwitch)gmpxx 
ArLibs                 :=  "gtk-x11-2.0" "gdk-x11-2.0" "pangocairo-1.0" "atk-1.0" "cairo" "gdk_pixbuf-2.0" "gio-2.0" "pangoft2-1.0" "pango-1.0" "gobject-2.0" "glib-2.0" "fontconfig" "freetype" "boost_system" "pthread" "boost_system" "boost_filesystem" "boost_thread" "boost_chrono" "gmp" "gmpxx" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/clang++ 
CC       := /usr/bin/clang 
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pedantic -stdlib=libc++  -O2 $(Preprocessors)
CFLAGS   :=  -O2 $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as 


##
## User defined environment variables
##
Objects0=$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_Client.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_graph.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_mandel_hp.cpp$(ObjectSuffix) $(IntermediateDirectory)/src_gtk_general.cpp$(ObjectSuffix) 



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
$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix): src/cmdline.cpp $(IntermediateDirectory)/src_cmdline.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sp2danny/extra/Fact/src/cmdline.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_cmdline.cpp$(DependSuffix): src/cmdline.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_cmdline.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_cmdline.cpp$(DependSuffix) -MM "src/cmdline.cpp"

$(IntermediateDirectory)/src_cmdline.cpp$(PreprocessSuffix): src/cmdline.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_cmdline.cpp$(PreprocessSuffix) "src/cmdline.cpp"

$(IntermediateDirectory)/src_Client.cpp$(ObjectSuffix): src/Client.cpp $(IntermediateDirectory)/src_Client.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sp2danny/extra/Fact/src/Client.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_Client.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_Client.cpp$(DependSuffix): src/Client.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_Client.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_Client.cpp$(DependSuffix) -MM "src/Client.cpp"

$(IntermediateDirectory)/src_Client.cpp$(PreprocessSuffix): src/Client.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_Client.cpp$(PreprocessSuffix) "src/Client.cpp"

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

$(IntermediateDirectory)/src_gtk_general.cpp$(ObjectSuffix): src/gtk_general.cpp $(IntermediateDirectory)/src_gtk_general.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/sp2danny/extra/Fact/src/gtk_general.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/src_gtk_general.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/src_gtk_general.cpp$(DependSuffix): src/gtk_general.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/src_gtk_general.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/src_gtk_general.cpp$(DependSuffix) -MM "src/gtk_general.cpp"

$(IntermediateDirectory)/src_gtk_general.cpp$(PreprocessSuffix): src/gtk_general.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/src_gtk_general.cpp$(PreprocessSuffix) "src/gtk_general.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) ./bin/Release/*$(ObjectSuffix)
	$(RM) ./bin/Release/*$(DependSuffix)
	$(RM) $(OutputFile)
	$(RM) ".build-release/Client"


