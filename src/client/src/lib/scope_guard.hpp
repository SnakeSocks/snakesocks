/*
 // do step 1
step1();
scope_guard guard1 = [&]() {
    // revert step 1
    revert1();
};

// step 2
step2();
guard1.dismiss();
 */

/* Exception safe usage:
 *
 * reinforce_scope_begin(_gname, [](){do_sth();})
 * 1+1;
 * 1+1=2;
 * 2+2=4;
 * reinforce_scope_end(_gname)
 *
 */


#ifndef _lib_scope_guard
#define _lib_scope_guard

#include <functional>

class scope_guard {
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

    scope_guard(const scope_guard&) = delete;
    void operator = (const scope_guard&) = delete;

private:
    std::function<void()> f;
};

#define defer(callable) scope_guard _guarder_id_##__COUNTER__(callable)
#define reinforce_scope_begin(guarderName, callable) scope_guard guarderName = callable; try{
#define reinforce_scope_end(guarderName) } catch(...) { guarderName.force_call(); throw;}

#endif
