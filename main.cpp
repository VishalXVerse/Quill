#include "Models.h"
#include "Core.h"
#include "Templates.h"
#include "Exceptions.h" // EXCEPTION HANDLING: user-defined exception hierarchy.
#include "Storage.h" // EXCEPTION HANDLING: local-first storage and RAII FileGuard.
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

void demonstrateObjectSlicing(Note baseNote) {
    std::cout << "\n[Object Slicing Occurred!]\n";
    std::cout << "A TaskNote was passed by value to a function expecting a base Note.\n";
    std::cout << "The task list data has been discarded (sliced), and it acts only as a base note:\n";
    baseNote.display();
}

// ------------------------------------------------------------
// NEW CODE: Typecasting helpers
// These show implicit conversions happening at a CALL SITE.
// ------------------------------------------------------------
void showPriority(const Priority& p) {          // expects a Priority...
    std::cout << "  showPriority(): level " << static_cast<int>(p) << " (" << p.label() << ")\n";
}

int addBonus(int base) {                         // ...expects an int
    return base + 10;
}
// ------------------------------------------------------------

// EXCEPTION HANDLING: its destructor proves stack unwinding in the demo output.
class UnwindTracer {
private:
    std::string scope;
public:
    explicit UnwindTracer(std::string scope) : scope(std::move(scope)) {
        std::cout << "[Enter] " << this->scope << "\n";
    }
    ~UnwindTracer() noexcept {
        std::cout << "[Stack unwinding] Destroyed " << scope << ".\n";
    }
};

// EXCEPTION HANDLING: throws NoteNotFoundException from a function; main catches it.
void requestMissingNote(const Vault& vault) noexcept(false) {
    UnwindTracer trace("requestMissingNote()");
    vault.requireNoteById("missing-note");
}

// EXCEPTION HANDLING: planned Command Palette feature validates an entered command.
void executeCommand(const std::string& command) noexcept(false) {
    if (command != "open" && command != "search" && command != "new") throw InvalidCommandException(command);
}

// EXCEPTION HANDLING: catch(...) cleans up, then throw; rethrows the original std::string unchanged.
void commandPaletteCatchAllDemo() noexcept(false) {
    try {
        throw std::string("Command Palette plugin response was malformed.");
    } catch (...) {
        LocalStorage::cleanupIncompleteOperation();
        throw;
    }
}

// EXCEPTION HANDLING: creates a temporary vault so a real [[link]] validator can find a broken target.
void demonstrateBrokenLink() noexcept(false) {
    Vault brokenVault;
    brokenVault.addNote(std::make_shared<Note>("broken-1", "Broken Link Demo", "See [[Missing Target]].", std::vector<std::string>{"demo"}));
    Indexer brokenIndexer(brokenVault);
    brokenIndexer.rebuildIndex();
    brokenIndexer.validateLinks();
}

void runExceptionHandlingDemo(const Vault& vault, const std::shared_ptr<Note>& sampleNote) {
    std::cout << "\n===== Exception Handling Demo =====\n";
    std::cout << "C++ has no Java-style finally or throws keyword. This demo uses RAII, catch(...), rethrow, and noexcept(false).\n";

    // EXCEPTION HANDLING: correct order is derived class first, then its base class.
    try {
        requestMissingNote(vault);
    } catch (const NoteNotFoundException& error) {
        std::cout << "[Derived catch] " << error.what() << " Code: " << static_cast<int>(error.code()) << "\n";
    } catch (const QuillException& error) {
        std::cout << "[Base catch] " << error.what() << "\n";
    }
    // WRONG ORDER - kept as a comment because the base handler would hide the derived handler:
    // catch (const QuillException& error) { /* catches NoteNotFoundException too */ }
    // catch (const NoteNotFoundException& error) { /* unreachable */ }

    try {
        demonstrateBrokenLink();
    } catch (const InvalidLinkException& error) {
        std::cout << "[Derived catch] " << error.what() << "\n";
    } catch (const QuillException& error) {
        std::cout << "[Base catch] " << error.what() << "\n";
    }

    try {
        DailyTaskNote invalidDay("bad-date", "Bad Daily Note", "Demo", {}, "2026-15-40");
        invalidDay.display();
    } catch (const InvalidDateException& error) {
        std::cout << "[Daily Notes] " << error.what() << "\n";
    }

    try {
        LocalStorage::saveNote(*sampleNote, "quill_demo_backup.md");
        std::cout << "[Local-first storage] Saved a Markdown backup.\n";
    } catch (const StorageException& error) {
        std::cout << "[Storage catch] " << error.what() << "\n";
    }

    try {
        LocalStorage::loadText("missing_quill_note.md");
    } catch (const StorageException& error) {
        std::cout << "[Storage catch] " << error.what() << "\n";
    }

    try {
        executeCommand("publish-everything");
    } catch (const QuillException& error) { // Catching by const reference preserves polymorphism and avoids slicing.
        std::cout << "[Command Palette] " << error.what() << "\n";
    }

    try {
        commandPaletteCatchAllDemo();
    } catch (const std::string& message) {
        std::cout << "[Rethrown std::string] " << message << "\n";
    }

    try {
        throw 404; // EXCEPTION HANDLING: built-in int exception for the syllabus demonstration.
    } catch (int status) {
        std::cout << "[Built-in int] Demo status: " << status << "\n";
    }

    try {
        throw std::runtime_error("Standard library runtime error demonstration.");
    } catch (const std::exception& error) {
        std::cout << "[std::exception base catch] " << error.what() << "\n";
    }

    std::cout << "[Post-try cleanup] The demo continues after all handled exceptions.\n";
}

