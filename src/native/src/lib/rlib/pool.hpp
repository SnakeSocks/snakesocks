#ifndef _RLIB_OBJ_POOL_HPP
#define _RLIB_OBJ_POOL_HPP 1

#include <rlib/impl/traceable_list.hpp>
#include <rlib/class_decorator.hpp>
#include <utility>
#include <tuple>
#include <functional>
#include <algorithm>

#ifdef RLIB_SWITCH_USE_MINGW_THREAD_FIX
#include <mingw.mutex.h>
#include <mingw.thread.h>
#include <mingw.condition_variable.h>
#else
#include <thread>
#include <mutex>
#include <condition_variable>
#endif

namespace rlib {
    /*
     * Multi-threaded object_pool. It will block current thread and wait if
     *     borrow_one() starves, until some other threads release their obj.
     */
    template<typename obj_t, typename... _bound_construct_args_t>
    class fixed_object_pool : rlib::nonmovable {
    protected:
        using element_t = obj_t;
        using buffer_t = impl::traceable_list<obj_t, bool>;
        using this_type = fixed_object_pool<obj_t, _bound_construct_args_t ...>;
    public:
        explicit fixed_object_pool(size_t max_size, _bound_construct_args_t ... _args)
                : max_size(max_size), _bound_args(std::forward<_bound_construct_args_t>(_args) ...) 
        {}

        void fill_full() {
            for (size_t cter = 0; cter < max_size; ++cter) {
                new_obj_to_buffer();
                free_list.push_back(&*--buffer.end());
            }
        }

        // `new` an object. Return nullptr if pool is full.
        obj_t *try_borrow_one() {
            std::lock_guard<std::mutex> _l(buffer_mutex);
            return do_try_borrow_one();
        }
        obj_t *borrow_one() {
            auto result = try_borrow_one();
            if(result)
                return result;
            // Not available. Wait for release_one.
            std::unique_lock<std::mutex> lk(buffer_mutex);

            borrow_cv.wait(lk, [this]{return this->new_obj_ready;});

            result = do_try_borrow_one();
            lk.unlock();
            if(!result)
                throw std::logic_error("unknown par error.");
            return result;
        }
        void release_one(obj_t *which) {
            {
                std::lock_guard<std::mutex> _l(buffer_mutex);
                free_list.push_front(which);
                typename buffer_t::iterator elem_iter(which);
                elem_iter.get_extra_info() = true; // mark as free.
                new_obj_ready = true;
            } // lock released.
            borrow_cv.notify_one();
        }

        void reconstruct_one(obj_t *which) {
            reconstruct_impl(which, std::make_index_sequence<sizeof...(_bound_construct_args_t)>());
        }

    protected:
        buffer_t buffer; // list<obj_t obj, bool is_free>
    private:
        std::tuple<_bound_construct_args_t ...> _bound_args;

        size_t max_size;
        std::list<obj_t *> free_list;
        std::mutex buffer_mutex;
        std::condition_variable borrow_cv;
        volatile bool new_obj_ready = false;

        // try_borrow_one without lock.
        obj_t *do_try_borrow_one() {
            // Optimize here if is performance bottleneck (lockless list... etc...)
            borrow_again:
            if (free_list.size() > 0) {
                // Some object is free. Just return one.
                obj_t *result = *free_list.begin();
                free_list.pop_front();

                typename buffer_t::iterator elem_iter(result);
                elem_iter.get_extra_info() = false; // mark as busy.
                new_obj_ready = false;
                return result;
            }
            if (buffer.size() < max_size) {
                new_obj_to_buffer();
                free_list.push_back(&*--buffer.end());
                goto borrow_again;
            }
            return nullptr;
        }

        // fake emplace_back
        template<size_t ... index_seq>
        inline void new_obj_to_buffer_impl(std::index_sequence<index_seq ...>) {
            buffer.emplace_one(buffer.end(), true, std::get<index_seq>(_bound_args) ...);
        }
        template<size_t ... index_seq>
        inline void reconstruct_impl(obj_t *which, std::index_sequence<index_seq ...>) {
            which->~obj_t();
            new(which) obj_t(std::get<index_seq>(_bound_args) ...);
        }

        inline void new_obj_to_buffer() {
            new_obj_to_buffer_impl(std::make_index_sequence<sizeof...(_bound_construct_args_t)>());
        }
    };
}

#endif
