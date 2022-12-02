int CO;
int LI;

int has_colors(void)
{
    return 1;
}

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

char erase_char = 'b';
char kill_char = 21;

HANDLE hConIn;
HANDLE hConOut;
void play_usersound(const char * x, int xx) {}
void mswin_destroy_reg(void) {}


void
mswin_raw_print(const char *str)
{
    fprintf(stdout, "%s", str);
}

void
mswin_raw_print_flush(void)
{
}

void gettty(void) {}
void settty(const char* x) {}
void setftty(void) {}

int GUILaunched = 0;

void
map_subkeyvalue(char *op)
{
    return;
}

void
set_altkeyhandler(const char *inName)
{
    return;
}

int
set_keyhandling_via_option(void)
{
    return 1;
}

void
set_altkeyhandling(const char *inName)
{
}

void
nethack_enter_consoletty(void)
{
    return;
}

void
tty_utf8graphics_fixup(void)
{
    return;
}

int tgetch(void)
{
    return getc(stdin);
}

void
consoletty_open(int mode)
{
}



#endif
