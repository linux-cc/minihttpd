#include "httpd/fcgid.h"
#include "httpd/connection.h"
#include <unistd.h>
#include <errno.h>

namespace httpd {

using memory::SimpleAlloc;

static bool showHelp(const char *appname) {
    _LOG_("Usage: %s [options] [arguments]", appname);
    _LOG_("-a <address> bind to unix domain socket address (default localhost)");
    _LOG_("-p <port> bind to unix domain socket port (default 9090)");
    _LOG_("-c <children> number of children to fork (default 1)");
    _LOG_("-h show this help");
    
    return false;
}

bool FastCgid::init(int argc, char *argv[]) {
    char o;
    const char *host = "localhost";
    const char *service = "9090";
    _children = 1;
    
    while ((o = getopt(argc, argv, "f:a:p:c:h")) != -1) {
        switch (o) {
            case 'f': _cgiFile = optarg; break;
            case 'a': host = optarg; break;
            case 'p': service = optarg; break;
            case 'c': _children = atoi(optarg); break;
            case 'h':
            default: return showHelp(argv[0]);
        }
    }
    if (_cgiFile.empty()) {
        _LOG_("no FastCGI application given");
        return false;
    }
    
    return _socket.create(host, service, Socket::F_LOCAL);
}

void FastCgid::run() {
    while (_children--) {
        pid_t pid = fork();
        if (pid < 0) {
            _LOG_("fork error:%d:%s", errno, strerror(errno));
            return;
        } else if (pid == 0) {
            process();
        } else {
            waitpid(pid);
        }
    }
}

void FastCgid::waitpid(pid_t pid) {
    int status = 0;
    struct timeval tv = { 0, 100 * 1000 };
    
    select(0, NULL, NULL, NULL, &tv);
    int e = ::waitpid(pid, &status, WNOHANG);
    if (e == 0) {
        _LOG_("fork child successfully, pid: %d", pid);
    } else if (WIFEXITED(status)) {
        _LOG_("child exited with status: %d", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        _LOG_("child signaled: %d", WTERMSIG(status));
    } else {
        _LOG_("child died with status: %d", status);
    }
}

void FastCgid::process() {
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    dup2(_socket, STDIN_FILENO);
    _socket.close();
    execl(_cgiFile.data(), NULL);
}

static Connection *_client = NULL;
static char **_envp = NULL;

int fcgiAccept() {
    if (!_client) {
        _client = SimpleAlloc<Connection>::New();
    }
    _client->close();
    int fd = TcpSocket(STDIN_FILENO).accept();
    if (fd < 0) {
        _LOG_("accept error: %d:%s", errno, strerror(errno));
        return -1;
    }
    _client->attach(fd);
    
    FcgiHeader header;
    _client->recv(&header, sizeof(header));
    uint32_t length = header.getLength();
    uint32_t dataPos = header.getDataPos();
    
    return 1;
}

}