void printMenu() {
    
    std::cout << "\nQUILL C++ CLI\n";

    std::cout << "1. List All Notes\n";
    std::cout << "2. Create Standard Note\n";
    std::cout << "3. Create Task Note\n";
    std::cout << "4. Rebuild Index & Show Graph\n";
    std::cout << "5. Test Operator Overloading (+ and <<)\n";
    std::cout << "6. Import Note via Stream Input (>> operator)\n";
    std::cout << "7. Show System Stats (Static Variables & Nested Class)\n";
    std::cout << "8. Demonstrate Object Slicing\n";
    std::cout << "9. Demonstrate Typecasting (Basic->Class, Class->Basic, Class->Class)\n";
    std::cout << "10. Demonstrate Templates (Function & Class Templates)\n";
    std::cout << "11. Demonstrate Exception Handling\n";
    std::cout << "0. Exit\n";
    std::cout << "Select option: ";
}

int main() {
    Vault vault;
    Indexer indexer(vault);

    auto n1 = std::make_shared<Note>("1", "Project Alpha", "The goal of Project Alpha is to integrate with [[Meeting Notes]].", std::vector<std::string>{"work"});
    auto n2 = std::make_shared<Note>("2", "Meeting Notes", "Discussed Q4 Strategy. See [[Project Alpha]].", std::vector<std::string>{"work", "meeting"});
    
    auto tn1 = std::make_shared<TaskNote>("3", "Grocery List", "Weekly restock.", std::vector<std::string>{"home"});
    tn1->addTask("Milk", false);
    tn1->addTask("Eggs", true);

    n1->editContent("Updated goal: integrate fully with [[Meeting Notes]].");

    // ------------------------------------------------------------
    // NEW CODE: extra sample notes used by the demonstrations folded
    // into the existing menu options below (options 1, 5, 7, 8).
    // ------------------------------------------------------------
    auto dtn1 = std::make_shared<DailyTaskNote>("4", "Daily Standup", "Notes from today's standup.", std::vector<std::string>{"daily"}, "2026-09-23");
    dtn1->addTask("Sync with team", false);

    auto shn1 = std::make_shared<ShareableNote>("5", "Public Roadmap", "Roadmap shared with the whole team.", std::vector<std::string>{"shared"});
    // ------------------------------------------------------------
    
    vault.addNote(n1);
    vault.addNote(n2);
    vault.addNote(tn1);
    // ------------------------------------------------------------
    // NEW CODE
    // ------------------------------------------------------------
    vault.addNote(dtn1);
    vault.addNote(shn1);
    // ------------------------------------------------------------

    indexer.rebuildIndex();

    int choice;
    while (true) {
        printMenu();
        if (!(std::cin >> choice)) break;

        if (choice == 0) {
            std::cout << "Exiting Quill C++...\n";
            break;
        }

        if (choice == 1) {
            std::cout << "\nAll Notes in Vault\n";
            for (const auto& n : vault.listAllNotes()) {
                // ------------------------------------------------------------
                // NEW CODE: Virtual Function
                // 'n' is held as a shared_ptr<Note> (conceptually a pointer
                // to the BASE class) but may actually point to a Note,
                // TaskNote, DailyTaskNote, or ShareableNote object. Because
                // getTypeName() is declared 'virtual' in Note and overridden
                // in each derived class, the call below resolves at RUNTIME
                // to the correct override for whatever object 'n' really
                // points to (dynamic dispatch through the vtable).
                // ------------------------------------------------------------
                std::cout << "[" << n->getTypeName() << "]";
                // ------------------------------------------------------------
                n->display();
            }
        } else if (choice == 2) {
            std::string title, content;
            std::cout << "Title: ";
            std::cin.ignore();
            std::getline(std::cin, title);
            std::cout << "Content: ";
            std::getline(std::cin, content);
            
            vault.addNote(std::make_shared<Note>("uuid-" + title, title, content, std::vector<std::string>{}));
            std::cout << "Standard Note created!\n";
        } else if (choice == 3) {
            std::string title, content, task;
            std::cout << "Title: ";
            std::cin.ignore();
            std::getline(std::cin, title);
            std::cout << "Content: ";
            std::getline(std::cin, content);
            
            auto newNote = std::make_shared<TaskNote>("uuid-" + title, title, content, std::vector<std::string>{});
            std::cout << "Enter a task to add: ";
            std::getline(std::cin, task);
            newNote->addTask(task, false);
            
            vault.addNote(newNote);
            std::cout << "Task Note created!\n";
        } else if (choice == 4) {
            indexer.rebuildIndex();
            std::cout << "\nBacklinks for 'Project Alpha'\n";
            auto bls = indexer.getBacklinks("Project Alpha");
            if (bls.empty()) std::cout << "None\n";
            for (const auto& bl : bls) {
                std::cout << "- " << bl << "\n";
            }
            
            std::cout << "\nBFS Reachability from 'Project Alpha'\n";
            std::cout << indexer.getReachableNotesBFS("Project Alpha") << "\n";
        } else if (choice == 5) {
            std::cout << "\nTesting Operator Overloading\n";

            // ------------------------------------------------------------
            // NEW CODE: "this" Pointer
            // addTag() uses the implicit 'this' pointer explicitly
            // (this->tags.push_back(...)) to modify the object it was
            // called on, then returns *this (a reference obtained by
            // dereferencing 'this') so the calls below chain onto the
            // SAME object: n2->addTag(...).addTag(...).
            // ------------------------------------------------------------
            std::cout << "Tagging 'Meeting Notes' via chained calls (uses the 'this' pointer):\n";
            n2->addTag("reviewed").addTag("finalized");
            std::cout << "n2 tags now: ";
            for (const auto& t : n2->getTags()) std::cout << "#" << t << " ";
            std::cout << "\n";
            // ------------------------------------------------------------

            std::cout << "\nMerging 'Project Alpha' and 'Meeting Notes' using the '+' operator\n";
            Note mergedNote = *n1 + *n2;
            std::cout << "\nPrinting merged Note using the '<<' operator:\n";
            std::cout << mergedNote << "\n";

            // ------------------------------------------------------------
            // NEW CODE: "this" Pointer
            // isSameObjectAs() compares the address stored in 'this'
            // against the address of 'other' to check object identity.
            // mergedNote is a freshly constructed object, so it is NOT
            // the same object as n1, even though it was built from n1.
            // ------------------------------------------------------------
            std::cout << "Is mergedNote the same object as n1? "
                      << (mergedNote.isSameObjectAs(*n1) ? "true" : "false") << "\n";
            // ------------------------------------------------------------
        } else if (choice == 6) {
            std::cout << "\nInput Stream Overloading (>>)\n";
            std::cin.ignore();
            Note importedNote("temp", "temp", "temp", {}); 
            std::cin >> importedNote; 
            vault.addNote(std::make_shared<Note>(importedNote));
            std::cout << "Note successfully imported into Vault via stream!\n";
        } else if (choice == 7) {
            std::cout << "\nStatic & Nested Class Features\n";
            std::cout << "Total Notes ever instantiated in memory: " << Note::getTotalNotesCreated() << "\n";
            
            std::cout << "\nMetadata for Project Alpha:\n";
            std::cout << "Edit Count: " << n1->meta.editCount << "\n";
            std::cout << "Creation Timestamp: " << n1->meta.createdAt << "\n";

            // ------------------------------------------------------------
            // NEW CODE: Virtual Base Class (diamond problem)
            // ShareableNote inherits Entity through TWO paths: Readable
            // and Writable. Both of those inherit from Entity using
            // 'virtual public Entity', so C++ collapses the two
            // would-be-separate Entity sub-objects into a single shared
            // one. identify() (from Entity), read() (from Readable) and
            // write() (from Writable) all operate on that one Entity.
            // ------------------------------------------------------------
            std::cout << "\nVirtual Base Class Demo (ShareableNote's diamond inheritance):\n";
            shn1->identify();
            shn1->read();
            shn1->write();
            // ------------------------------------------------------------

            // ------------------------------------------------------------
            // NEW CODE: Pointer to Objects
            // getRawNotePointer() hands back a plain Note* (a raw pointer
            // to the object living inside the Vault's shared_ptr), rather
            // than a shared_ptr. It is a non-owning view of the object.
            // ------------------------------------------------------------
            std::cout << "\nRaw pointer fetched directly from the Vault:\n";
            Note* rawPtr = vault.getRawNotePointer("1");
            if (rawPtr) {
                std::cout << "Note* -> \"" << rawPtr->getTitle() << "\" (address: " << rawPtr << ")\n";
            }
            // ------------------------------------------------------------
        } else if (choice == 8) {
            std::cout << "\nDemonstrating Object Slicing\n";
            std::cout << "First, this is what our TaskNote normally looks like:\n";
            tn1->display();
            
            std::cout << "\nNow passing it to a function that accepts 'Note' by value...\n";
            demonstrateObjectSlicing(*tn1);

            // ------------------------------------------------------------
            // NEW CODE: Pointer to Objects & Pointer to Derived Classes
            // Passing an object by VALUE (above) copies only the Note
            // portion and slices off the TaskNote-specific data. Using a
            // POINTER instead avoids that entirely: no copy is made, the
            // pointer just stores the address of the original object, so
            // its full derived type and virtual behavior stay intact.
            // ------------------------------------------------------------
            std::cout << "\nContrast: accessing the SAME object through a base-class pointer\n";
            std::cout << "(no copy is made, so nothing is sliced off):\n";
            Note* basePtr = tn1.get();   // pointer to Note, pointing at a derived TaskNote object
            basePtr->display();          // virtual dispatch -> TaskNote::display(), nothing lost

            std::cout << "\nDowncasting that base pointer to a pointer to the derived class:\n";
            TaskNote* derivedPtr = dynamic_cast<TaskNote*>(basePtr);
            if (derivedPtr) {
                derivedPtr->addTask("Bread", false); // only reachable through the derived pointer
                std::cout << "Added a task through the derived-class pointer:\n";
                derivedPtr->display();
            }
            // ------------------------------------------------------------
        } else if (choice == 9) {
            // ------------------------------------------------------------
            // NEW CODE: Typecasting
            // ------------------------------------------------------------
            std::cout << "\n===== Typecasting Demo =====\n";

            // ---- 1. BASIC -> CLASS ----
            std::cout << "\n[1] Basic -> Class (converting constructors)\n";
            Note quick = "Call the dentist tomorrow at 5pm";   // const char* -> Note
            std::cout << "  const char* became a Note titled: '" << quick.getTitle() << "'\n";

            Priority p = 4;                                    // int -> Priority
            std::cout << "  int 4 became a Priority: " << p.label() << "\n";
            showPriority(9);                                   // implicit int -> Priority (clamped to 5)

            // ---- 2. CLASS -> BASIC ----
            std::cout << "\n[2] Class -> Basic (conversion operators)\n";
            int level = p;                                     // Priority -> int (implicit)
            std::cout << "  int level = p;            -> " << level << "\n";
            std::cout << "  addBonus(p)               -> " << addBonus(p) << "  (Priority passed where an int is expected)\n";
            int words = static_cast<int>(quick);               // Note -> int (explicit)
            std::cout << "  static_cast<int>(quick)   -> " << words << " words\n";

            // ---- 3. CLASS -> CLASS ----
            std::cout << "\n[3] Class -> Class\n";

            // (a) constructor in the DESTINATION class: Note -> TaskNote
            Note trip("t1", "Weekend Trip",
                      "Plan the trip:\n- Book flights\n- [x] Renew passport\n- Pack bags",
                      std::vector<std::string>{"travel"});
            std::cout << "  (a) Note -> TaskNote via TaskNote(const Note&):\n";
            TaskNote promoted = trip;
            promoted.display();

            // (b) conversion operator in the SOURCE class: Note -> NoteSummary
            std::cout << "\n  (b) Note -> NoteSummary via Note::operator NoteSummary():\n  ";
            NoteSummary summary = *n1;
            summary.display();
            // ------------------------------------------------------------
        } else if (choice == 10) {
            // ------------------------------------------------------------
            // NEW CODE: Templates
            // ------------------------------------------------------------
            std::cout << "\n===== Templates Demo =====\n";

            // ---- FUNCTION TEMPLATES ----
            std::cout << "\n[Function templates]\n";
            std::cout << "  maxOf(3, 9)                    -> " << maxOf(3, 9) << "\n";
            std::cout << "  maxOf(2.5, 1.5)                -> " << maxOf(2.5, 1.5) << "\n";
            std::cout << "  maxOf<std::string>(apple, pear)-> "
                      << maxOf<std::string>("apple", "pear") << "\n";
            std::cout << "  maxOf(Priority(2), Priority(5))-> "
                      << maxOf(Priority(2), Priority(5)).label() << "  (works via Priority -> int)\n";
            std::cout << "  maxOf<Note>(n1, n2) [specialized: longer content wins] -> '"
                      << maxOf<Note>(*n1, *n2).getTitle() << "'\n";

            std::cout << "  clampValue(15, 1, 5)           -> " << clampValue(15, 1, 5) << "\n";
            std::cout << "  clampValue(0.2, 0.5, 1.0)      -> " << clampValue(0.2, 0.5, 1.0) << "\n";

            int a = 1, b = 2;
            swapValues(a, b);
            std::string s1 = "left", s2 = "right";
            swapValues(s1, s2);
            std::cout << "  swapValues(1,2)                -> " << a << ", " << b << "\n";
            std::cout << "  swapValues(left,right)         -> " << s1 << ", " << s2 << "\n";

            auto allNotes = vault.listAllNotes();
            auto workNotes = filterItems(allNotes, [](const std::shared_ptr<Note>& n) {
                for (const auto& t : n->getTags()) if (t == "work") return true;
                return false;
            });
            std::cout << "  filterItems(notes tagged #work)-> " << workNotes.size() << " note(s): ";
            for (const auto& n : workNotes) std::cout << "'" << n->getTitle() << "' ";
            std::cout << "\n";
            std::cout << "  joinToString(n2 tags)          -> " << joinToString(n2->getTags()) << "\n";
            std::cout << "  joinToString({1,2,3}, \" | \")   -> " << joinToString(std::vector<int>{1, 2, 3}, " | ") << "\n";

            // ---- CLASS TEMPLATES ----
            std::cout << "\n[Class templates]\n";

            Registry<std::string, int> tagCounts;                       // <string, int>
            for (const auto& n : allNotes)
                for (const auto& t : n->getTags()) tagCounts[t]++;
            std::cout << "  Registry<string,int> tag usage (" << tagCounts.size() << " tags):";
            tagCounts.forEach([](const std::string& tag, int count) {
                std::cout << " #" << tag << "=" << count;
            });
            std::cout << "\n";

            Registry<std::string, std::shared_ptr<Note>> byTitle;      // <string, shared_ptr<Note>>
            for (const auto& n : allNotes) byTitle.add(n->getTitle(), n);
            const auto* found = byTitle.find("Public Roadmap");
            std::cout << "  Registry<string,shared_ptr<Note>> lookup 'Public Roadmap' -> "
                      << (found ? (*found)->getTypeName() : std::string("not found")) << "\n";
            std::cout << "  contains('Nope')? " << (byTitle.contains("Nope") ? "yes" : "no") << "\n";

            RecentList<std::string, 3> recent;                         // non-type parameter N = 3
            for (const auto& n : allNotes) recent.push(n->getTitle());
            recent.push("Project Alpha");                              // re-use moves it to the front
            std::cout << "  RecentList<string," << RecentList<std::string, 3>::capacity()
                      << "> (newest first): " << joinToString(recent.toVector()) << "\n";
            // ------------------------------------------------------------
        } else if (choice == 11) {
            runExceptionHandlingDemo(vault, n1); // EXCEPTION HANDLING: run each syllabus example from the menu.
        }
    }

    return 0;
}
