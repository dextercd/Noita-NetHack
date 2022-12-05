#include <cstdio>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "process_manager.hpp"

int main()
{
    procman_init();

    #define WRITE(TEXT) \
        procman_write_stdin(TEXT, sizeof(TEXT) - 1);

    auto flush = [&]() {
        Sleep(10);
        procman_read_stdout();
        Sleep(10);
        procman_handle_events();
        std::puts("-------");
        procman_dump_terminal();
    };

    WRITE("dexter");
    WRITE("\n");
    flush();
    WRITE("y\n");
    flush();
    WRITE("y\n\n\n\n\n");
    flush();
    WRITE("hhhhhhhhhjjjjjjjjq");
    flush();
}
