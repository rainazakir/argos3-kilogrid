# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.22.1/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.22.1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/rzakir/Documents/argosim/argos3-kilobot/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/rzakir/Documents/argosim/argos3-kilobot/build

# Utility rule file for argos3plugin_simulator_kilobot_autogen.

# Include any custom commands dependencies for this target.
include plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/compiler_depend.make

# Include the progress variables for this target.
include plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/progress.make

plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/Users/rzakir/Documents/argosim/argos3-kilobot/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Automatic MOC for target argos3plugin_simulator_kilobot"
	cd /Users/rzakir/Documents/argosim/argos3-kilobot/build/plugins/robots/kilobot && /usr/local/Cellar/cmake/3.22.1/bin/cmake -E cmake_autogen /Users/rzakir/Documents/argosim/argos3-kilobot/build/plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/AutogenInfo.json Release

argos3plugin_simulator_kilobot_autogen: plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen
argos3plugin_simulator_kilobot_autogen: plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/build.make
.PHONY : argos3plugin_simulator_kilobot_autogen

# Rule to build all files generated by this target.
plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/build: argos3plugin_simulator_kilobot_autogen
.PHONY : plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/build

plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/clean:
	cd /Users/rzakir/Documents/argosim/argos3-kilobot/build/plugins/robots/kilobot && $(CMAKE_COMMAND) -P CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/cmake_clean.cmake
.PHONY : plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/clean

plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/depend:
	cd /Users/rzakir/Documents/argosim/argos3-kilobot/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/rzakir/Documents/argosim/argos3-kilobot/src /Users/rzakir/Documents/argosim/argos3-kilobot/src/plugins/robots/kilobot /Users/rzakir/Documents/argosim/argos3-kilobot/build /Users/rzakir/Documents/argosim/argos3-kilobot/build/plugins/robots/kilobot /Users/rzakir/Documents/argosim/argos3-kilobot/build/plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : plugins/robots/kilobot/CMakeFiles/argos3plugin_simulator_kilobot_autogen.dir/depend

