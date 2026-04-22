#ifndef PYLIST_H
#define PYLIST_H

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_set>

class pylist {
public:
    enum class Type { INT, LIST };

private:
    struct Inner {
        Type type;
        int iv;
        std::vector<pylist> lv;
        Inner() : type(Type::LIST), iv(0), lv() {}
        explicit Inner(int v) : type(Type::INT), iv(v), lv() {}
    };

    std::shared_ptr<Inner> ptr;

    static void print_impl(std::ostream &os, const pylist &x, std::unordered_set<const Inner*> &vis) {
        if (!x.ptr) { os << "[]"; return; }
        if (x.ptr->type == Type::INT) {
            os << x.ptr->iv;
            return;
        }
        const Inner* id = x.ptr.get();
        if (vis.count(id)) {
            os << "[...]";
            return;
        }
        vis.insert(id);
        os << "[";
        for (size_t i = 0; i < x.ptr->lv.size(); ++i) {
            if (i) os << ", ";
            print_impl(os, x.ptr->lv[i], vis);
        }
        os << "]";
        vis.erase(id);
    }

public:
    // Constructors
    pylist() : ptr(std::make_shared<Inner>()) {}
    pylist(int v) : ptr(std::make_shared<Inner>(v)) {}

    // Copy and assign default to share the same inner
    pylist(const pylist&) = default;
    pylist& operator=(const pylist&) = default;

    // Assignment from int: rebind this handle to a new int value
    pylist& operator=(int v) {
        ptr = std::make_shared<Inner>(v);
        return *this;
    }

    // Append operations (only valid when this is a list)
    void append(const pylist &x) {
        ensure_list();
        ptr->lv.push_back(x);
    }

    void append(int v) {
        ensure_list();
        ptr->lv.emplace_back(v);
    }

    // Pop from back; if empty or not a list, return empty list
    pylist pop() {
        if (!ptr || ptr->type != Type::LIST || ptr->lv.empty()) return pylist();
        pylist back = ptr->lv.back();
        ptr->lv.pop_back();
        return back;
    }

    // Indexing
    pylist &operator[](size_t i) {
        ensure_list();
        return ptr->lv[i];
    }

    const pylist &operator[](size_t i) const {
        return ptr->lv[i];
    }

    // Conversion to int for arithmetic and comparisons
    operator int() const {
        return ptr && ptr->type == Type::INT ? ptr->iv : 0;
    }

    friend std::ostream &operator<<(std::ostream &os, const pylist &ls) {
        if (!ls.ptr) { os << "[]"; return os; }
        if (ls.ptr->type == Type::INT) { os << ls.ptr->iv; return os; }
        std::unordered_set<const Inner*> vis;
        print_impl(os, ls, vis);
        return os;
    }

private:
    void ensure_list() {
        if (!ptr) ptr = std::make_shared<Inner>();
        if (ptr->type == Type::INT) {
            // Rebind to a new, empty list if it's currently an int
            ptr = std::make_shared<Inner>();
        }
    }
};

#endif // PYLIST_H
