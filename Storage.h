#ifndef STORAGE_H
#define STORAGE_H

#include "Models.h"
#include <fstream>
#include <string>

using namespace std;

// EXCEPTION HANDLING: RAII guard used as C++'s practical "finally" equivalent.
class FileGuard {
private:
    ofstream file;
    string path;

public:
    explicit FileGuard(const string& path);
    ~FileGuard() noexcept;
    ofstream& stream();
};

class LocalStorage {
public:
    // throws: StorageException. noexcept(false) documents that this operation may throw.
    static void saveNote(const Note& note, const string& path) noexcept(false);
    // throws: StorageException. Used by the local-first storage demo.
    static string loadText(const string& path) noexcept(false);
    static void cleanupIncompleteOperation() noexcept;
};

#endif
