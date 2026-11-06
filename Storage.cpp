#include "Storage.h"
#include "Exceptions.h"
#include <iostream>
#include <sstream>

using namespace std;

// EXCEPTION HANDLING: constructor acquires the file resource or throws.
FileGuard::FileGuard(const string& path) : file(path), path(path) {
    if (!file.is_open()) throw StorageException(path);
}

// EXCEPTION HANDLING: destructor runs during normal exit and stack unwinding.
FileGuard::~FileGuard() noexcept {
    if (file.is_open()) {
        file.close();
        cout << "[RAII cleanup] Closed '" << path << "'.\n";
    }
}

ofstream& FileGuard::stream() { return file; }

void LocalStorage::saveNote(const Note& note, const string& path) noexcept(false) {
    FileGuard guard(path); // EXCEPTION HANDLING: guard guarantees close without a finally block.
    guard.stream() << "# " << note.getTitle() << "\n\n" << note.getContent() << "\n";
    if (!guard.stream()) throw StorageException(path);
}

string LocalStorage::loadText(const string& path) noexcept(false) {
    ifstream file(path);
    if (!file.is_open()) throw StorageException(path);
    ostringstream text;
    text << file.rdbuf();
    return text.str();
}

void LocalStorage::cleanupIncompleteOperation() noexcept {
    cout << "[Manual cleanup] Released temporary Command Palette resources.\n";
}
