#include "httpd/server.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    using httpd::Server;
    Server svr(4, 8, 10);
    if (argc == 3)
        svr.start(argv[1], argv[2]);
    else if (argc == 2)
        svr.start("localhost", argv[1]);
    else
        svr.start("localhost", "9090");

    svr.run();

    return 0;
}
