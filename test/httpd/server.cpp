#include "httpd/server.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    USING_CLASS(httpd, Server);
    Server svr;
    if (argc == 3)
        svr.start(atoi(argv[1]), atoi(argv[2]), 10);
    else
        svr.start(2, 8, 10);

    svr.run();

    return 0;
}
