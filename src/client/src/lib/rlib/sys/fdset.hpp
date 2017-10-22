#ifndef R_FDSET_HPP
#define R_FDSET_HPP

#include <unistd.h>
#include <sys/types.h>
namespace rlib{
    class FileDescriptorSet
    {
    public:
        using fd=int;
        FileDescriptorSet() : m_size(0), maxFileDescriptor(NULL) {FD_ZERO(&m_fds_data);}
        void push(fd FileDescriptor) {FD_SET(FileDescriptor, &m_fds_data); ++m_size; maxFileDescriptor = (maxFileDescriptor > FileDescriptor ? maxFileDescriptor : FileDescriptor);}
        void pop(fd FileDescriptor) {FD_CLR(FileDescriptor, &m_fds_data); --m_size;} //It will break maxFileDescriptor.(for performance reason).
        void clear() {FD_ZERO(&m_fds_data); m_size = 0;maxFileDescriptor = 0;}
        bool check(fd FileDescriptor) {return FD_ISSET(FileDescriptor, &m_fds_data);}
        size_t size() const {return m_size;}
        int getMaxFileDescriptor() const {return maxFileDescriptor;}
        fd_set *getptr() {return &m_fds_data;}
    private:
        fd_set m_fds_data;
        size_t m_size;
        int maxFileDescriptor;
    };
}
#endif
