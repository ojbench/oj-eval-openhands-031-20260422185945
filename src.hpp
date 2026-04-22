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

    std::shared_ptr<Inner> ptr;      // owning reference
    std::weak_ptr<Inner> wptr;       // non-owning reference (to break cycles)
    bool weak = false;               // whether this handle uses weak ref

    // Context: if this handle is an element inside a container, parent_ctx points to that container's Inner
    std::weak_ptr<Inner> parent_ctx;

    std::shared_ptr<Inner> get_shared() const {
        return weak ? wptr.lock() : ptr;
    }

    static void print_impl(std::ostream &os, const pylist &x, std::unordered_set<const Inner*> &vis) {
        auto p = x.get_shared();
        if (!p) { os << "[]"; return; }
        if (p->type == Type::INT) { os << p->iv; return; }
        const Inner* id = p.get();
        if (vis.count(id)) { os << "[...]"; return; }
        vis.insert(id);
        os << "[";
        for (size_t i = 0; i < p->lv.size(); ++i) {
            if (i) os << ", ";
            print_impl(os, p->lv[i], vis);
        }
        os << "]";
        vis.erase(id);
    }

    bool same_inner(const pylist &o) const {
        auto a = get_shared();
        auto b = o.get_shared();
        return a && b && a.get() == b.get();
    }

    static pylist make_weak_to(const std::shared_ptr<Inner> &target) {
        pylist r;
        r.ptr.reset();
        r.wptr = target;
        r.weak = true;
        return r;
    }

public:
    pylist() : ptr(std::make_shared<Inner>()), wptr(), weak(false), parent_ctx() {}
    pylist(int v) : ptr(std::make_shared<Inner>(v)), wptr(), weak(false), parent_ctx() {}

    pylist(const pylist&) = default;
    // Custom assignment to handle cycle prevention when assigning parent to itself
    pylist& operator=(const pylist& rhs) {
        if (this == &rhs) return *this;
        auto parent = parent_ctx.lock();
        auto rhs_p = rhs.get_shared();
        if (parent && rhs_p && parent.get() == rhs_p.get()) {
            ptr.reset();
            wptr = rhs_p;
            weak = true;
        } else {
            ptr = rhs_p;
            wptr.reset();
            weak = false;
        }
        return *this;
    }

    // Assignment from int: rebind this handle to a new int value
    pylist& operator=(int v) {
        ptr = std::make_shared<Inner>(v);
        wptr.reset();
        weak = false;
        parent_ctx.reset();
        return *this;
    }

    // Append operations (only valid when this is a list)
    void append(const pylist &x) {
        ensure_list();
        auto p = get_shared();
        // Avoid ownership cycles on self-append
        if (x.get_shared() && p && x.get_shared().get() == p.get()) {
            p->lv.push_back(make_weak_to(p));
        } else {
            p->lv.push_back(x);
        }
    }

    void append(int v) {
        ensure_list();
        get_shared()->lv.emplace_back(v);
    }

    // Pop from back; if empty or not a list, return empty list
    pylist pop() {
        auto p = get_shared();
        if (!p || p->type != Type::LIST || p->lv.empty()) return pylist();
        pylist back = p->lv.back();
        p->lv.pop_back();
        return back;
    }

    // Indexing
    pylist &operator[](size_t i) {
        ensure_list();
        auto p = get_shared();
        pylist &child = p->lv[i];
        // mark child's parent context to detect self-assignment
        child.parent_ctx = p;
        return child;
    }

    const pylist &operator[](size_t i) const {
        return get_shared()->lv[i];
    }

    // Conversion to int for arithmetic and comparisons
    operator int() const {
        auto p = get_shared();
        return p && p->type == Type::INT ? p->iv : 0;
    }

    // Assignment from another pylist with cycle prevention when assigning parent to its element
    pylist& assign_from(const pylist &rhs) {
        auto parent = parent_ctx.lock();
        auto rhs_p = rhs.get_shared();
        if (parent && rhs_p && parent.get() == rhs_p.get()) {
            // assign as weak reference to the parent to avoid cycle
            ptr.reset();
            wptr = rhs_p;
            weak = true;
        } else {
            ptr = rhs_p;
            wptr.reset();
            weak = false;
        }
        // keep parent_ctx as is (still inside container)
        return *this;
    }


    friend std::ostream &operator<<(std::ostream &os, const pylist &ls) {
        auto p = ls.get_shared();
        if (!p) { os << "[]"; return os; }
        if (p->type == Type::INT) { os << p->iv; return os; }
        std::unordered_set<const Inner*> vis;
        print_impl(os, ls, vis);
        return os;
    }

private:
    void ensure_list() {
        auto p = get_shared();
        if (!p) {
            ptr = std::make_shared<Inner>();
            wptr.reset();
            weak = false;
            parent_ctx.reset();
            return;
        }
        if (p->type == Type::INT) {
            ptr = std::make_shared<Inner>();
            wptr.reset();
            weak = false;
            parent_ctx.reset();
        }
    }
};

#endif // PYLIST_H
