#include <windows.h>
#include <iostream>
#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>

struct DataRecord {
    std::map<int, long long> storage;
};

DataRecord g_data;
std::mutex g_mutex;
HANDLE g_singleInstanceMutex;

bool ensureSingleInstance() {
    g_singleInstanceMutex = CreateMutex(NULL, TRUE, L"MyDaemonSingleInstanceMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cerr << "Another instance of the daemon is already running." << std::endl;
        return false;
    }
    return true;
}

long long getCurrentTimestamp() {
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

std::string processCommand(int choice, int value) {
    long long now = getCurrentTimestamp();
    std::string response;

    switch (choice) {
    case 1: // Insert
        if (value > 0 && g_data.storage.find(value) == g_data.storage.end()) {
            g_data.storage[value] = now;
            response = "Inserted " + std::to_string(value) + " at " + std::to_string(now);
        }
        else {
            response = "Error: Duplicate or invalid number.";
        }
        break;

    case 2: // Delete
        if (g_data.storage.erase(value)) {
            response = "Deleted " + std::to_string(value);
        }
        else {
            response = "Error: Number not found.";
        }
        break;

    case 3: // Print All
        for (auto const& [num, ts] : g_data.storage) {
            response += std::to_string(num) + " (TS: " + std::to_string(ts) + ")\n";
        }
        if (response.empty()) response = "List is empty.";
        break;

    case 4: // Clear All
        g_data.storage.clear();
        response = "All records cleared.";
        break;

    default:
        response = "Invalid choice.";
    }

    return response;
}

void HandleClient(HANDLE hPipe) {
    while (true) {
        int choice = 0;
        int value = 0;
        DWORD read = 0;

        if (!ReadFile(hPipe, &choice, sizeof(int), &read, NULL) || read != sizeof(int)) {
            break; // client closed or error
        }

        if (choice == 1 || choice == 2) {
            if (!ReadFile(hPipe, &value, sizeof(int), &read, NULL) || read != sizeof(int)) {
                break;
            }
        }

        std::string response;
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            response = processCommand(choice, value);
        }

        DWORD written;
        int len = (int)response.size();
        WriteFile(hPipe, &len, sizeof(int), &written, NULL);
        WriteFile(hPipe, response.c_str(), len, &written, NULL);

        std::cout << "[Daemon Log] " << response << std::endl;
    }

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}



int main() {
    if (!ensureSingleInstance()) return 1;

    std::cout << "Daemon running. In-memory storage active..." << std::endl;

    while (true) {
        HANDLE hPipe = CreateNamedPipe(
            L"\\\\.\\pipe\\MyIpcPipe",
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            1024, 1024, 0, NULL);

        if (ConnectNamedPipe(hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
            std::thread clientThread(HandleClient, hPipe);
            clientThread.detach();
        }
    }

    return 0;
}
