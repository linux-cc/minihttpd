#include "httpd/gzip.h"

int main(int argc, char *argv[]) {
    USING_CLASS(httpd, GZip);
    GZip zip;
    if (argc < 2) {
        _LOG_("usage %s file\n", argv[0]);
        return -1;
    }
    if (argc == 3)
        zip.zip(argv[1], argv[2]);
    else
        zip.zip(argv[1]);

    return 0;
}
