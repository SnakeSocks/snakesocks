#ifndef R_SWLOCK_HPP
#define R_SWLOCK_HPP

#include <pthread.h>
namespace rlib {    
    [[deprecated]] class RWLock
    {
    public:
        RWLock() : isFree(true) {pthread_rwlock_init(&m_lock, NULL);}
        ~RWLock() {pthread_rwlock_destroy(&m_lock);}
        void acquireShared() {pthread_rwlock_rdlock(&m_lock);isFree = false;}
        void acquireExclusive() {pthread_rwlock_wrlock(&m_lock);isFree = false;}
        void release() {pthread_rwlock_unlock(&m_lock);isFree = true;}
    //    bool tryAcquireShared() {return pthread_rwlock_tryrdlock(&m_lock) == 0;}
    //    bool tryAcquireExclusive() {return pthread_rwlock_trywrlock(&m_lock) == 0;}
    private:
        pthread_rwlock_t m_lock;
        bool isFree;
    };
}

#endif
