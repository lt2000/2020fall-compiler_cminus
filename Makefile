# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target install/local
install/local: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local

# Special rule for the target install/local
install/local/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local/fast

# Special rule for the target install
install: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install

# Special rule for the target install
install/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install/fast

# Special rule for the target list_install_components
list_install_components:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Available install components are: \"Unspecified\""
.PHONY : list_install_components

# Special rule for the target list_install_components
list_install_components/fast: list_install_components

.PHONY : list_install_components/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# Special rule for the target install/strip
install/strip: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip

# Special rule for the target install/strip
install/strip/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/cp/2020fall-Compiler_CMinus/CMakeFiles /home/cp/2020fall-Compiler_CMinus/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/cp/2020fall-Compiler_CMinus/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named intrinsics_gen

# Build rule for target.
intrinsics_gen: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 intrinsics_gen
.PHONY : intrinsics_gen

# fast build rule for target.
intrinsics_gen/fast:
	$(MAKE) -f CMakeFiles/intrinsics_gen.dir/build.make CMakeFiles/intrinsics_gen.dir/build
.PHONY : intrinsics_gen/fast

#=============================================================================
# Target rules for targets named flex

# Build rule for target.
flex: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 flex
.PHONY : flex

# fast build rule for target.
flex/fast:
	$(MAKE) -f src/lexer/CMakeFiles/flex.dir/build.make src/lexer/CMakeFiles/flex.dir/build
.PHONY : flex/fast

#=============================================================================
# Target rules for targets named syntax

# Build rule for target.
syntax: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 syntax
.PHONY : syntax

# fast build rule for target.
syntax/fast:
	$(MAKE) -f src/parser/CMakeFiles/syntax.dir/build.make src/parser/CMakeFiles/syntax.dir/build
.PHONY : syntax/fast

#=============================================================================
# Target rules for targets named cminus_io

# Build rule for target.
cminus_io: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 cminus_io
.PHONY : cminus_io

# fast build rule for target.
cminus_io/fast:
	$(MAKE) -f src/io/CMakeFiles/cminus_io.dir/build.make src/io/CMakeFiles/cminus_io.dir/build
.PHONY : cminus_io/fast

#=============================================================================
# Target rules for targets named common

# Build rule for target.
common: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 common
.PHONY : common

# fast build rule for target.
common/fast:
	$(MAKE) -f src/common/CMakeFiles/common.dir/build.make src/common/CMakeFiles/common.dir/build
.PHONY : common/fast

#=============================================================================
# Target rules for targets named IR_lib

# Build rule for target.
IR_lib: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 IR_lib
.PHONY : IR_lib

# fast build rule for target.
IR_lib/fast:
	$(MAKE) -f src/lightir/CMakeFiles/IR_lib.dir/build.make src/lightir/CMakeFiles/IR_lib.dir/build
.PHONY : IR_lib/fast

#=============================================================================
# Target rules for targets named cminusfc

# Build rule for target.
cminusfc: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 cminusfc
.PHONY : cminusfc

# fast build rule for target.
cminusfc/fast:
	$(MAKE) -f src/cminusfc/CMakeFiles/cminusfc.dir/build.make src/cminusfc/CMakeFiles/cminusfc.dir/build
.PHONY : cminusfc/fast

#=============================================================================
# Target rules for targets named test_logging

# Build rule for target.
test_logging: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test_logging
.PHONY : test_logging

# fast build rule for target.
test_logging/fast:
	$(MAKE) -f tests/CMakeFiles/test_logging.dir/build.make tests/CMakeFiles/test_logging.dir/build
.PHONY : test_logging/fast

#=============================================================================
# Target rules for targets named test_ast

# Build rule for target.
test_ast: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test_ast
.PHONY : test_ast

# fast build rule for target.
test_ast/fast:
	$(MAKE) -f tests/CMakeFiles/test_ast.dir/build.make tests/CMakeFiles/test_ast.dir/build
.PHONY : test_ast/fast

#=============================================================================
# Target rules for targets named lexer

# Build rule for target.
lexer: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 lexer
.PHONY : lexer

# fast build rule for target.
lexer/fast:
	$(MAKE) -f tests/lab1/CMakeFiles/lexer.dir/build.make tests/lab1/CMakeFiles/lexer.dir/build
.PHONY : lexer/fast

#=============================================================================
# Target rules for targets named parser

# Build rule for target.
parser: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 parser
.PHONY : parser

# fast build rule for target.
parser/fast:
	$(MAKE) -f tests/lab2/CMakeFiles/parser.dir/build.make tests/lab2/CMakeFiles/parser.dir/build
.PHONY : parser/fast

#=============================================================================
# Target rules for targets named stu_while_generator

# Build rule for target.
stu_while_generator: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 stu_while_generator
.PHONY : stu_while_generator

# fast build rule for target.
stu_while_generator/fast:
	$(MAKE) -f tests/lab3/CMakeFiles/stu_while_generator.dir/build.make tests/lab3/CMakeFiles/stu_while_generator.dir/build
.PHONY : stu_while_generator/fast

#=============================================================================
# Target rules for targets named stu_if_generator

# Build rule for target.
stu_if_generator: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 stu_if_generator
.PHONY : stu_if_generator

# fast build rule for target.
stu_if_generator/fast:
	$(MAKE) -f tests/lab3/CMakeFiles/stu_if_generator.dir/build.make tests/lab3/CMakeFiles/stu_if_generator.dir/build
.PHONY : stu_if_generator/fast

#=============================================================================
# Target rules for targets named stu_assign_generator

# Build rule for target.
stu_assign_generator: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 stu_assign_generator
.PHONY : stu_assign_generator

# fast build rule for target.
stu_assign_generator/fast:
	$(MAKE) -f tests/lab3/CMakeFiles/stu_assign_generator.dir/build.make tests/lab3/CMakeFiles/stu_assign_generator.dir/build
.PHONY : stu_assign_generator/fast

#=============================================================================
# Target rules for targets named stu_fun_generator

# Build rule for target.
stu_fun_generator: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 stu_fun_generator
.PHONY : stu_fun_generator

# fast build rule for target.
stu_fun_generator/fast:
	$(MAKE) -f tests/lab3/CMakeFiles/stu_fun_generator.dir/build.make tests/lab3/CMakeFiles/stu_fun_generator.dir/build
.PHONY : stu_fun_generator/fast

#=============================================================================
# Target rules for targets named gcd_array_generator

# Build rule for target.
gcd_array_generator: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 gcd_array_generator
.PHONY : gcd_array_generator

# fast build rule for target.
gcd_array_generator/fast:
	$(MAKE) -f tests/lab3/CMakeFiles/gcd_array_generator.dir/build.make tests/lab3/CMakeFiles/gcd_array_generator.dir/build
.PHONY : gcd_array_generator/fast

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... install/local"
	@echo "... install"
	@echo "... list_install_components"
	@echo "... rebuild_cache"
	@echo "... edit_cache"
	@echo "... install/strip"
	@echo "... intrinsics_gen"
	@echo "... flex"
	@echo "... syntax"
	@echo "... cminus_io"
	@echo "... common"
	@echo "... IR_lib"
	@echo "... cminusfc"
	@echo "... test_logging"
	@echo "... test_ast"
	@echo "... lexer"
	@echo "... parser"
	@echo "... stu_while_generator"
	@echo "... stu_if_generator"
	@echo "... stu_assign_generator"
	@echo "... stu_fun_generator"
	@echo "... gcd_array_generator"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

