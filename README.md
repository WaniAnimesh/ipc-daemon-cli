# IPC Daemon & CLI System (Windows / C++)

This project implements a daemon process and a command-line client that communicate using Windows Named Pipes.

The daemon maintains an in-memory set of positive integers, each associated with a Unix timestamp of insertion.
Multiple clients can connect concurrently.

## Features

- Insert a positive integer (unique only)
- Delete a specific integer
- Print all stored integers in sorted order with timestamps
- Clear all entries
- Supports multiple simultaneous clients
- Thread-safe daemon

## IPC Mechanism

Windows Named Pipes (`\\\\.\\pipe\\MyIpcPipe`) are used for inter-process communication.
Each client connection is handled in a dedicated thread.

## Data Structure

The daemon uses:

std::map<int, long long>

1. Keys are unique by definition (prevents duplicates)

2. Sorted by default

3. Efficient insert, delete, and lookup (O(log n))


## Build Instructions

### Requirements:

Windows

Visual Studio 2022 (or compatible)

MSVC toolchain

### Steps:

Open IpcSystem.slnx in Visual Studio

Build the solution (x64, Debug or Release)

Run DaemonProcess.exe

Run one or more instances of ClientCLI.exe

The daemon must be running before any client connects.


