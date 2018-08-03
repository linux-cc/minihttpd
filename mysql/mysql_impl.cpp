#include "mysql_impl.h"

namespace myframe {
namespace mysql {

Mysql::Row::Row(MYSQL_ROW row, int num) {
    for (int i = 0; i < num; ++i) {
        _fields.push_back(row[i]);
    }
}

string Mysql::Row::dump() const {
    string result;
    for (int i = 0; i < size(); ++i) {
        if (result.empty()) {
            result = _fields[i];
        } else {
            result += ", " + _fields[i];
        }
    }
    return result + '\n';
}

int Mysql::execute(const char *sql) {
    if (mysql_query(&_mysql, sql)) {
        return -1;
    }
    MYSQL_RES *result = mysql_store_result(&_mysql);
    if (result) {
        int num = mysql_num_fields(result);
        MYSQL_FIELD *fs = mysql_fetch_fields(result);
        for (int i = 0; i < num; ++i) {
            Field f(fs[i].name, fs[i].org_name, i);
            _fields.push_back(f);
        }
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            Row r(row, num);
            _rows.push_back(r);
        }
        mysql_free_result(result);
        return _rows.size();
    } else if (mysql_field_count(&_mysql) == 0) {
        //query was not a SELECT-like statement
        return mysql_affected_rows(&_mysql);
    }
    return -1;
}

string Mysql::dump() const {
    string result;
    for (int i = 0; i < (int)_fields.size(); ++i) {
        if (result.empty()) {
            result = _fields[i]._name;
        } else {
            result += ", " + _fields[i]._name;
        }
    }
    result += '\n';
    for (Iterator it = begin(); it != end(); ++it) {
        result += it->dump();
    }
    return result;
}

} /* namespace mysql */
} /* namespace myframe */

#ifdef __MYSQL_MAIN__

#include <stdio.h>
int main(int argc, char *argv[]) {
    using myframe::mysql::Mysql;
    Mysql mysql;
    mysql.connectTimeout(3);
    if (!mysql.connect("linshh", "linshh", "80.209.232.2", 3306)) {
        printf("connect error: %d:%s\n", mysql.errno(), mysql.error());
        return -1;
    }
    if (!mysql.selectDb(argv[1])) {
        printf("selectDb error: %d:%s\n", mysql.errno(), mysql.error());
        return -1;
    }
    int ret = mysql.execute(argv[2]);
    if (ret == -1) {
        printf("execute error: %d:%s\n", mysql.errno(), mysql.error());
        return -1;
    }
    printf("%s\n", mysql.dump().c_str());
    return 0;
}
#endif
