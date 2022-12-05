#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <cstdlib>

namespace fs = std::filesystem;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <MinHook.h>
#include <katerm/terminal.hpp>

#include "sdlmin.hpp"

// Job process so if the game crashes our child processes are taken alongside with it..
HANDLE job_object;

HANDLE stdin_write;
HANDLE stdout_read;

bool currently_writing = false;
HANDLE event_stdin_writing;
bool currently_reading = false;
HANDLE event_stdout_reading;

OVERLAPPED stdin_overlapped;
OVERLAPPED stdout_overlapped;

katerm::terminal terminal{{90, 30}};
katerm::decoder term_decoder;
katerm::terminal_instructee term_instructee{&terminal};

extern "C" __declspec(dllexport)
void procman_init()
{
    auto current_path = fs::current_path();
    auto nethack_dir = current_path / "mods/Noita-NetHack/NetHack";
    auto nethack_path = nethack_dir / "nethack.exe";

    if (MH_Initialize() != MH_OK) {
        std::cerr << "Couldn't initialize MinHook\n";
    }

    job_object = CreateJobObjectA(nullptr, nullptr);

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_limit{};
    job_limit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(job_object, JobObjectExtendedLimitInformation,
        &job_limit, sizeof(job_limit)
    );

    std::string pipe_prefix = "\\\\.\\pipe\\NN-" + std::to_string(GetCurrentProcessId());
    std::string pipe_input_name = pipe_prefix + "-IN";
    std::string pipe_output_name = pipe_prefix + "-OUT";

    stdin_write = CreateNamedPipeA(
        pipe_input_name.c_str(),
        PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
        PIPE_UNLIMITED_INSTANCES,
        2048, 2048,
        0,
        nullptr
    );

    stdout_read = CreateNamedPipeA(
        pipe_output_name.c_str(),
        PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
        PIPE_UNLIMITED_INSTANCES,
        2048, 2048,
        0,
        nullptr
    );

    auto stdin_read = CreateFileA(
        pipe_input_name.c_str(),
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    auto stdout_write = CreateFileA(
        pipe_output_name.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (stdin_write == INVALID_HANDLE_VALUE || stdout_read == INVALID_HANDLE_VALUE ||
        stdin_read == INVALID_HANDLE_VALUE || stdout_write == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Error when opening pipes\n";
        std::exit(10);
    }

    SetHandleInformation(stdin_read, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    SetHandleInformation(stdout_write, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    STARTUPINFO startup_info{};
    startup_info.cb = sizeof(STARTUPINFO);
    startup_info.hStdError = stdout_write;
    startup_info.hStdOutput = stdout_write;
    startup_info.hStdInput = stdin_read;
    startup_info.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION proc_info;

    // The process must be started suspended so that it can't create child processes
    // outside of the job object.
    CreateProcessA(
        nethack_path.string().c_str(),
        nullptr, // command line
        nullptr, // proc attr
        nullptr, // thread attr
        TRUE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED,
        nullptr, // env
        nethack_dir.string().c_str(),
        &startup_info,
        &proc_info
    );

    AssignProcessToJobObject(job_object, proc_info.hProcess);
    ResumeThread(proc_info.hThread);

    CloseHandle(stdout_write);
    CloseHandle(stdin_read);

    event_stdin_writing = CreateEventA(nullptr, false, false, nullptr);
    event_stdout_reading = CreateEventA(nullptr, false, false, nullptr);
    stdin_overlapped.hEvent = event_stdin_writing;
    stdout_overlapped.hEvent = event_stdout_reading;
}

void handle_stdin_complete()
{
    currently_writing = false;
}

char write_buffer[1024];

extern "C" __declspec(dllexport)
bool procman_write_stdin(const char* text, std::size_t length)
{
    if (currently_writing)
        return false;

    length = length < sizeof(write_buffer) ? length : sizeof(write_buffer);
    std::memcpy(write_buffer, text, length);

    currently_writing = true;
    auto result = WriteFile(
        stdin_write,
        write_buffer,
        length,
        nullptr,
        &stdin_overlapped
    );

    DWORD last_error;
    if (result || (last_error = GetLastError()) == ERROR_IO_PENDING) {
        handle_stdin_complete();
        return true;
    }

    std::cerr << "Error in: " << __func__ << ": " << last_error << '\n';
    return false;
}


using SDL_PollEvent_f = int(*)(SDL_Event* event);
using SDL_GetKeyName_f = const char*(*)(std::int32_t key);

SDL_PollEvent_f original_SDL_PollEvent = nullptr;
SDL_GetKeyName_f SDL_GetKeyName = nullptr;

std::atomic<bool> capture_keyboard;

int SDL_PollEvent_hook(SDL_Event* event)
{
    if (!capture_keyboard)
        return original_SDL_PollEvent(event);

    auto ret = original_SDL_PollEvent(event);
    if (!ret) return ret;

    switch (event->type) {
        default:
            return ret;

        case SDL_TEXTINPUT: {
            auto inputev = (SDL_TextInputEvent*)event;
            procman_write_stdin(inputev->text, std::strlen(inputev->text));
        } break;

        case SDL_KEYDOWN: {
            auto keyev = (SDL_KeyboardEvent*)event;
            if (keyev->keysym.sym <= 0x80) {
                char in_character = (char)keyev->keysym.sym;
                char out_character = 0;
                bool control = keyev->keysym.mod & (KMOD_LCTRL | KMOD_RCTRL);
                bool shift = keyev->keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT);

                if (in_character == '\r') out_character = '\n';
                if (in_character == '\b') out_character = '\b';
                if (in_character == '\x1b') out_character = '\x1b';

                if (control)
                    out_character = in_character & 0x1f;

                if (out_character) {
                    char key[]{out_character};
                    procman_write_stdin(key, 1);
                }
            }
        } break;

        case SDL_KEYUP: 
        case SDL_KEYMAPCHANGED:
        case SDL_TEXTEDITING:
            ;
    }

    return SDL_PollEvent_hook(event);
}

extern "C" __declspec(dllexport)
bool procman_install_poll_event_hook(void* vfpoll_event, void* vfget_key_name)
{
    SDL_GetKeyName = reinterpret_cast<SDL_GetKeyName_f>(vfget_key_name);

    auto poll_event = reinterpret_cast<SDL_PollEvent_f>(vfpoll_event);
    auto create_result = MH_CreateHook(
        vfpoll_event,
        reinterpret_cast<void*>(SDL_PollEvent_hook),
        reinterpret_cast<void**>(&original_SDL_PollEvent)
    );

    if (create_result != MH_OK) {
        std::cerr << "Couldn't create SDL_PollEvent hook\n";
        return false;
    }

    auto enable_result = MH_EnableHook(vfpoll_event);

    if (enable_result != MH_OK) {
        std::cerr << "Couldn't enable SDL_PollEvent hook\n";
        return false;
    }

    return true;
}

extern "C" __declspec(dllexport)
void procman_set_capture_input(bool state)
{
    capture_keyboard = state;
}


extern "C" __declspec(dllexport)
void procman_dump_terminal()
{
    //std::ofstream out("terminal.txt");
    std::ostream& out = std::cout;
    auto& screen = terminal.screen;
    auto size = screen.size();
    for (int y = 0; y != size.height; ++y) {
        for (int x = 0; x != size.width; ++x) {
            auto& glyph = screen.get_glyph({x, y});
            if (glyph.code == ' ' || (glyph.code > 32 && glyph.code < 0x7f))
                out << (char)glyph.code;
            else if (glyph.code == '\0')
                out << ' ';
            else
                out << '[' << (int)glyph.code << ']';
        }
        out << '\n';
    }
    out << std::flush;
}

char receive_buffer[1024];

void handle_stdout_complete()
{
    if (!currently_reading) {
        std::cerr << "Expected to be in reading state!\n";
        std::exit(11);
    }

    currently_reading = false;

    DWORD transferred;
    auto result = GetOverlappedResult(stdout_read, &stdout_overlapped, &transferred, FALSE);
    if (!result) {
        std::cerr << "GetOverlappedResult failed in " << __func__ << '\n';
        return;
    }

    /*
    std::cout << "r: -------\n";
    for (std::size_t i = 0; i != transferred; ++i) {
        std::cout << (int)receive_buffer[i] << " ";
    }
    std::cout << '\n';

    for (std::size_t i = 0; i != transferred; ++i) {
        char ch = receive_buffer[i];
        if (ch >= 0x20 && ch < 0x7f)
            std::cout << ch;
        else
            std::cout << ".";
    }
    std::cout << "\n";
    std::cout << "--------\n";
    */

    term_decoder.decode(receive_buffer, transferred, term_instructee);
}

extern "C" __declspec(dllexport)
void procman_read_stdout()
{
    if (currently_reading)
        return;

    currently_reading = true;

    auto result = ReadFile(
        stdout_read,
        receive_buffer,
        sizeof(receive_buffer),
        nullptr,
        &stdout_overlapped
    );

    DWORD last_error;
    if (result || (last_error = GetLastError()) == ERROR_IO_PENDING) {
        return;
    }

    std::cerr << "Error in: " << __func__ << ": " << last_error << '\n';
}

extern "C" __declspec(dllexport)
void procman_handle_events()
{
    HANDLE events[]{
        event_stdin_writing,
        event_stdout_reading,
    };

    auto result = WaitForMultipleObjects(std::size(events), events, FALSE, 0);
    switch (result) {
        case WAIT_OBJECT_0 + 0: {
            handle_stdin_complete();
        } break;

        case WAIT_OBJECT_0 + 1: {
            handle_stdout_complete();
        } break;

        case WAIT_FAILED: {
            auto last_error = GetLastError();
            std::cerr << "Error in: " << __func__ << ": " << last_error << '\n';
        }

        case WAIT_TIMEOUT:
            return;
    }

    // Keep handling events until we hit a failure or timeout
    procman_handle_events();
}

std::string last_line = "";

extern "C" __declspec(dllexport)
const char* procman_get_line(int linenum)
{
    auto& screen = terminal.screen;
    auto width = screen.size().width;
    auto line = screen.get_line(linenum);
    last_line.clear();
    for (std::size_t index = 0; index != width; ++index) {
        auto ch = (char)line[index].code;
        if (ch == '\0') ch = ' ';
        last_line.push_back(ch);
    }
    return last_line.c_str();
}

extern "C" __declspec(dllexport)
int procman_scroll_changed()
{
    return terminal.screen.changed_scroll();
}

extern "C" __declspec(dllexport)
bool procman_line_changed(int linenr)
{
    return terminal.screen.lines[linenr].changed;
}

extern "C" __declspec(dllexport)
const char* procman_glyph_at(int x, int y)
{
    static char return_buffer[5] = "";
    auto codepoint = terminal.screen.get_line(y)[x].code;
    if (codepoint <= 255) {
        return_buffer[0] = codepoint == 0 ? ' ' : (char)codepoint;
        return_buffer[1] = '\0';
        return return_buffer;
    }

    return "?";
}

extern "C" __declspec(dllexport)
void procman_colour_at(int x, int y, katerm::colour* ret)
{
    *ret = terminal.screen.get_line(y)[x].style.fg;
}

extern "C" __declspec(dllexport)
void procman_clear_changed()
{
    terminal.screen.clear_changes();
}
