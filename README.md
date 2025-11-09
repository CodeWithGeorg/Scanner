# Interactive_scanner

A minimal, single-file **interactive TCP port scanner** in C++ (IPv4).  
The program prompts for a target IP/hostname and optional port range and timeout when run without command-line arguments. Useful for learning how port scanning works and for quick LAN checks.

> **Warning:** Only scan systems you own or have explicit permission to test. Unauthorized scanning may be illegal.

## Features
- Interactive prompt when run with no arguments
- Accepts command-line arguments for scripted use
- Uses non-blocking `connect()` + `select()` to implement per-port timeouts
- IPv4 hostname resolution (via `getaddrinfo`)
- Small and easy to read — great for learning

## Requirements
- Linux (Kali recommended) or any POSIX-compatible OS
- `g++` (supports `-std=c++11`)

Install build essentials on Debian/Ubuntu/Kali:

```bash
sudo apt update
sudo apt install -y build-essential

```
Compile
```
g++ -std=c++11 interactive_scanner.cpp -o interactive_scanner
```
Run
```
./interactive_scanner
```

## Exammple of an output
```

Scanning example.com (93.184.216.34) ports 1-100 with timeout 300 ms

Port 22 is OPEN
Port 80 is OPEN

Scan complete.


