#ifndef __MYSQL_MYSQL_IMPL_H__
#define __MYSQL_MYSQL_IMPL_H__

#include <mysql.h>
#include <stdint.h>
#include <vector>
#include <string>

namespace mysql {

using std::string;
using std::vector;

class Mysql {
public:
    struct Row {
        Row(MYSQL_ROW row, int num);
        const string &operator[](int index) const {
            return _fields[index];
        }
        int size() const {
            return _fields.size();
        }
        typedef vector<string>::const_iterator Iterator;
        Iterator begin() const {
            return _fields.begin();
        }
        Iterator end() const {
            return _fields.end();
        }
        string dump() const;
        vector<string> _fields;
    };
public:
    Mysql() {
        mysql_init(&_mysql);
    }
    bool connect(const char *user, const char *passwd, const char *host, int port) {
        return mysql_real_connect(&_mysql, host, user, passwd, NULL, port, NULL, 0) != NULL;
    }
    uint64_t lastInsertId() {
        return mysql_insert_id(&_mysql);
    }
    bool selectDb(const char *dbname) {
        return mysql_select_db(&_mysql, dbname) == 0;
    }
    void close() {
        mysql_close(&_mysql);
    }
    void connectTimeout(int seconds) {
        option(MYSQL_OPT_CONNECT_TIMEOUT, &seconds);
    }
    void readTimeout(int seconds) {
        option(MYSQL_OPT_READ_TIMEOUT, &seconds);
    }
    void writeTimeout(int seconds) {
        option(MYSQL_OPT_WRITE_TIMEOUT, &seconds);
    }
    int option(mysql_option option, const void *arg) {
        return mysql_options(&_mysql, option, arg);
    }
    int errcode() {
        return mysql_errno(&_mysql);
    }
    const char *errinfo() {
        return mysql_error(&_mysql);
    }
    int size() const {
        return _rows.size();
    }
    const Row &operator[](int index) const {
        return _rows[index];
    }
    typedef vector<Row>::const_iterator Iterator;
    Iterator begin() const {
        return _rows.begin();
    }
    Iterator end() const {
        return _rows.end();
    }
    int execute(const char *sql);
    string dump() const;

private:
    struct Field {
        Field(const string &name, const string &orgName, int index):
            _name(name), _orgName(orgName), _index(index) {}
        string _name;
        string _orgName;
        int _index;
    };
    MYSQL _mysql;
    vector<Field> _fields;
    vector<Row> _rows;
};

} /* namespace mysql */
#endif /* ifndef __MYSQL_MYSQL_IMPL_H__ */
