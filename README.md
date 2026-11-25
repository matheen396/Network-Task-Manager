# Netask — Multithreaded Network Task Manager

A small, runnable C++20 client/server task manager for learning and portfolio use. The server accepts multiple concurrent TCP clients, serializes access to shared task state, and writes each change to a local data file. It targets Windows first and also builds on Linux/macOS.

## Features

- Task create, list, edit, assign, status update, and delete
- TCP client/server using Winsock2 on Windows and BSD sockets elsewhere
- One worker thread per connected client; synchronized in-memory task manager
- File persistence with temporary-file replacement and percent-escaped fields
- CMake build, dependency-free unit tests, and Docker image
- Newline-delimited, pipe-separated protocol for easy inspection

## Requirements

- CMake 3.20 or later
- C++20 compiler (Visual Studio 2022, GCC 11+, or Clang 14+ recommended)

## Build and test on Windows

From PowerShell in this folder:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The executables are in `build\Release\` with the Visual Studio generator, or `build\` with a single-configuration generator.

## Run locally

Start the server in one PowerShell window:

```powershell
.\build\Release\netask_server.exe 5050
```

In another window, use the client (replace the path if using a single-configuration generator):

```powershell
.\build\Release\netask.exe 127.0.0.1 5050 ping
.\build\Release\netask.exe 127.0.0.1 5050 create "Ship release" "Cut version 1.0" "Ada"
.\build\Release\netask.exe 127.0.0.1 5050 list
.\build\Release\netask.exe 127.0.0.1 5050 assign 1 "Grace"
.\build\Release\netask.exe 127.0.0.1 5050 status 1 in_progress
.\build\Release\netask.exe 127.0.0.1 5050 update 1 "Ship v1" "Release checklist complete"
.\build\Release\netask.exe 127.0.0.1 5050 status 1 done
.\build\Release\netask.exe 127.0.0.1 5050 delete 1
```

By default, data is stored in `data/tasks.db` relative to the server's working directory. Set `NETASK_DATA` to choose another file. The server listens on all interfaces; only expose it on a trusted network because this starter protocol has no authentication or encryption.

## Protocol

Each request is one line; each response is one line, with task-list responses containing additional task lines. Separate fields with `|`; percent-encode `%`, `|`, CR, and LF as `%25`, `%7C`, `%0D`, and `%0A`.

```text
PING
LIST
CREATE|title|description|assignee
UPDATE|id|title|description
ASSIGN|id|assignee
STATUS|id|todo|in_progress|done
DELETE|id
```

Responses begin with `OK|` or `ERR|`. IDs are allocated by the server. The server keeps task records in memory and persists changes to a single file. Run one server process against a given data file; concurrent multi-process writes are not coordinated.

## Deploy with Docker on a TCP-capable VPS

Vercel Functions are request-driven functions documented around HTTP handlers; they are not the right place to run this always-on raw TCP listener. Render web services route public traffic as HTTP. Render private services can listen on arbitrary TCP ports over Render's private network, so they are an option only when clients also run on that private network; they do not give this server a public TCP endpoint. For direct public client connections, use a Linux VPS or container host that explicitly supports public inbound TCP ports and persistent storage.

1. Push this repository to GitHub.
2. On an Ubuntu VPS, install Docker and clone the repository.
3. Build the image: `docker build -t netask:1.0 .`
4. Create a persistent directory and run: `docker run -d --name netask --restart unless-stopped -p 5050:5050 -v netask-data:/data netask:1.0`
5. Allow inbound TCP port `5050` in the provider firewall only for trusted client IPs. Keep SSH access restricted too.
6. Connect with `netask <server-public-ip> 5050 list` from a built client.

Render's private-service option is suitable when the client is another service in the same Render region/network. See [Render private services](https://render.com/docs/private-services) and [Render private networking](https://render.com/docs/private-network). Vercel's documented [Functions model](https://vercel.com/docs/functions) is request/invocation based.

Keep the volume mounted at `/data`; it holds `tasks.db` across container restarts. For internet-facing production use, add authentication, TLS (or a VPN/private network), request limits, structured rotating logs, backups, and a process supervisor/health checks before opening the port publicly.

## Repository layout

```text
include/netask/   public interfaces
src/              implementation and executable entry points
tests/            dependency-free unit tests
Dockerfile        reproducible Linux container build
CMakeLists.txt    build targets and CTest registration
```

## License

MIT; see [LICENSE](LICENSE).
