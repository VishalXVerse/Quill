#ifndef TEMPLATES_H
#define TEMPLATES_H

// ------------------------------------------------------------------
// NEW CODE: Templates
//
// Templates are compiled per type, so their definitions must be
// visible to every file that uses them. That is why everything here
// lives in a header (there is no Templates.cpp).
// ------------------------------------------------------------------

#include "Models.h"
#include <algorithm>
#include <cstddef>
#include <deque>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// ==================================================================
// FUNCTION TEMPLATES
// ==================================================================

// Generic maximum. Works for any T that supports operator<
// (int, double, string, and even Priority, which converts to int).
template <typename T>
const T& maxOf(const T& a, const T& b) {
    return (a < b) ? b : a;
}

// Explicit (full) specialization for Note: Note has no operator<, and
// "bigger" for a note should mean "longer content", so we give it its
// own version. 'inline' is required because this lives in a header.
template <>
inline const Note& maxOf<Note>(const Note& a, const Note& b) {
    return (a.getContent().length() < b.getContent().length()) ? b : a;
}

// Keep a value inside [lo, hi].
template <typename T>
T clampValue(T value, T lo, T hi) {
    return (value < lo) ? lo : ((hi < value) ? hi : value);
}

// Generic swap using move semantics.
template <typename T>
void swapValues(T& a, T& b) {
    T tmp = move(a);
    a = move(b);
    b = move(tmp);
}

// Two template parameters: the element type T and a predicate type
// Pred (deduced from the lambda / function passed in).
// Returns copies of every element for which pred(element) is true.
template <typename T, typename Pred>
vector<T> filterItems(const vector<T>& items, Pred pred) {
    vector<T> result;
    for (const auto& item : items) {
        if (pred(item)) result.push_back(item);
    }
    return result;
}

// Joins any vector whose elements support operator<< into one string.
template <typename T>
string joinToString(const vector<T>& items, const string& sep = ", ") {
    ostringstream oss;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) oss << sep;
        oss << items[i];
    }
    return oss.str();
}

// ==================================================================
// CLASS TEMPLATES
// ==================================================================

// Registry<Key, Value>: a generic keyed collection (two type
// parameters). Examples used in the demo:
//   Registry<string, int>                    -> tag usage counts
//   Registry<string, shared_ptr<Note>>       -> notes by title
template <typename Key, typename Value>
class Registry {
private:
    map<Key, Value> items;

public:
    void add(const Key& key, const Value& value) { items[key] = value; }

    bool remove(const Key& key) { return items.erase(key) > 0; }

    bool contains(const Key& key) const { return items.find(key) != items.end(); }

    // Non-owning pointer to the stored value, or nullptr if absent.
    const Value* find(const Key& key) const {
        auto it = items.find(key);
        return (it == items.end()) ? nullptr : &it->second;
    }

    // Inserts a default-constructed Value if the key is new.
    Value& operator[](const Key& key) { return items[key]; }

    size_t size() const { return items.size(); }

    // Calls fn(key, value) for every entry, in key order.
    template <typename Fn>
    void forEach(Fn fn) const {
        for (const auto& kv : items) fn(kv.first, kv.second);
    }
};

// RecentList<T, N>: keeps the N most recently used items, newest first.
// 'N' is a NON-TYPE template parameter, so the capacity is fixed at
// compile time and is part of the type: RecentList<int,3> and
// RecentList<int,5> are different types.
template <typename T, size_t N>
class RecentList {
private:
    deque<T> items;

public:
    void push(const T& value) {
        // If it is already in the list, move it to the front.
        auto it = find(items.begin(), items.end(), value);
        if (it != items.end()) items.erase(it);

        items.push_front(value);
        if (items.size() > N) items.pop_back();
    }

    size_t size() const { return items.size(); }
    static constexpr size_t capacity() { return N; }

    vector<T> toVector() const {
        return vector<T>(items.begin(), items.end());
    }
};

#endif
