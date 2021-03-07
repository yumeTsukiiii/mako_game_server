cmake_minimum_required(VERSION 3.15)
project(linux_game_server C)

set(CMAKE_C_STANDARD 99)

add_executable(linux_game_server main.c aioutil.c aioutil.h socketutil.c socketutil.h signalutil.c signalutil.h multiclienthandler.c multiclienthandler.h globaldatastore.c globaldatastore.h)

target_link_libraries(linux_game_server -lrt)