#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include <cstdlib>
#include <csignal>

static void sig_handler(int) {
    exit(130);
}

#endif
