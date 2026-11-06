#include "Core.h"
#include "Exceptions.h" // EXCEPTION HANDLING: custom Quill exception types.
#include <regex>
#include <queue>
#include <unordered_set>
#include <sstream>
#include <algorithm>
#include <functional>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <exception>

using namespace std;

void Vault::addNote(shared_ptr<Note> note) {
    notes.push_back(note);
}

void Vault::deleteNote(const string& id) {
    for (auto it = notes.begin(); it != notes.end(); ++it) {
        if ((*it)->getId() == id) {
            notes.erase(it);
            break;
        }
    }
}

vector<shared_ptr<Note>> Vault::listAllNotes() const {
    return notes;
}

shared_ptr<Note> Vault::getNoteById(const string& id) const {
    for (const auto& note : notes) {
        if (note->getId() == id) return note;
    }
    return nullptr;
}

shared_ptr<Note> Vault::requireNoteById(const string& id) const noexcept(false) {
    auto note = getNoteById(id); // EXCEPTION HANDLING: reuse the non-throwing lookup for optional searches.
    if (!note) throw NoteNotFoundException(id); // EXCEPTION HANDLING: throw from callee; caller handles it.
    return note;
}

// ------------------------------------------------------------
// NEW CODE: Pointer to Objects
// ------------------------------------------------------------
Note* Vault::getRawNotePointer(const string& id) const {
    for (const auto& note : notes) {
        if (note->getId() == id) return note.get(); // raw pointer to the managed object
    }
    return nullptr;
}
// ------------------------------------------------------------

Indexer::Indexer(const Vault& vault) : vault(vault) {}

// THREADS: Runnable equivalent. operator() is the run() body; results stay private to the task.
namespace {
    class IndexTask {
    private:
        int id;
        const vector<shared_ptr<Note>>& notes;
        const regex& linkRegex;
        size_t begin, end;
        mutex& lock;        // guards console output and the shared counter
        size_t& totalLinks; // shared counter (synchronization demo)

        void say(const string& state) const {
            lock_guard<mutex> guard(lock);
            cout << "[Worker " << id << "] state: " << state << "\n";
        }

    public:
        vector<pair<string, string>> links; // (source title, linked title)
        exception_ptr error;

        IndexTask(int id, const vector<shared_ptr<Note>>& notes, const regex& linkRegex,
                  size_t begin, size_t end, mutex& lock, size_t& totalLinks)
            : id(id), notes(notes), linkRegex(linkRegex), begin(begin), end(end),
              lock(lock), totalLinks(totalLinks) {}

        void announce(const string& state) const { say(state); }

        void operator()() { // run()
            try {
                ostringstream running;
                running << "RUNNING (thread id " << this_thread::get_id() << ", " << (end - begin) << " notes)";
                say(running.str());

                this_thread::yield();
                say("TIMED_WAITING (sleep_for 20 ms)");
                this_thread::sleep_for(chrono::milliseconds(20));

                for (size_t n = begin; n < end; ++n) {
                    string content = notes[n]->getContent();
                    auto it = sregex_iterator(content.begin(), content.end(), linkRegex);
                    for (; it != sregex_iterator(); ++it) {
                        links.emplace_back(notes[n]->getTitle(), (*it)[1].str());
                    }
                }

                {
                    lock_guard<mutex> guard(lock); // synchronization: only one thread updates the counter
                    totalLinks += links.size();
                }
                say("TERMINATED (found " + to_string(links.size()) + " links)");
            } catch (...) {
                error = current_exception(); // rethrown by the main thread after join()
            }
        }
    };

    // THREADS: Thread wrapper ("extends Thread" equivalent). start() launches, join() waits.
    class WorkerThread {
    private:
        IndexTask& task;
        thread t;

    public:
        explicit WorkerThread(IndexTask& task) : task(task) {}
        void start() { t = thread(ref(task)); }
        void join() { if (t.joinable()) t.join(); }
        ~WorkerThread() { join(); }
    };
}

void Indexer::rebuildIndex() {
    backlinks.clear();
    forwardLinks.clear();

    const size_t workerCount = 2;
    cout << "Indexing notes using " << workerCount << " worker threads...\n";

    const regex linkRegex(R"(\[\[(.*?)\]\])"); // compiled once, read-only in threads
    const auto notes = vault.listAllNotes();
    const size_t chunk = (notes.size() + workerCount - 1) / workerCount;

    mutex lock;
    size_t totalLinks = 0;

    vector<unique_ptr<IndexTask>> tasks;
    vector<unique_ptr<WorkerThread>> workers;
    for (size_t w = 0; w < workerCount; ++w) {
        size_t begin = min(w * chunk, notes.size());
        size_t end = min(begin + chunk, notes.size());
        tasks.push_back(make_unique<IndexTask>(int(w + 1), cref(notes), cref(linkRegex), begin, end, lock, totalLinks));
        workers.push_back(make_unique<WorkerThread>(*tasks.back()));
    }

    for (size_t w = 0; w < workerCount; ++w) {
        tasks[w]->announce("NEW");
        workers[w]->start();00
        tasks[w]->announce("RUNNABLE (start() called)");
    }

    for (auto& worker : workers) worker->join(); // all threads finish before we continue

    // Merge in worker order in this thread only, so link order matches the old loop.
    for (auto& task : tasks) {
        if (task->error) rethrow_exception(task->error);
        for (const auto& link : task->links) {
            forwardLinks[link.first].push_back(link.second);
            backlinks[link.second].push_back(link.first);
        }
    }
    cout << "All workers joined. " << totalLinks << " links indexed.\n";
}

vector<string> Indexer::searchNotes(const string& query) const {
    vector<string> results;
    string q = query;
    for (const auto& note : vault.listAllNotes()) {
        if (note->getTitle().find(q) != string::npos || note->getContent().find(q) != string::npos) {
            results.push_back(note->getTitle());
        }
    }
    return results;
}

vector<string> Indexer::getBacklinks(const string& title) const {
    auto it = backlinks.find(title);
    if (it != backlinks.end()) return it->second;
    return {};
}

string Indexer::getReachableNotesBFS(const string& startTitle) const {
    unordered_set<string> visited;
    queue<string> q;
    vector<string> reachable;

    q.push(startTitle);
    visited.insert(startTitle);

    while (!q.empty()) {
        string current = q.front();
        q.pop();

        if (current != startTitle) {
            reachable.push_back(current);
        }

        auto it = forwardLinks.find(current);
        if (it != forwardLinks.end()) {
            for (const auto& neighbor : it->second) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    q.push(neighbor);
                }
            }
        }
    }

    ostringstream oss;
    if (reachable.empty()) {
        oss << "No connected notes.";
    } else {
        for (const auto& n : reachable) oss << n << " ";
    }
    return oss.str();
}

void Indexer::validateLinks() const noexcept(false) {
    unordered_set<string> knownTitles;
    for (const auto& note : vault.listAllNotes()) knownTitles.insert(note->getTitle());
    for (const auto& source : forwardLinks) {
        for (const auto& target : source.second) {
            if (knownTitles.find(target) == knownTitles.end()) throw InvalidLinkException(target);
        }
    }
}

string NoteComparator::compare(const Note& a, const Note& b) {
    ostringstream oss;
    oss << "Comparison between '" << a.getTitle() << "' and '" << b.getTitle() << "':\n";
    oss << "Length: " << a.getContent().length() << " chars vs " << b.getContent().length() << " chars.\n";
    oss << "Tags: " << a.getTags().size() << " vs " << b.getTags().size() << ".\n";
    return oss.str();
}
