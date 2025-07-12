

# uffCS_TALKS

A lightweight, high-performance Redis-inspired in-memory key-value store written in modern **C/C++**.

This project implements many core features of Redis—including support for Strings, Lists, and Hashes—with custom low-level data structures and efficient networking using **epoll** for I/O multiplexing. It is designed for educational purposes and to explore systems-level programming concepts.

---

## ✨ Features

- 🧠 **Custom HashMap**: Built from scratch with Murmur3 hash function for fast key lookup.
- 📦 **List Structures**: Includes `ListOpack` and `QuickList` to simulate Redis-like list operations.
- ⚡ **Efficient I/O**: Uses `epoll` to support multiple concurrent client connections.
- 🛠️ **Modular Codebase**: Clean separation between networking, data handling, and command processing.

---

## ✅ Supported Commands

### String Commands
- `GET key`  
- `SET key value`  
- `INCR key`  
- `DECR key`  
- `EXISTS key`  
- `DEL key`  
- `TYPE key`  
- `ECHO message`  
- `PING`

### List Commands
- `LPUSH key value`
- `RPUSH key value`
- `LPOP key`
- `RPOP key`
- `LLEN key`
- `LRANGE key start stop`

### Hash Commands
- `HSET key field value`
- `HGET key field`
- `HDEL key field`
- `HGETALL key`

---

## 🏗️ Build Instructions

### Prerequisites

- GCC or Clang
- CMake ≥ 3.10
- Linux environment (for `epoll`)

### Steps

```bash
git clone https://github.com/YashSuthar983/uffCS_TALKS.git
cd uffCS_TALKS
chmod +x run.sh
./run.sh
```

---

## 🧪 Example

```bash
> SET mykey Hello
+OK

> GET mykey
"Hello"

> LPUSH list 1 2 3
:3

> LRANGE list 0 2
1) "3"
2) "2"
3) "1"

> HSET myhash field1 value1
:1

> HGET myhash field1
"value1"
```

---

## 📁 Directory Structure

```bash
uffCS_TALKS/
├── src/               # Source code
├── run.sh             # Build & run script
├── CMakeLists.txt     # Build configuration
└── .gitignore
```

---

## 🚀 Roadmap

- [ ] Persistence (RDB / AOF)
- [ ] Pub/Sub support
- [ ] Auth system
- [ ] More hash and list operations
- [ ] Cluster mode simulation

---

## 🤝 Contribution

PRs are welcome! If you find any issues or want to suggest improvements, feel free to open an issue or submit a pull request.

---

## 📜 License

This project is licensed under the MIT License.
