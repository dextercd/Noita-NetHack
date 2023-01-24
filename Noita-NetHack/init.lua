local ffi = require("ffi")

ffi.cdef([[

void SDL_StartTextInput();
void SDL_StopTextInput();
int SDL_PollEvent(struct SDL_Event* event);
const char* SDL_GetKeyName(int32_t key);

]])

local SDL = ffi.load("SDL2")


ffi.cdef([[

void procman_init();
bool procman_write_stdin(const char* text, size_t length);
bool procman_install_poll_event_hook(
    void* vfpoll_event,
    void* vfget_key_name
);
void procman_set_capture_input(bool enable);
void procman_dump_terminal();
void procman_read_stdout();
void procman_handle_events();
const char* procman_get_line(int linenum);

struct colour {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

int procman_scroll_changed();
bool procman_line_changed(int linenr);
const char* procman_glyph_at(int x, int y);
void procman_colour_at(int x, int y, struct colour* ret);
void procman_clear_changed();
int procman_cursor_x();
int procman_cursor_y();
]])

local pm = ffi.load("mods/Noita-NetHack/process-manager.dll")
pm.procman_init()
pm.procman_install_poll_event_hook(
    SDL.SDL_PollEvent,
    SDL.SDL_GetKeyName
)

function capture_input(enable)
    if enable then
        pm.procman_set_capture_input(true)
        SDL.SDL_StartTextInput()
    else
        SDL.SDL_StopTextInput()
        pm.procman_set_capture_input(false)
    end
end

capture_input(true)


local terminal_width = 90
local terminal_height = 30


local lines = {}
for i=1, terminal_height do
    table.insert(lines, {})
end

function fetch_terminal_changes()
    local scroll_change = pm.procman_scroll_changed() 
    if scroll_change ~= 0 then
        if scroll_change > 1 then
            for i=1, scroll_change do
                table.insert(
                    lines,
                    table.remove(lines, 1)
                )
            end
        else
            for i=1, -scroll_change do
                table.insert(
                    lines, 1,
                    table.remove(lines)
                )
            end
        end
    end

    for linenr=0, terminal_height - 1 do
        if pm.procman_line_changed(linenr) then
            lines[linenr + 1] = load_line(linenr)
        end
    end

    pm.procman_clear_changed()
end

function load_line(linenr)
    local sections = {}

    local current_colour = ffi.new("struct colour")
    pm.procman_colour_at(0, linenr, current_colour)
    local current_string = {ffi.string(pm.procman_glyph_at(0, linenr))}

    for x = 1, terminal_width - 1 do
        local next_colour = ffi.new("struct colour")
        pm.procman_colour_at(x, linenr, next_colour)
        if
            next_colour.r ~= current_colour.r or
            next_colour.g ~= current_colour.g or
            next_colour.b ~= current_colour.b
        then
            table.insert(sections, {current_colour, table.concat(current_string)})
            current_colour = next_colour
            current_string = {}
        end

        table.insert(
            current_string,
            ffi.string(pm.procman_glyph_at(x, linenr))
        )
    end

    table.insert(sections, {current_colour, table.concat(current_string)})
    return sections
end

local mono_width = 6
local mono_height = 9
local font_space_width = 3

function OnWorldPreUpdate()
    pm.procman_read_stdout()
    pm.procman_handle_events()

    gui = gui or GuiCreate()
    GuiStartFrame(gui)

    local offsetx = 20
    local offsety = 50

    GuiZSetForNextWidget(gui, 11)
    GuiImageNinePiece(gui, 1,
        offsetx, offsety,
        terminal_width * mono_width, mono_height * terminal_height
    )

    local cursor_x = pm.procman_cursor_x()
    local cursor_y = pm.procman_cursor_y()

    fetch_terminal_changes()
    for linenr=1, #lines do
        local line = lines[linenr]
        local y = (linenr - 1) * 9

        local character_index = 0

        for _, section in ipairs(line) do
            local colour, text = unpack(section)
            local fr = tonumber(colour.r) / 255
            local fg = tonumber(colour.g) / 255
            local fb = tonumber(colour.b) / 255

            for ti=1, #text do
                local ch = string.sub(text, ti, ti)

                local x = character_index * mono_width
                local x_adjust = 0

                if ch == '$' then
                    ch = ' ' .. ch
                    x_adjust = -font_space_width
                end

                GuiColorSetForNextWidget(gui, fr, fg, fb, 1)
                GuiText(gui, x + offsetx + x_adjust, y + offsety, ch)

                if cursor_x == character_index and cursor_y == linenr - 1 then
                    GuiColorSetForNextWidget(gui, 1, 1, 1, .6)
                    GuiText(gui, x + offsetx - 0.5, y + offsety + 0.2, '_')
                end


                character_index = character_index + 1
            end
        end
    end
end
