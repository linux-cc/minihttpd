#include "httpd/gzip.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    using httpd::GZip;
    GZip zip;
    if (argc < 2) {
        printf("usage %s [-level] file\n", argv[0]);
        return -1;
    }

    if (argv[1][0] == '-') {
        zip.setLevel(argv[1][1] - '0');
        if (argc == 3)
            zip.zip(argv[2]);
        else
            zip.zip(argv[2], argv[3]);
    } else if (argc == 2) {
        zip.zip(argv[1]);
    } else {
        zip.zip(argv[1], argv[2]);
    }

    return 0;
}
