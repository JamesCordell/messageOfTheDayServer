cmake_minimum_required (VERSION 2.6)
project (motd)
# The version number.
set (motd_VERSION_MAJOR 1)
set (motd_VERSION_MINOR 0)
 
 
# add the executable
add_executable(motdServer src/socketServer.c)
add_executable(motdClient src/socketClient.c)
