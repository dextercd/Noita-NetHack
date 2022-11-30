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


add_library(libnethack
    src/allmain.c
    src/apply.c
    src/artifact.c
    src/attrib.c
    src/ball.c
    src/bones.c
    src/botl.c
    src/cmd.c
    src/date.c
    src/dbridge.c
    src/decl.c
    src/detect.c
    src/dig.c
    src/display.c
    src/dlb.c
    src/do.c
    src/do_name.c
    src/do_wear.c
    src/dog.c
    src/dogmove.c
    src/dokick.c
    src/dothrow.c
    src/drawing.c
    src/dungeon.c
    src/eat.c
    src/end.c
    src/engrave.c
    src/exper.c
    src/explode.c
    src/extralev.c
    src/files.c
    src/fountain.c
    src/hack.c
    src/hacklib.c
    src/insight.c
    src/invent.c
    src/isaac64.c
    src/light.c
    src/lock.c
    src/mail.c
    src/makemon.c
    src/mcastu.c
    src/mdlib.c
    src/mhitm.c
    src/mhitu.c
    src/minion.c
    src/mklev.c
    src/mkmap.c
    src/mkmaze.c
    src/mkobj.c
    src/mkroom.c
    src/mon.c
    src/mondata.c
    src/monmove.c
    src/monst.c
    src/mplayer.c
    src/mthrowu.c
    src/muse.c
    src/music.c
    src/nhlobj.c
    src/nhlsel.c
    src/nhlua.c
    src/o_init.c
    src/objects.c
    src/objnam.c
    src/options.c
    src/pager.c
    src/pickup.c
    src/pline.c
    src/polyself.c
    src/potion.c
    src/pray.c
    src/priest.c
    src/quest.c
    src/questpgr.c
    src/read.c
    src/rect.c
    src/region.c
    src/restore.c
    src/rip.c
    src/rnd.c
    src/role.c
    src/rumors.c
    src/save.c
    src/sfstruct.c
    src/shk.c
    src/shknam.c
    src/sit.c
    src/sounds.c
    src/sp_lev.c
    src/spell.c
    src/steal.c
    src/steed.c
    src/symbols.c
    src/sys.c
    src/teleport.c
    src/timeout.c
    src/topten.c
    src/track.c
    src/trap.c
    src/u_init.c
    src/uhitm.c
    src/utf8map.c
    src/vault.c
    src/version.c
    src/vision.c
    src/weapon.c
    src/were.c
    src/wield.c
    src/windows.c
    src/wizard.c
    src/worm.c
    src/worn.c
    src/write.c
    src/zap.c
    src/alloc.c

    sys/share/cppregex.cpp
    sys/libnh/libnhmain.c

    win/shim/winshim.c
)

if (WIN32)
    # target_sources(libnethack)
else()
    target_sources(libnethack
        PRIVATE
            sys/share/unixtty.c
            sys/share/ioctl.c
            sys/unix/unixunix.c
    )
endif()

target_include_directories(libnethack PRIVATE include)
target_link_libraries(libnethack PRIVATE liblua)
target_compile_definitions(libnethack
    PRIVATE
        SHIM_GRAPHICS
        NOTTYGRAPHICS
)
