/* Exception safe usage:
 *
 * reinforce_scope_begin(_gname, [](){do_sth();})
 * 1+1;
 * 1+1=2;
 * 2+2=4;
 * reinforce_scope_end(_gname)
 *
 */

#ifndef R_SCOPE_GUARD
#define R_SCOPE_GUARD

//#include <rlib/require/cxx11>
#include <functional>
#include "noncopyable.hpp"

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

        scope_guard(const scope_guard&) = delete;
        void operator = (const scope_guard&) = delete;

    private:
        std::function<void()> f;
    };
}

// This is some crazy magic that helps produce __BASE__247
// Vanilla interpolation of __BASE__##__LINE__ would produce __BASE____LINE__
// I still can't figure out why it works, but it has to do with macro resolution ordering
#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res
#define RLIB_UNIQUE_NAME(base) PP_CAT(base, __COUNTER__)
#ifndef defer
#define defer(callable) ::rlib::scope_guard RLIB_UNIQUE_NAME(_guarder_id_) (callable)
#endif

#define reinforce_scope_begin(guarderName, callable) scope_guard guarderName = callable; try{
#define reinforce_scope_end(guarderName) } catch(...) { guarderName.force_call(); throw;}


#endif
