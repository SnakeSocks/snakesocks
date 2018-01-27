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

#ifndef R_SCOPE_GUARD_BUFFER_HPP
#define R_SCOPE_GUARD_BUFFER_HPP

#include <rlib/require/cxx11>
#include <functional>
#include <deque>
#include <rlib/class_decorator.hpp>

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
    };
}

#endif
