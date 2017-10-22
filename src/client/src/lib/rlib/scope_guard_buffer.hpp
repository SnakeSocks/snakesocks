/*
scope_guards scope_exit, scope_fail;

action1();
scope_exit += [](){ cleanup1(); };
scope_fail += [](){ rollback1(); };

action2();
scope_exit += [](){ cleanup2(); };
scope_fail += [](){ rollback2(); };

//...

scope_fail.dismiss();
*/

//TODO: Undone.
#ifndef R_SCOPE_GUARD_BUFFER_HPP
#define R_SCOPE_GUARD_BUFFER_HPP

#include <functional>
#include <deque>
#include "noncopyable.hpp"

namespace rlib {
    class scope_guards : public std::deque<std::function<void()>>, private noncopyable
    {
    public:
        template<class Callable>
        scope_guards& operator += (Callable && undo_func) {
            emplace_front(std::forward<Callable>(undo_func));
        }
    
        ~scope_guards() {
            for(auto &f : *this) f(); // must not throw
        }
    
        void dismiss() noexcept {
            clear();
        }
    
        scope_guards() = default;
        scope_guards(const scope_guards&) = delete;
        void operator = (const scope_guards&) = delete;
    };
}

#endif
