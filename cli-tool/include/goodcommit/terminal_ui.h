#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <csignal>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

#ifdef _WIN32

static void enable_vt()
{
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE)
        return;
    DWORD mode = 0;
    if (!GetConsoleMode(h, &mode))
        return;
    SetConsoleMode(h, mode | 0x0004);
}

#else

static void enable_vt() {}

#endif

#define GOODCOMMIT_LOGO_LINES 6

static void print_logo()
{
    const char *GIT_COLOR = "\033[38;2;241;78;50m";
    const char *RESET_COLOR = "\033[0m";

    std::cout
        << GIT_COLOR << "  ██████╗  ██████╗  ██████╗ ██████╗ " << RESET_COLOR << " ██████╗ ██████╗ ███╗   ███╗███╗   ███╗██╗████████╗\n"
        << GIT_COLOR << " ██╔════╝ ██╔═══██╗██╔═══██╗██╔══██╗" << RESET_COLOR << "██╔════╝██╔═══██╗████╗ ████║████╗ ████║██║╚══██╔══╝\n"
        << GIT_COLOR << " ██║  ███╗██║   ██║██║   ██║██║  ██║" << RESET_COLOR << "██║     ██║   ██║██╔████╔██║██╔████╔██║██║   ██║   \n"
        << GIT_COLOR << " ██║   ██║██║   ██║██║   ██║██║  ██║" << RESET_COLOR << "██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██║   ██║   \n"
        << GIT_COLOR << " ╚██████╔╝╚██████╔╝╚██████╔╝██████╔╝" << RESET_COLOR << "╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║██║   ██║   \n"
        << GIT_COLOR << "  ╚═════╝  ╚═════╝  ╚═════╝ ╚═════╝ " << RESET_COLOR << " ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚═╝   ╚═╝   \n";
}

static void erase_logo(int tail_lines)
{
    int up = 8 + tail_lines;
    int down = tail_lines + 2;
    std::cout << "\033[" << up << "A\033[" << GOODCOMMIT_LOGO_LINES << "M\033[" << down << "B" << std::flush;
}

static void print_exit_reason(const std::string &reason)
{
    std::cout << "Exited with \033[2m" << reason << "\033[0m\n";
}

static void sig_handler(int)
{
    
    std::cout << "\033[?25h\n";
    print_exit_reason("ctrl+c");
    fflush(stdout);
    exit(130);
}

static std::atomic<bool> spinner_go{false};
static std::jthread spinner_thread;

static void spinner_func(const std::string &text)
{
    const char frames[] = "|/-\\";
    int i = 0;

    while (spinner_go)
    {
        
        std::cout << "\r  " << frames[i] << " " << text << std::flush;

        
        std::this_thread::sleep_for(std::chrono::milliseconds(80));

        
        
        i = (i + 1) % 4;
    }
}

static void start_spinner(const std::string &text)
{
    spinner_go = true;
    spinner_thread = std::jthread(spinner_func, text);
}

static void stop_spinner()
{
    spinner_go = false;
    if (spinner_thread.joinable())
    {
        spinner_thread.join();
    }
    std::cout << "\r\033[2K" << std::flush;
}

#ifdef _WIN32

static int select_option(const std::vector<std::string> &options)
{
    int selected = 0;
    enable_vt();

    std::cout << "\033[?25l" << std::flush;

    int total_lines = (int)options.size() + 1;
    bool first_draw = true;

    while (true)
    {
        if (!first_draw)
        {
            std::cout << "\033[" << total_lines << "A\r";
        }
        std::cout << "\033[J";

        for (int i = 0; i < (int)options.size(); i++)
        {
            if (i == selected)
            {
                std::cout << "\033[1;32m  > " << options[i] << "\033[0m\n";
            }
            else
            {
                std::cout << "    " << options[i] << "\n";
            }
        }
        std::cout << "  \033[90m(up/down to navigate, Enter to confirm)\033[0m\n"
                  << std::flush;
        first_draw = false;

        int ch = _getch();
        if (ch == 0 || ch == 224)
        {
            ch = _getch();
            if (ch == 72)
            {
                selected = (selected - 1 + (int)options.size()) % (int)options.size();
            }
            else if (ch == 80)
            {
                selected = (selected + 1) % (int)options.size();
            }
            continue;
        }

        if (ch == 3)
        {
            std::cout << "\033[?25h\n" << std::flush;
            print_exit_reason("ctrl+c");
            return -1;
        }

        if (ch == 27)
        {
            std::cout << "\033[?25h\n" << std::flush;
            print_exit_reason("esc");
            return -1;
        }

        if (ch == '\r' || ch == '\n')
        {
            std::cout << "\033[?25h" << std::flush;
            std::cout << "\n" << std::flush;
            return selected;
        }
    }
}

