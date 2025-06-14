cmake_minimum_required(VERSION 3.10)

# Define the project name
project(verytb)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# build verytb
add_subdirectory(verytb)

# Add any demo project here
add_subdirectory(sim/picorv32-main)