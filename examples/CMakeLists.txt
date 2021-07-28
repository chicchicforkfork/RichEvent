project(rich_example)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_COMPILER /usr/bin/c++)
set(CMAKE_CXX_FLAGS "-m64 -std=c++17")
add_compile_options(-W -Wall -g -fpermissive)

include(FetchContent)

FetchContent_Declare(kairos
  GIT_REPOSITORY https://github.com/chicchicforkfork/Kairos.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(kairos)

include_directories(include)

add_executable(reader
    reader.cpp
)
target_link_libraries(reader
PUBLIC
    richevent
    kairos
)

add_executable(writer
    writer.cpp
)
target_link_libraries(writer
PUBLIC
    richevent
    kairos
)
