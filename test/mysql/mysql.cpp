#include "mysql/mysql.h"
#include <stdio.h>

USING_NS(mysql);

int main(int argc, char *argv[]) {
    Mysql mysql;
    mysql.connectTimeout(3);
    if (!mysql.connect("linshh", "linshh", "80.209.232.2", 3306)) {
        printf("connect errinfo: %d:%s\n", mysql.errcode(), mysql.errinfo());
        return -1;
    }
    if (!mysql.selectDb(argv[1])) {
        printf("selectDb errinfo: %d:%s\n", mysql.errcode(), mysql.errinfo());
        return -1;
    }
    int ret = mysql.execute(argv[2]);
    if (ret == -1) {
        printf("execute errinfo: %d:%s\n", mysql.errcode(), mysql.errinfo());
        return -1;
    }
    printf("%s\n", mysql.dump().c_str());
    return 0;
}
