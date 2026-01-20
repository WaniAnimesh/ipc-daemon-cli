#define NOMINMAX   // Prevent <windows.h> from defining min/max macros
#include <windows.h>
#include <iostream>
#include <string>
#include <limits>

bool SendCommand(HANDLE hPipe, int choice, int value = 0) {
    DWORD written;
    if (!WriteFile(hPipe, &choice, sizeof(int), &written, NULL)) {
        std::cerr << "Error writing choice. Code=" << GetLastError() << std::endl;
        return false;
    }
    if (choice == 1 || choice == 2) {
        if (!WriteFile(hPipe, &value, sizeof(int), &written, NULL)) {
            std::cerr << "Error writing value. Code=" << GetLastError() << std::endl;
            return false;
        }
    }

    // Receive response length
    int len;
    DWORD read;
    if (!ReadFile(hPipe, &len, sizeof(int), &read, NULL)) {
        std::cerr << "Error reading response length. Code=" << GetLastError() << std::endl;
        return false;
    }

    // Read response in chunks
    std::string response(len, '\0');
    DWORD totalRead = 0;
    while (totalRead < (DWORD)len) {
        DWORD chunk;
        if (!ReadFile(hPipe, &response[totalRead], len - totalRead, &chunk, NULL)) {
            std::cerr << "Error reading response. Code=" << GetLastError() << std::endl;
            return false;
        }
        totalRead += chunk;
    }

    std::cout << "\n--- Server Response ---\n" << response << "\n-----------------------\n";
    return true;
}

int main() {
    HANDLE hPipe = CreateFile(L"\\\\.\\pipe\\MyIpcPipe", GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Could not connect to Daemon. Code=" << GetLastError() << std::endl;
        return 1;
    }

    int choice, val;
    while (true) {
        std::cout << "\n1. Insert\n2. Delete\n3. Print All\n4. Clear All\n5. Exit\nChoice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // works now
            std::cout << "Invalid input. Please enter a number between 1 and 5.\n";
            continue;
        }

        if (choice == 5) break;
        if (choice < 1 || choice > 5) {
            std::cout << "Invalid choice. Please enter a number between 1 and 5.\n";
            continue;
        }

        if (choice == 1 || choice == 2) {
            std::cout << "Enter positive integer: ";
            if (!(std::cin >> val)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a positive integer.\n";
                continue;
            }
            if (val <= 0) {
                std::cout << "Invalid input. Must be positive.\n";
                continue;
            }
            SendCommand(hPipe, choice, val);
        }
        else {
            SendCommand(hPipe, choice);
        }
    }

    CloseHandle(hPipe);
    return 0;
}
