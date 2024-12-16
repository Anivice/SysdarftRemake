#ifndef UI_CURSES_H
#define UI_CURSES_H

#include <array>
#include <mutex>
#include <atomic>
#include <ncurses.h>

struct cursor_position_t {
    int x;
    int y;
};

#define WIDTH   127
#define HEIGHT  31

class ui_curses {
private:
    std::array< std::array<int, HEIGHT>, WIDTH> video_memory = { 0 };
    std::mutex memory_access_mutex;
    std::atomic<bool> video_memory_changed = true;
    std::atomic<bool> monitor_input_status = true;
    std::atomic<bool> running_thread_current_status = true;
    std::atomic<bool> monitor_input_exited = false;
    std::atomic<bool> running_thread_current_exited = false;
    void run();
    void monitor_input();

public:
    void cleanup();
    void initialize();
    void set_cursor(int, int);
    cursor_position_t get_cursor();
    void display_char(int, int, int);
    void set_cursor_visibility(int);
};

#endif //UI_CURSES_H