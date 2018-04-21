/* Exception safe usage:
 *
 * reinforce_scope_begin(_gname, [](){do_sth();})
 * do_something();
 * reinforce_scope_end(_gname)
 *
 */

#ifndef R_SCOPE_GUARD
#define R_SCOPE_GUARD

#include <rlib/require/cxx11>
#include <functional>
#include <rlib/class_decorator.hpp>

namespace rlib {
    class scope_guard : private noncopyable
    {
    public: 
        template<class Callable> 
        scope_guard(Callable && undo_func) : f(std::forward<Callable>(undo_func)) {}

        scope_guard(scope_guard && other) : f(std::move(other.f)) {
            other.f = nullptr;
        }

        ~scope_guard() {
            if(f) f(); // must not throw
        }

        void dismiss() noexcept {
            f = nullptr;
        }

        void force_call() noexcept {
            if(f) f();
            dismiss();
        }

    private:
        std::function<void()> f;
    };
}


#ifndef defer
#include <rlib/macro.hpp>
#define defer(callable) ::rlib::scope_guard RLIB_MAKE_UNIQUE_NAME(_guarder_id_) (callable)
#endif

#define RLIB_reinforce_scope_begin(guarderName, callable) scope_guard guarderName = callable; try{
#define RLIB_reinforce_scope_end(guarderName) } catch(...) { guarderName.force_call(); throw;}

/*
scope_guards scope_exit, scope_fail;

action1();
scope_exit += []{ cleanup1(); };
scope_fail += []{ rollback1(); };

action2();
scope_exit += []{ cleanup2(); };
scope_fail += []{ rollback2(); };

do_something();

scope_fail.dismiss();
*/

#include <deque>

namespace rlib {
    class scope_guards : private noncopyable
    {
    public:
        template<class Callable>
        scope_guards& operator += (Callable && undo_func) {
            fbuf.emplace_front(std::forward<Callable>(undo_func));
        }

        ~scope_guards() {
            force_call();
        }

        void dismiss() noexcept {
            fbuf.clear();
        }

        void force_call() noexcept {
            for(auto &f : fbuf) f();
            dismiss();
        }
    private:
        std::deque<std::function<void(void)> > fbuf;
    };
}


#endif
