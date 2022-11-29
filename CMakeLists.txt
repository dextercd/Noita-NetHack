cmake_minimum_required(VERSION 3.24)

project(NetHack)

include(FetchContent)

FetchContent_Declare(Lua
    URL https://www.lua.org/ftp/lua-5.4.4.tar.gz
    URL_HASH SHA1=03c27684b9d5d9783fb79a7c836ba1cdc5f309cd
)

FetchContent_GetProperties(Lua)
if (NOT lua_POPULATED)
    FetchContent_Populate(Lua)
    configure_file(LuaCMakeLists.txt ${lua_SOURCE_DIR}/CMakeLists.txt COPYONLY)
    add_subdirectory(${lua_SOURCE_DIR} ${lua_BINARY_DIR})
endif()
