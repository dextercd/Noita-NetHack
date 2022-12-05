#ifndef NH_NOITA_PROCMAN_HPP
#define NH_NOITA_PROCMAN_HPP

#include <cstddef>

extern "C" {
void procman_init();
bool procman_write_stdin(const char* text, std::size_t length);
bool procman_install_poll_event_hook(void* vfpoll_event);
void procman_dump_terminal();
void procman_read_stdout();
void procman_handle_events();
const char* procman_get_line(int linenum);
}

#endif // Header guard
