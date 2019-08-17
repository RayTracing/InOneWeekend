
PROJECT( Real_Type_Exploration )

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# Create real as float executable
add_executable( Real_Type_Float
  main.cc
)

# create real as double executable
ADD_DEFINITIONS ( -D REAL_AS_DOUBLE )
add_executable( Real_Type_Double
  main.cc
)

