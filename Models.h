#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <chrono>
#include <iostream>
#include <utility>
#include <cstddef>
#include "Exceptions.h" // EXCEPTION HANDLING: Daily Notes can reject invalid dates.

using namespace std;

// ------------------------------------------------------------------
// NEW CODE: Typecasting support classes
//
// Priority  -> demonstrates BASIC -> CLASS (int -> Priority, via a
//              converting constructor) and CLASS -> BASIC
//              (Priority -> int, via a conversion operator).
// NoteSummary -> an unrelated class used as the target of the
//              CLASS -> CLASS conversion Note -> NoteSummary.
// ------------------------------------------------------------------
class Priority {
private:
    int level; // always kept in the range 1..5

public:
    // BASIC -> CLASS: not marked 'explicit', so an int converts to a
    // Priority implicitly:  Priority p = 4;
    Priority(int lvl = 1);

    // CLASS -> BASIC: lets a Priority be used wherever an int is
    // expected:  int x = p;
    operator int() const;

    string label() const;
};

class NoteSummary {
public:
    string title;
    int wordCount;
    size_t tagCount;

    NoteSummary(string title, int wordCount, size_t tagCount);
    void display() const;
};
// ------------------------------------------------------------------

class Note {
public:

    class Metadata {
    public:
        int editCount;
        long long createdAt;
        Metadata();
        void incrementEdits();
    };

protected:
    string id;
    string title;
    string content;
    vector<string> tags;
    long long lastModified;

public:
    Metadata meta; 
    static int totalNotesCreated;

    static int getTotalNotesCreated();

    Note(string id, string title, string content, vector<string> tags);
    virtual ~Note() = default;

    virtual void editContent(const string& newContent);
    virtual void display() const;

    string getId() const;
    string getTitle() const;
    string getContent() const;
    vector<string> getTags() const;

    // ------------------------------------------------------------
    // NEW CODE: Virtual Function
    // getTypeName() is declared virtual here and overridden in the
    // derived classes below (TaskNote, DailyTaskNote). This lets
    // code that only holds a Note* / Note& call getTypeName() and
    // have the CORRECT derived-class version run automatically
    // (runtime polymorphism / dynamic dispatch via the vtable).
    // ------------------------------------------------------------
    virtual string getTypeName() const;
    // ------------------------------------------------------------

    // ------------------------------------------------------------
    // NEW CODE: "this" Pointer
    // addTag() uses the implicit 'this' pointer explicitly to
    // modify the calling object's own members and then returns
    // *this (a reference to the dereferenced 'this' pointer) so
    // calls can be chained, e.g. note.addTag("a").addTag("b").
    //
    // isSameObjectAs() compares the address held in 'this' against
    // the address of another object to check object identity
    // (not just equal content).
    // ------------------------------------------------------------
    Note& addTag(const string& tag);
    bool isSameObjectAs(const Note& other) const;
    // ------------------------------------------------------------
    
    friend ostream& operator<<(ostream& os, const Note& note);
    friend Note operator+(const Note& lhs, const Note& rhs);
    
    friend istream& operator>>(istream& is, Note& note);
    
    friend class Indexer;

    // ------------------------------------------------------------
    // NEW CODE: Typecasting (conversions out of / into Note)
    // ------------------------------------------------------------
    // BASIC -> CLASS: builds a Note from a plain C-string
    //   Note quick = "Call the dentist";
    Note(const char* quickText);

    // CLASS -> BASIC: word count of the content. Marked 'explicit' so
    // it only happens on purpose: static_cast<int>(note)
    explicit operator int() const;

    // CLASS -> CLASS (conversion operator in the SOURCE class):
    //   NoteSummary s = note;
    operator NoteSummary() const;
    // ------------------------------------------------------------
};

class TaskNote : public Note {
protected:
    vector<pair<string, bool>> tasks;

public:
    TaskNote(string id, string title, string content, vector<string> tags);

    // ------------------------------------------------------------
    // NEW CODE: Typecasting - CLASS -> CLASS (constructor in the
    // DESTINATION class). Promotes a plain Note to a TaskNote:
    //   TaskNote t = someNote;
    // Lines starting with "- " in the content become tasks.
    // ------------------------------------------------------------
    TaskNote(const Note& source);
    // ------------------------------------------------------------

    void addTask(const string& task, bool isDone);
    void display() const override;

    // ------------------------------------------------------------
    // NEW CODE: Virtual Function (override)
    // ------------------------------------------------------------
    string getTypeName() const override;
    // ------------------------------------------------------------
};

class DailyTaskNote final : public TaskNote {
private:
    string date;

public:
    // EXCEPTION HANDLING: throws InvalidDateException for an invalid Daily Notes date.
    DailyTaskNote(string id, string title, string content, vector<string> tags, string date) noexcept(false);
    
    void display() const final override;

    // ------------------------------------------------------------
    // NEW CODE: Virtual Function (override, further specialized)
    // ------------------------------------------------------------
    string getTypeName() const override;
    // ------------------------------------------------------------
};

// ------------------------------------------------------------------
// NEW CODE: Virtual Base Class (solving the Diamond Problem)
//
// Entity is a common base class. Both Readable and Writable inherit
// from Entity using the 'virtual' keyword ("virtual public Entity").
// When ShareableNote then inherits from BOTH Readable and Writable,
// C++ guarantees only ONE shared sub-object of Entity exists inside
// ShareableNote, instead of two separate, ambiguous copies. This is
// the classic diamond-inheritance problem and 'virtual' inheritance
// is the language feature that fixes it.
//
//              Entity
//             /     |
//        Readable   Writable      (both: virtual public Entity)
//             |     /
//          ShareableNote
// ------------------------------------------------------------------
class Entity {
public:
    string entityId;
    Entity() : entityId("entity-0") {}
    virtual void identify() const {
        cout << "Entity ID: " << entityId << "\n";
    }
    virtual ~Entity() = default;
};

class Readable : virtual public Entity {
public:
    void read() const {
        cout << "[Readable] Reading entity: " << entityId << "\n";
    }
};

class Writable : virtual public Entity {
public:
    void write() const {
        cout << "[Writable] Writing entity: " << entityId << "\n";
    }
};

class ShareableNote : public Note, public Readable, public Writable {
public:
    ShareableNote(string id, string title, string content, vector<string> tags);
    void display() const override;
    string getTypeName() const override;
};
// ------------------------------------------------------------------

#endif
