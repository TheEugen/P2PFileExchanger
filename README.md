# 📡 P2PFileExchanger

A Windows peer-to-peer file transfer tool written in C++. Both peers run the same binary — one acts as the receiver, the other initiates the send. Files are streamed directly over TCP with no intermediary server.

---

## Features

- **Direct TCP transfer** — Files are sent peer-to-peer over TCP on port 27015 using Winsock2, with no relay or central server
- **Accept/decline prompt** — The receiving peer is interactively asked to accept or decline each incoming transfer request
- **Public IP lookup** — Automatically fetches the machine's public IP via `api.ipify.org` (WinHTTP) so you know what address to share with the other peer
- **Chunked streaming** — Files are read and sent in 4096-byte chunks, supporting large files up to and beyond 4 GB
- **Handshake protocol** — The sender first transmits a request packet (file size + file path), waits for confirmation, then begins streaming
- **Win32 file I/O** — Uses `CreateFileA`, `ReadFile`, and `WriteFile` directly for efficient file handling

---

## Project Structure

```
P2PFileExchanger/
├── Main.cpp        # Entry point, run loop, CLI input parsing
├── Server.cpp/.h   # Receiver logic: listen, accept/decline, write to disk
├── Client.cpp/.h   # Sender logic: connect, handshake, stream file
├── FileIO.cpp/.h   # Win32 file abstraction (open, read, write, size)
├── Utils.h         # Public IP fetch via WinHTTP (api.ipify.org)
├── Resources.h     # Shared constants (port, buffer sizes)
└── Cleanup.h       # Error handling and resource cleanup helpers
```

---

## Getting Started

### Prerequisites

- Windows
- Visual Studio (MSVC) — uses Winsock2 and Win32 APIs

### Build

```bash
git clone https://github.com/TheEugen/P2PFileExchanger.git
```

Open the project in Visual Studio and build. Link against `ws2_32.lib` and `winhttp.lib` (the latter is already pulled in via `#pragma comment` in `Utils.h`).

---

## Usage

Both peers run the same executable. At startup, **the receiving peer** enters `s` to start the server thread listening for incoming connections. **The sending peer** skips that prompt and uses the send command in the main loop:

```
send <Path-To-File> <IP-Address>
```

If the file path contains spaces, wrap it in quotes:

```
send "C:\My Files\document.pdf" 203.0.113.42
```

### Transfer flow

1. Sender connects to the receiver's IP on port 27015
2. Sender transmits a request packet containing the file size and path
3. Receiver is prompted to accept (`y`) or decline (`n`)
4. If accepted, the sender streams the file in 4096-byte chunks
5. Receiver writes chunks to disk and sends a completion acknowledgement
6. Both sides return to the main loop

---

## Known Limitations / Work in Progress

- The destination file path on the receiver side is currently hardcoded — configurable save paths are not yet implemented
- Only one transfer per session is handled (multi-request loop is implemented but commented out)
- No encryption — transfers are plaintext over TCP

---

## Roadmap

- [ ] Configurable save path on the receiver side
- [ ] Support multiple sequential transfer requests per session
- [ ] Encrypted transfers
- [ ] GUI for easier use
