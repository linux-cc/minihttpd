#include "httpd/server.h"

static bool _quit = false;

int main(int argc, char *argv[]) {
    USING_CLASS(httpd, Server);
    Server svr;
    if (argc == 3)
        svr.start(atoi(argv[1]), atoi(argv[2]));
    else
        svr.start(4, 32);

    while (!_quit) {
        sleep(1);
    }
    return 0;
}
