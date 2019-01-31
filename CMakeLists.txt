# CMakeList.txt : CMake project for shooter, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)


if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wsign-conversion -pedantic")
endif()

INCLUDE_DIRECTORIES( $ENV{BOOST_DIR} )

# Add source to this project's executable.
add_executable (shooter "entity_system_class.cpp" "game.cpp")

# TODO: Add tests and install targets if needed.
