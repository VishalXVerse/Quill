#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>
#include <utility>

// EXCEPTION HANDLING: Quill's user-defined exception hierarchy.
enum class QuillErrorCode { NoteNotFound = 101, InvalidLink = 102, Storage = 103, InvalidCommand = 104, InvalidDate = 105 };

class QuillException : public std::exception {
private:
    std::string message;
    QuillErrorCode errorCode;

public:
    QuillException(std::string message, QuillErrorCode errorCode) : message(std::move(message)), errorCode(errorCode) {}
    const char* what() const noexcept override { return message.c_str(); }
    QuillErrorCode code() const noexcept { return errorCode; }
};

class NoteNotFoundException final : public QuillException {
public:
    explicit NoteNotFoundException(const std::string& id)
        : QuillException("Note with ID '" + id + "' was not found.", QuillErrorCode::NoteNotFound) {}
};

class InvalidLinkException final : public QuillException {
public:
    explicit InvalidLinkException(const std::string& target)
        : QuillException("Broken [[link]] target: '" + target + "'.", QuillErrorCode::InvalidLink) {}
};

class StorageException final : public QuillException {
public:
    explicit StorageException(const std::string& path)
        : QuillException("Could not open local storage file: '" + path + "'.", QuillErrorCode::Storage) {}
};

class InvalidCommandException final : public QuillException {
public:
    explicit InvalidCommandException(const std::string& command)
        : QuillException("Unknown Command Palette command: '" + command + "'.", QuillErrorCode::InvalidCommand) {}
};

class InvalidDateException final : public QuillException {
public:
    explicit InvalidDateException(const std::string& date)
        : QuillException("Invalid Daily Notes date: '" + date + "'. Use YYYY-MM-DD.", QuillErrorCode::InvalidDate) {}
};

#endif
