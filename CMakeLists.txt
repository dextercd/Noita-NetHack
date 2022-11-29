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


# alloc library
add_library(alloc OBJECT
    src/alloc.c
    util/panic.c
)

target_include_directories(alloc PRIVATE include)
target_link_libraries(alloc PRIVATE liblua)


# dlb
add_executable(dlb
    util/dlb_main.c
    src/dlb.c
    $<TARGET_OBJECTS:alloc>
)

target_include_directories(dlb PRIVATE include)
target_link_libraries(dlb PRIVATE liblua)


# uudecode
add_executable(uudecode sys/share/uudecode.c)
target_include_directories(uudecode PRIVATE include)
