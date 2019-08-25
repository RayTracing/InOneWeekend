cmake_minimum_required(VERSION 3.6)

set(ProjectName "InOneWeekend")

project(${ProjectName})

# Add source and header files
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
# Exclude the build directory
list(FILTER sources EXCLUDE REGEX "build/")
# Create the VS filters based on the directory tree layout
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

# Add the project
add_executable (${ProjectName} ${sources})

# Add openmp
# https://stackoverflow.com/questions/56202041/compiling-and-linking-against-openmp-with-appleclang-on-mac-os-x-mojave
find_package(OpenMP) # Find the package
target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_LIBRARIES}) # Link against it for C++

# Set this project as startup project (MSVC)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ProjectName})