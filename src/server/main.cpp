// Minimal dependency-free HTTP static file server in pure C++ (POSIX).
// Serves the wasm build output (index.html, ss.js, ss.wasm) on the LAN.
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

static std::string nowStamp() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	struct tm tmv;
	localtime_r(&ts.tv_sec, &tmv);
	char buf[32];
	size_t n = strftime(buf, sizeof buf, "%H:%M:%S", &tmv);
	snprintf(buf + n, sizeof buf - n, ".%03ld", ts.tv_nsec / 1000000);
	return buf;
}

static const char* mime(const std::string& path) {
	auto ext = [&]() {
		auto pos = path.rfind('.');
		return pos == std::string::npos ? "" : path.substr(pos);
	};
	std::string e = ext();
	if (e == ".html")
		return "text/html; charset=utf-8";
	if (e == ".js" || e == ".mjs")
		return "text/javascript; charset=utf-8";
	if (e == ".wasm")
		return "application/wasm";
	if (e == ".svg")
		return "image/svg+xml";
	if (e == ".png")
		return "image/png";
	if (e == ".jpg" || e == ".jpeg")
		return "image/jpeg";
	if (e == ".css")
		return "text/css; charset=utf-8";
	if (e == ".json")
		return "application/json";
	if (e == ".txt")
		return "text/plain; charset=utf-8";
	if (e == ".ico")
		return "image/x-icon";
	if (e == ".map")
		return "application/json";
	return "application/octet-stream";
}

static bool readAll(int fd, std::string& out) {
	char buf[16384];
	ssize_t n;
	while ((n = read(fd, buf, sizeof buf)) > 0)
		out.append(buf, (size_t)n);
	return n == 0;
}

static size_t sendStr(int fd, const std::string& s) {
	size_t off = 0;
	while (off < s.size()) {
		ssize_t n = write(fd, s.data() + off, s.size() - off);
		if (n <= 0)
			break;
		off += (size_t)n;
	}
	return off;
}

static ssize_t serveFile(int fd, const std::string& root, const std::string& reqPath) {
	// sanitize: no "..", no query/fragment, default to index.html for dirs
	std::string path = reqPath;
	if (path.find("..") != std::string::npos)
		return false;
	size_t q = path.find_first_of("?#");
	if (q != std::string::npos)
		path = path.substr(0, q);
	if (path.empty() || path == "/")
		path = "/index.html";
	if (path[0] != '/')
		path = "/" + path;

	std::string full = root + path;
	struct stat st;
	if (stat(full.c_str(), &st) != 0 || S_ISDIR(st.st_mode)) {
		if (S_ISDIR(st.st_mode))
			full += "/index.html";
		else
			return -1;
		if (stat(full.c_str(), &st) != 0)
			return -1;
	}

	int f = open(full.c_str(), O_RDONLY);
	if (f < 0)
		return -1;

	std::string body;
	bool ok = readAll(f, body);
	close(f);
	if (!ok)
		return -1;

	char hdr[512];
	snprintf(
		hdr,
		sizeof hdr,
		"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
		"Cache-Control: no-cache\r\nConnection: close\r\n\r\n",
		mime(full),
		body.size());
	size_t sent = sendStr(fd, hdr);
	sent += sendStr(fd, body);
	return (ssize_t)sent;
}

static void handleConn(int fd, const std::string& root, const std::string& peer) {
	std::string req;
	char buf[8192];
	ssize_t n = recv(fd, buf, sizeof buf, 0);
	if (n <= 0)
		return;
	req.assign(buf, (size_t)n);

	size_t eol = req.find("\r\n");
	std::string reqLine = eol == std::string::npos ? req : req.substr(0, eol);
	printf("[%s] %s REQ %s\n", nowStamp().c_str(), peer.c_str(), reqLine.c_str());
	fflush(stdout);

	// parse "GET /path HTTP/1.1"
	if (req.rfind("GET ", 0) != 0) {
		size_t sent = sendStr(
			fd,
			"HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n"
			"Connection: close\r\n\r\n");
		printf("[%s] %s RES %s -> 405 (%zu bytes)\n", nowStamp().c_str(), peer.c_str(), reqLine.c_str(), sent);
		fflush(stdout);
		return;
	}
	size_t p1 = req.find(' ', 4);
	std::string path = req.substr(4, p1 - 4);
	ssize_t sent = serveFile(fd, root, path);
	if (sent < 0) {
		std::string body = "404 Not Found\n";
		char hdr[256];
		snprintf(
			hdr,
			sizeof hdr,
			"HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n"
			"Content-Length: %zu\r\nConnection: close\r\n\r\n",
			body.size());
		size_t n1 = sendStr(fd, hdr);
		size_t n2 = sendStr(fd, body);
		printf(
			"[%s] %s RES %s -> 404 (%zu bytes)\n",
			nowStamp().c_str(),
			peer.c_str(),
			path.c_str(),
			n1 + n2);
	} else {
		printf("[%s] %s RES %s -> 200 (%zd bytes)\n", nowStamp().c_str(), peer.c_str(), path.c_str(), sent);
	}
	fflush(stdout);
}

int main(int argc, char** argv) {
	int port = 8080;
	std::string root = "web";
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--port") && i + 1 < argc)
			port = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--root") && i + 1 < argc)
			root = argv[++i];
	}

	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		perror("socket");
		return 1;
	}
	int one = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons((uint16_t)port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sfd, (sockaddr*)&addr, sizeof addr) < 0 || listen(sfd, 16) < 0) {
		perror("bind/listen");
		return 1;
	}
	printf("shan-shui server: http://0.0.0.0:%d  root=%s\n", port, root.c_str());

	for (;;) {
		sockaddr_in ca{};
		socklen_t cl = sizeof ca;
		int cfd = accept(sfd, (sockaddr*)&ca, &cl);
		if (cfd < 0)
			continue;
		char peer[64] = "-";
		inet_ntop(AF_INET, &ca.sin_addr, peer, sizeof peer);
		handleConn(cfd, root, peer);
		close(cfd);
	}
}
