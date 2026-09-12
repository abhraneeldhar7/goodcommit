#include "xommit/commands.h"
#include "xommit/terminal_ui.h"

#include <csignal>

int main(int argc, char* argv[]) {
#ifdef _WIN32
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
#else
    struct sigaction sa = {};
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
#endif
    enable_vt();
    return dispatch(argc, argv);
}
