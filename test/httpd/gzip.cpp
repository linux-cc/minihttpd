#include "httpd/gzip.h"

int main(int argc, char *argv[]) {
    using httpd::GZip;
    GZip zip;
    if (argc < 2) {
        _LOG_("usage %s [-level] file\n", argv[0]);
        return -1;
    }

    if (argv[1][0] == '-') {
        zip.setLevel(argv[1][1] - '0');
        if (argc == 3)
            zip.init(argv[2]);
        else
            zip.init(argv[2], argv[3]);
    } else if (argc == 2) {
        zip.init(argv[1]);
    } else {
        zip.init(argv[1], argv[2]);
    }
    zip.zip();

    return 0;
}
