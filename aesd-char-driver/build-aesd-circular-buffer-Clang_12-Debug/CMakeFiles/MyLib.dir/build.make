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
CMAKE_SOURCE_DIR = /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/build-aesd-circular-buffer-Clang_12-Debug

# Include any dependencies generated for this target.
include CMakeFiles/MyLib.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/MyLib.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MyLib.dir/flags.make

CMakeFiles/MyLib.dir/aesd-circular-buffer.c.o: CMakeFiles/MyLib.dir/flags.make
CMakeFiles/MyLib.dir/aesd-circular-buffer.c.o: ../aesd-circular-buffer.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/build-aesd-circular-buffer-Clang_12-Debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/MyLib.dir/aesd-circular-buffer.c.o"
	/usr/bin/clang-10 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/MyLib.dir/aesd-circular-buffer.c.o   -c /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/aesd-circular-buffer.c

CMakeFiles/MyLib.dir/aesd-circular-buffer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/MyLib.dir/aesd-circular-buffer.c.i"
	/usr/bin/clang-10 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/aesd-circular-buffer.c > CMakeFiles/MyLib.dir/aesd-circular-buffer.c.i

CMakeFiles/MyLib.dir/aesd-circular-buffer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/MyLib.dir/aesd-circular-buffer.c.s"
	/usr/bin/clang-10 $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/aesd-circular-buffer.c -o CMakeFiles/MyLib.dir/aesd-circular-buffer.c.s

# Object files for target MyLib
MyLib_OBJECTS = \
"CMakeFiles/MyLib.dir/aesd-circular-buffer.c.o"

# External object files for target MyLib
MyLib_EXTERNAL_OBJECTS =

libMyLib.a: CMakeFiles/MyLib.dir/aesd-circular-buffer.c.o
libMyLib.a: CMakeFiles/MyLib.dir/build.make
libMyLib.a: CMakeFiles/MyLib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/build-aesd-circular-buffer-Clang_12-Debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libMyLib.a"
	$(CMAKE_COMMAND) -P CMakeFiles/MyLib.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MyLib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MyLib.dir/build: libMyLib.a

.PHONY : CMakeFiles/MyLib.dir/build

CMakeFiles/MyLib.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MyLib.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MyLib.dir/clean

CMakeFiles/MyLib.dir/depend:
	cd /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/build-aesd-circular-buffer-Clang_12-Debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/build-aesd-circular-buffer-Clang_12-Debug /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/build-aesd-circular-buffer-Clang_12-Debug /home/amente/Downloads/EmLinuxCourseAssigments/A1/aesd-char-driver/build-aesd-circular-buffer-Clang_12-Debug/CMakeFiles/MyLib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MyLib.dir/depend

