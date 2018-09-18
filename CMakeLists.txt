# CMakeList.txt : CMake project for shooter, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

INCLUDE_DIRECTORIES( $ENV{BOOST_DIR} )

# Add source to this project's executable.
add_executable (shooter "game.cpp")

# TODO: Add tests and install targets if needed.
