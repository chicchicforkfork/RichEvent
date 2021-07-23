project(rich_example)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_COMPILER /usr/bin/c++)
set(CMAKE_CXX_FLAGS "-m64 -std=c++17")
add_compile_options(-W -Wall -g -fpermissive)

include_directories(include)

add_executable(reader_pull
    reader_pull.cpp
)
target_link_libraries(reader_pull
PUBLIC
    richevent
)

add_executable(writer_push
    writer_push.cpp
)
target_link_libraries(writer_push
PUBLIC
    richevent
)
