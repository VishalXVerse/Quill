#include "Models.h"
#include <sstream>
#include <regex> // EXCEPTION HANDLING: validates the ISO date shape for Daily Notes.

int Note::totalNotesCreated = 0;

int Note::getTotalNotesCreated() {
    return totalNotesCreated;
}

Note::Metadata::Metadata() {
    editCount = 0;
    createdAt = std::chrono::system_clock::now().time_since_epoch().count();
}

void Note::Metadata::incrementEdits() {
    editCount++;
}

Note::Note(std::string id, std::string title, std::string content, std::vector<std::string> tags)
    : id(std::move(id)), title(std::move(title)), content(std::move(content)), tags(std::move(tags)), meta() {
    lastModified = std::chrono::system_clock::now().time_since_epoch().count();
    totalNotesCreated++;
}

void Note::editContent(const std::string& newContent) {
    this->content = newContent;
    lastModified = std::chrono::system_clock::now().time_since_epoch().count();
    meta.incrementEdits();
}

void Note::display() const {
    std::cout << "\nNote: " << title << "\n";
    std::cout << "ID: " << id << "\nTags: ";
    for (const auto& tag : tags) std::cout << "#" << tag << " ";
    std::cout << "\nContent:\n" << content << "\n";
    std::cout << "[Edits: " << meta.editCount << "]\n";
}

std::string Note::getId() const { return id; }
std::string Note::getTitle() const { return title; }
std::string Note::getContent() const { return content; }
std::vector<std::string> Note::getTags() const { return tags; }

// ------------------------------------------------------------
// NEW CODE: Virtual Function (base implementation)
// ------------------------------------------------------------
std::string Note::getTypeName() const {
    return "Note";
}
// ------------------------------------------------------------

// ------------------------------------------------------------
// NEW CODE: "this" Pointer
// ------------------------------------------------------------
Note& Note::addTag(const std::string& tag) {
    // 'this' is a pointer to the current object. We use it explicitly
    // here (this->tags, this->meta) even though plain 'tags'/'meta'
    // would work identically, to make the pointer visible.
    this->tags.push_back(tag);
    this->meta.incrementEdits();

    // '*this' dereferences the 'this' pointer to get a reference to
    // the current object, enabling method chaining:
    //   note.addTag("a").addTag("b").addTag("c");
    return *this;
}

bool Note::isSameObjectAs(const Note& other) const {
    // Comparing the address stored in 'this' with the address of
    // 'other' tells us whether both refer to the SAME object in
    // memory, as opposed to two different objects with equal data.
    return this == &other;
}
// ------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const Note& note) {
    os << "\nNote: " << note.title << "\n";
    os << "ID: " << note.id << "\nTags: ";
    for (const auto& tag : note.tags) os << "#" << tag << " ";
    os << "\nContent:\n" << note.content;
    return os;
}

std::istream& operator>>(std::istream& is, Note& note) {
    std::cout << "Enter Note Title: ";
    std::getline(is, note.title);
    std::cout << "Enter Note Content: ";
    std::getline(is, note.content);
    note.id = "stream-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    note.lastModified = std::chrono::system_clock::now().time_since_epoch().count();
    return is;
}

Note operator+(const Note& lhs, const Note& rhs) {
    std::vector<std::string> combinedTags = lhs.tags;
    for (const auto& tag : rhs.tags) {
        combinedTags.push_back(tag);
    }
    
    return Note(
        lhs.id + "_" + rhs.id,
        lhs.title + " + " + rhs.title,
        lhs.content + "\n\nMerged\n\n" + rhs.content,
        combinedTags
    );
}

TaskNote::TaskNote(std::string id, std::string title, std::string content, std::vector<std::string> tags)
    : Note(std::move(id), std::move(title), std::move(content), std::move(tags)) {}

void TaskNote::addTask(const std::string& task, bool isDone) {
    tasks.push_back({task, isDone});
}

void TaskNote::display() const {
    std::cout << "\nTask Note: " << title << "\n";
    std::cout << "ID: " << id << "\nTags: ";
    for (const auto& tag : tags) std::cout << "#" << tag << " ";
    std::cout << "\nContent:\n" << content << "\nTasks:\n";
    for (const auto& t : tasks) {
        std::cout << (t.second ? "[x] " : "[ ] ") << t.first << "\n";
    }
}

// ------------------------------------------------------------
// NEW CODE: Virtual Function (override)
// ------------------------------------------------------------
std::string TaskNote::getTypeName() const {
    return "TaskNote";
}
// ------------------------------------------------------------

