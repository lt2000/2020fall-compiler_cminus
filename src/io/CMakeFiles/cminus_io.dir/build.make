# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cp/2020fall-Compiler_CMinus

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cp/2020fall-Compiler_CMinus

# Include any dependencies generated for this target.
include src/io/CMakeFiles/cminus_io.dir/depend.make

# Include the progress variables for this target.
include src/io/CMakeFiles/cminus_io.dir/progress.make

# Include the compile flags for this target's objects.
include src/io/CMakeFiles/cminus_io.dir/flags.make

src/io/CMakeFiles/cminus_io.dir/io.c.o: src/io/CMakeFiles/cminus_io.dir/flags.make
src/io/CMakeFiles/cminus_io.dir/io.c.o: src/io/io.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cp/2020fall-Compiler_CMinus/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object src/io/CMakeFiles/cminus_io.dir/io.c.o"
	cd /home/cp/2020fall-Compiler_CMinus/src/io && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/cminus_io.dir/io.c.o   -c /home/cp/2020fall-Compiler_CMinus/src/io/io.c

src/io/CMakeFiles/cminus_io.dir/io.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cminus_io.dir/io.c.i"
	cd /home/cp/2020fall-Compiler_CMinus/src/io && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/cp/2020fall-Compiler_CMinus/src/io/io.c > CMakeFiles/cminus_io.dir/io.c.i

src/io/CMakeFiles/cminus_io.dir/io.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cminus_io.dir/io.c.s"
	cd /home/cp/2020fall-Compiler_CMinus/src/io && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/cp/2020fall-Compiler_CMinus/src/io/io.c -o CMakeFiles/cminus_io.dir/io.c.s

# Object files for target cminus_io
cminus_io_OBJECTS = \
"CMakeFiles/cminus_io.dir/io.c.o"

# External object files for target cminus_io
cminus_io_EXTERNAL_OBJECTS =

libcminus_io.a: src/io/CMakeFiles/cminus_io.dir/io.c.o
libcminus_io.a: src/io/CMakeFiles/cminus_io.dir/build.make
libcminus_io.a: src/io/CMakeFiles/cminus_io.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cp/2020fall-Compiler_CMinus/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library ../../libcminus_io.a"
	cd /home/cp/2020fall-Compiler_CMinus/src/io && $(CMAKE_COMMAND) -P CMakeFiles/cminus_io.dir/cmake_clean_target.cmake
	cd /home/cp/2020fall-Compiler_CMinus/src/io && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cminus_io.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/io/CMakeFiles/cminus_io.dir/build: libcminus_io.a

.PHONY : src/io/CMakeFiles/cminus_io.dir/build

src/io/CMakeFiles/cminus_io.dir/clean:
	cd /home/cp/2020fall-Compiler_CMinus/src/io && $(CMAKE_COMMAND) -P CMakeFiles/cminus_io.dir/cmake_clean.cmake
.PHONY : src/io/CMakeFiles/cminus_io.dir/clean

src/io/CMakeFiles/cminus_io.dir/depend:
	cd /home/cp/2020fall-Compiler_CMinus && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cp/2020fall-Compiler_CMinus /home/cp/2020fall-Compiler_CMinus/src/io /home/cp/2020fall-Compiler_CMinus /home/cp/2020fall-Compiler_CMinus/src/io /home/cp/2020fall-Compiler_CMinus/src/io/CMakeFiles/cminus_io.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/io/CMakeFiles/cminus_io.dir/depend