#else

static int select_option(const std::vector<std::string> &options)
{
    int selected = 0;

    
    struct termios orig_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG); 
    raw.c_iflag &= ~(IXON);                 
    raw.c_cc[VMIN] = 0;                     
    raw.c_cc[VTIME] = 1;                    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    
    std::cout << "\033[?25l" << std::flush;

    int total_lines = (int)options.size() + 1;
    bool first_draw = true;

    while (true)
    {
        
        if (!first_draw)
        {
            std::cout << "\033[" << total_lines << "A\r";
        }
        std::cout << "\033[J"; 

        
        for (int i = 0; i < (int)options.size(); i++)
        {
            if (i == selected)
            {
                std::cout << "\033[1;32m  > " << options[i] << "\033[0m\n";
            }
            else
            {
                std::cout << "    " << options[i] << "\n";
            }
        }
        std::cout << "  \033[90m(up/down to navigate, Enter to confirm)\033[0m\n"
                  << std::flush;
        first_draw = false;

        
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1)
            continue;

        if (c == 27)
        {
            
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1)
            {
                std::cout << "\033[?25h\n";
                print_exit_reason("esc");
                break;
            }
            if (read(STDIN_FILENO, &seq[1], 1) != 1)
            {
                std::cout << "\033[?25h\n";
                print_exit_reason("esc");
                break;
            }
            if (seq[0] == '[')
            {
                if (seq[1] == 'A')
                {
                    
                    selected = (selected - 1 + (int)options.size()) % (int)options.size();
                }
                else if (seq[1] == 'B')
                {
                    
                    selected = (selected + 1) % (int)options.size();
                }
            }
        }
        else if (c == 3)
        {
            
            std::cout << "\033[?25h\n"; 
            print_exit_reason("ctrl+c");
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
            return -1;
        }
        else if (c == '\r' || c == '\n')
        {
            
            std::cout << "\033[?25h"; 
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
            return selected;
        }
    }

    
    std::cout << "\033[?25h";
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    return -1;
}

#endif

static int select_option_numbered(const std::vector<std::string> &options)
{
    std::cout << "\nSelect a commit message:\n\n";
    for (size_t i = 0; i < options.size(); i++)
    {
        std::cout << "  " << (i + 1) << ". " << options[i] << "\n";
    }
    std::cout << "\nEnter number (1-" << options.size() << "): " << std::flush;

    std::string input;
    std::getline(std::cin, input);

    if (input.empty())
        return -1;

    
    int choice = 0;
    for (char c : input)
    {
        if (c < '0' || c > '9')
            return -1;
        choice = choice * 10 + (c - '0');
    }

    if (choice < 1 || choice > (int)options.size())
        return -1;
    return choice - 1;
}

static void run_test()
{
    enable_vt();

    
    start_spinner("Testing loader... (press Enter to stop)");
    std::cout << "\n"
              << std::flush;

    
    std::cout << "Press Enter to continue..." << std::flush;
    std::string buf;
    std::getline(std::cin, buf);

    stop_spinner();

    
    std::vector<std::string> options = {"option 1", "option 2", "option 3"};
    int idx = select_option(options);

    if (idx == -1)
    {
        return;
    }

    std::cout << "\nSelected: " << options[idx] << "\n";
}

#ifdef _WIN32

static std::string read_password() {
    std::string key;
    while (true) {
        int ch = _getch();
        if (ch == '\r' || ch == '\n') break;
        if (ch == 3) { std::cout << "\n"; print_exit_reason("ctrl+c"); exit(130); }
        if (ch == 27) { std::cout << "\n"; print_exit_reason("esc"); exit(130); }
        if (ch == 8 || ch == 127) {
            if (!key.empty()) {
                key.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }
        if (ch == 0 || ch == 224) { _getch(); continue; }
        if (ch >= 32 && ch < 127) {
            key += (char)ch;
            std::cout << "*" << std::flush;
        }
    }
    return key;
}

#else

static std::string read_password() {
    struct termios orig, raw;
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    std::string key;
    while (true) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) continue;
        if (c == '\n' || c == '\r') break;
        if (c == 3) { std::cout << "\n"; tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig); print_exit_reason("ctrl+c"); exit(130); }
        if (c == 27) { std::cout << "\n"; tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig); print_exit_reason("esc"); exit(130); }
        if (c == 127 || c == 8) {
            if (!key.empty()) {
                key.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }
        if (c >= 32 && c < 127) {
            key += c;
            std::cout << "*" << std::flush;
        }
    }
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    return key;
}

#endif

#endif
