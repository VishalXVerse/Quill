#ifndef CORE_H
#define CORE_H

#include "Models.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

class Vault {
private:
    std::vector<std::shared_ptr<Note>> notes;

public:
    void addNote(std::shared_ptr<Note> note);
    void deleteNote(const std::string& id);
    std::vector<std::shared_ptr<Note>> listAllNotes() const;
    std::shared_ptr<Note> getNoteById(const std::string& id) const;
    // EXCEPTION HANDLING: throws NoteNotFoundException; C++ documents, but does not require, callers to declare it.
    std::shared_ptr<Note> requireNoteById(const std::string& id) const noexcept(false);

    // ------------------------------------------------------------
    // NEW CODE: Pointer to Objects
    // Returns a raw Note* (a plain pointer to the underlying object)
    // instead of a std::shared_ptr. This is used to demonstrate raw
    // object pointers explicitly. The Vault still owns the object
    // via the shared_ptr in 'notes'; the returned pointer is a
    // non-owning "view" and must not be deleted by the caller.
    // ------------------------------------------------------------
    Note* getRawNotePointer(const std::string& id) const;
    // ------------------------------------------------------------
};

class Indexer {
private:
    const Vault& vault;
    std::unordered_map<std::string, std::vector<std::string>> backlinks;
    std::unordered_map<std::string, std::vector<std::string>> forwardLinks;

public:
    Indexer(const Vault& vault);
    void rebuildIndex();
    std::vector<std::string> searchNotes(const std::string& query) const;
    std::vector<std::string> getBacklinks(const std::string& title) const;
    std::string getReachableNotesBFS(const std::string& startTitle) const;
    // EXCEPTION HANDLING: validates [[links]] for Bidirectional Linking and Backlinks.
    void validateLinks() const noexcept(false);
};

class NoteComparator {
public:
    static std::string compare(const Note& a, const Note& b);
};

#endif
