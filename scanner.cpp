// interactive_scanner.cpp
// Minimal interactive TCP port scanner (prompts if no args).
// Build: g++ -std=c++11 interactive_scanner.cpp -o interactive_scanner

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

bool resolve_to_ipv4(const std::string &host, std::string &out_ip) {
    struct in_addr addr4;
    if (inet_pton(AF_INET, host.c_str(), &addr4) == 1) {
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr4, buf, sizeof(buf));
        out_ip = buf;
        return true;
    }
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0) return false;
    char buf[INET_ADDRSTRLEN];
    struct sockaddr_in *sa = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
    inet_ntop(AF_INET, &(sa->sin_addr), buf, sizeof(buf));
    out_ip = buf;
    freeaddrinfo(res);
    return true;
}

bool connect_with_timeout(const std::string &ip, int port, int timeout_ms) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) flags = 0;
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, ip.c_str(), &sa.sin_addr) != 1) {
        close(sock);
        return false;
    }

    int rc = connect(sock, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa));
    if (rc == 0) { close(sock); return true; }
    if (errno != EINPROGRESS) { close(sock); return false; }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sock, &wfds);
    struct timeval tv{};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    rc = select(sock + 1, nullptr, &wfds, nullptr, &tv);
    if (rc <= 0) { close(sock); return false; }

    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0) { close(sock); return false; }
    close(sock);
    return (err == 0);
}

int main(int argc, char **argv) {
    std::string target;
    int start_port = 1;
    int end_port = 100;
    int timeout_ms = 300;

    // If target not provided as argv, prompt interactively
    if (argc >= 2) {
        target = argv[1];
        if (argc >= 3) start_port = std::stoi(argv[2]);
        if (argc >= 4) end_port = std::stoi(argv[3]);
        if (argc >= 5) timeout_ms = std::stoi(argv[4]);
    } else {
        std::cout << "Enter target IP or hostname: ";
        std::getline(std::cin, target);
        if (target.empty()) {
            std::cerr << "No target provided. Exiting.\n";
            return 1;
        }
        std::cout << "Start port (default 1): ";
        std::string s;
        std::getline(std::cin, s);
        if (!s.empty()) start_port = std::stoi(s);
        std::cout << "End port (default 100): ";
        std::getline(std::cin, s);
        if (!s.empty()) end_port = std::stoi(s);
        std::cout << "Timeout ms (default 300): ";
        std::getline(std::cin, s);
        if (!s.empty()) timeout_ms = std::stoi(s);
    }

    if (start_port < 1) start_port = 1;
    if (end_port > 65535) end_port = 65535;
    if (end_port < start_port) std::swap(start_port, end_port);

    std::string ip;
    if (!resolve_to_ipv4(target, ip)) {
        std::cerr << "Cannot resolve target: " << target << "\n";
        return 2;
    }

    std::cout << "Scanning " << target << " (" << ip << ") ports " << start_port << "-" << end_port
              << " with timeout " << timeout_ms << " ms\n\n";

    bool found_any = false;
    for (int p = start_port; p <= end_port; ++p) {
        if (connect_with_timeout(ip, p, timeout_ms)) {
            std::cout << "Port " << p << " is OPEN\n";
            found_any = true;
        }
    }

    if (!found_any) std::cout << "\nNo open ports found in range.\n";
    else std::cout << "\nScan complete.\n";

    return 0;
}