DailyTaskNote::DailyTaskNote(std::string id, std::string title, std::string content, std::vector<std::string> tags, std::string date) noexcept(false)
    : TaskNote(std::move(id), std::move(title), std::move(content), std::move(tags)), date(std::move(date)) {
    static const std::regex isoDate(R"(^[0-9]{4}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$)");
    if (!std::regex_match(this->date, isoDate)) throw InvalidDateException(this->date); // EXCEPTION HANDLING: fail early for Daily Notes.
}

void DailyTaskNote::display() const {
    std::cout << "\nDaily Task [" << date << "]: " << title << "\n";
    std::cout << "ID: " << id << "\nTags: ";
    for (const auto& tag : tags) std::cout << "#" << tag << " ";
    std::cout << "\nContent:\n" << content << "\nTasks:\n";
    for (const auto& t : tasks) {
        std::cout << (t.second ? "[x] " : "[ ] ") << t.first << "\n";
    }
}

// ------------------------------------------------------------
// NEW CODE: Virtual Function (override)
// ------------------------------------------------------------
std::string DailyTaskNote::getTypeName() const {
    return "DailyTaskNote";
}
// ------------------------------------------------------------

// ------------------------------------------------------------------
// NEW CODE: Virtual Base Class - ShareableNote implementation
// ShareableNote inherits Note directly, and inherits Entity through
// TWO paths (Readable and Writable), each of which used 'virtual'
// inheritance from Entity. Because of that 'virtual' keyword there
// is still only one Entity sub-object, so entityId is unambiguous.
// ------------------------------------------------------------------
ShareableNote::ShareableNote(std::string id, std::string title, std::string content, std::vector<std::string> tags)
    : Note(std::move(id), std::move(title), std::move(content), std::move(tags)) {
    // Entity's default constructor already ran once (not twice) thanks
    // to virtual inheritance, so we can safely customize its data here.
    this->entityId = "shareable-" + this->id;
}

void ShareableNote::display() const {
    std::cout << "\nShareable Note: " << title << "\n";
    std::cout << "ID: " << id << " | Entity ID: " << entityId << "\n";
    std::cout << "Tags: ";
    for (const auto& tag : tags) std::cout << "#" << tag << " ";
    std::cout << "\nContent:\n" << content << "\n";
}

std::string ShareableNote::getTypeName() const {
    return "ShareableNote";
}
// ------------------------------------------------------------------

// ------------------------------------------------------------------
// NEW CODE: Typecasting
// ------------------------------------------------------------------
namespace {
    int countWords(const std::string& text) {
        std::istringstream iss(text);
        std::string word;
        int n = 0;
        while (iss >> word) ++n;
        return n;
    }
}

// ---- Priority: BASIC -> CLASS and CLASS -> BASIC ----
Priority::Priority(int lvl) : level(lvl < 1 ? 1 : (lvl > 5 ? 5 : lvl)) {}

Priority::operator int() const { return level; }

std::string Priority::label() const {
    switch (level) {
        case 1:  return "Lowest";
        case 2:  return "Low";
        case 3:  return "Medium";
        case 4:  return "High";
        default: return "Critical";
    }
}

// ---- NoteSummary: target of the CLASS -> CLASS conversion ----
NoteSummary::NoteSummary(std::string title, int wordCount, std::size_t tagCount)
    : title(std::move(title)), wordCount(wordCount), tagCount(tagCount) {}

void NoteSummary::display() const {
    std::cout << "Summary: '" << title << "' | " << wordCount
              << " words | " << tagCount << " tag(s)\n";
}

// ---- Note: BASIC -> CLASS (const char* -> Note) ----
// Delegates to the main constructor so totalNotesCreated still counts it.
// The title is the first 20 characters of the text.
Note::Note(const char* quickText)
    : Note("quick-" + std::to_string(totalNotesCreated + 1),
           std::string(quickText).substr(0, 20),
           std::string(quickText),
           std::vector<std::string>{}) {}

// ---- Note: CLASS -> BASIC (Note -> int, explicit) ----
Note::operator int() const {
    return countWords(content);
}

// ---- Note: CLASS -> CLASS (Note -> NoteSummary, source-side operator) ----
Note::operator NoteSummary() const {
    return NoteSummary(title, countWords(content), tags.size());
}

// ---- TaskNote: CLASS -> CLASS (Note -> TaskNote, destination-side ctor) ----
TaskNote::TaskNote(const Note& source)
    : Note(source.getId(), source.getTitle(), source.getContent(), source.getTags()) {
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.rfind("- [x] ", 0) == 0)      addTask(line.substr(6), true);
        else if (line.rfind("- [ ] ", 0) == 0) addTask(line.substr(6), false);
        else if (line.rfind("- ", 0) == 0)     addTask(line.substr(2), false);
    }
}
// ------------------------------------------------------------------
