#ifndef R_SIO_HPP
#define R_SIO_HPP

#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <stdexcept>

#ifndef WIN32
#include <sys/socket.h>
//POSIX Version
namespace rlib {
    class fdIO
    {
    public:
        static ssize_t readn(int fd, void *vptr, size_t n) noexcept //Return -1 on error, read bytes on success, blocks until nbytes done.
        {
            size_t  nleft;
            ssize_t nread;
            char   *ptr;
        
            ptr = (char *)vptr;
            nleft = n;
            while (nleft > 0) {
                if ( (nread = read(fd, ptr, nleft)) < 0) {
                    if (errno == EINTR)
                        nread = 0;      /* and call read() again */
                    else
                        return (-1);
                } else if (nread == 0)
                    return (-1);              /* EOF */
            
                nleft -= nread;
                ptr += nread;
            }
            return (n);         /* return success */
        }
        static ssize_t writen(int fd, const void *vptr, size_t n) noexcept //Return -1 on error, read bytes on success, blocks until nbytes done.
        {
            size_t nleft;
            ssize_t nwritten;
            const char *ptr;
        
            ptr = (const char *)vptr;
            nleft = n;
            while (nleft > 0) {
                if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
                    if (nwritten < 0 && errno == EINTR)
                        nwritten = 0;   /* and call write() again */
                    else
                        return (-1);    /* error */
                 }
             
                 nleft -= nwritten;
                 ptr += nwritten;
            }
            return (n);
        }
        static ssize_t readall(int fd, void **pvptr, size_t initSize) noexcept //Return -1 on error, read bytes on success. pvptr must be a malloc/calloced buffer, I'll malloc one if *pvptr is NULL.
        {
            size_t current = initSize ? initSize : 1024;
            void *vptr = *pvptr;
            if(vptr == NULL)
                vptr = malloc(current);
            void *currvptr = vptr;
        
            {
                ssize_t ret = read(fd, currvptr, current / 2);
                if(ret == -1) return -1;
                if(ret < current / 2)
                {
                    *pvptr = vptr;
                    return ret;
                }
                currvptr = (char *)vptr + current / 2;
            }
        
            while(true)
            {
                ssize_t ret = read(fd, currvptr, current / 2);
                if(ret == -1) return -1;
                if(ret < current)
                {
                    *pvptr = vptr;
                    return ret + current / 2;
                }
            
                current *= 2;
                void *vptrBackup = vptr;
                if((vptr = realloc(vptr, current)) == NULL) {
                    free(vptrBackup);
                    errno = EMSGSIZE;
                    return -1;
                }
                currvptr = (char *)vptr + current / 2;
            }
        }
        static void readn_ex(int fd, void *vptr, size_t n) //never return error.
        {
            auto ret = readn(fd, vptr, n);
            if(ret == -1) throw std::runtime_error("readn failed.");
        }
        static void writen_ex(int fd, const void *vptr, size_t n)
        {
            auto ret = writen(fd, vptr, n);
            if(ret == -1) throw std::runtime_error("writen failed.");
        }
        static ssize_t readall_ex(int fd, void **pvptr, size_t initSize) //never return -1
        {
            auto ret = readall(fd, pvptr, initSize);
            if(ret == -1) throw std::runtime_error("readall failed.");
            return ret;
        }
    };

    class sockIO 
    {
    public:
        static ssize_t recvn(int fd, void *vptr, size_t n, int flags) noexcept //Return -1 on error, read bytes on success, blocks until nbytes done.
        {
            size_t  nleft;
            ssize_t nread;
            char   *ptr;
        
            ptr = (char *)vptr;
            nleft = n;
            while (nleft > 0) {
                if ( (nread = recv(fd, ptr, nleft, flags)) < 0) {
                    if (errno == EINTR)
                        nread = 0;      /* and call read() again */
                    else
                        return (-1);
                } else if (nread == 0)
                    return -1;              /* EOF */
            
                nleft -= nread;
                ptr += nread;
            }
            return (n);         /* return success */
        }
        static ssize_t sendn(int fd, const void *vptr, size_t n, int flags) noexcept //Return -1 on error, read bytes on success, blocks until nbytes done.
        {
            size_t nleft;
            ssize_t nwritten;
            const char *ptr;
        
            ptr = (const char *)vptr;
            nleft = n;
            while (nleft > 0) {
                if ( (nwritten = send(fd, ptr, nleft, flags)) <= 0) {
                    if (nwritten < 0 && errno == EINTR)
                        nwritten = 0;   /* and call write() again */
                    else
                        return (-1);    /* error */
                 }
             
                 nleft -= nwritten;
                 ptr += nwritten;
            }
            return (n);
        }
        static ssize_t recvall(int fd, void **pvptr, size_t initSize, int flags) noexcept //Return -1 on error, read bytes on success. pvptr must be a malloc/calloced buffer, I'll malloc one if *pvptr is NULL.
        {
            size_t current = initSize ? initSize : 1024;
            void *vptr = *pvptr;
            if(vptr == NULL)
                vptr = malloc(current);
            void *currvptr = vptr;
        
            {
                ssize_t ret = recv(fd, currvptr, current / 2, flags);
                if(ret == -1) return -1;
                if(ret < current / 2)
                {
                    *pvptr = vptr;
                    return ret;
                }
                currvptr = (char *)vptr + current / 2;
            }
        
            while(true)
            {
                ssize_t ret = recv(fd, currvptr, current / 2, flags);
                if(ret == -1) return -1;
                if(ret < current)
                {
                    *pvptr = vptr;
                    return ret + current / 2;
                }
            
                current *= 2;
                void *vptrBackup = vptr;
                if((vptr = realloc(vptr, current)) == NULL) {
                    free(vptrBackup);
                    errno = EMSGSIZE;
                    return -1;
                }
                currvptr = (char *)vptr + current / 2;
            }
        }
        static void recvn_ex(int fd, void *vptr, size_t n, int flags) //return read bytes.
        {
            auto ret = recvn(fd, vptr, n, flags);
            if(ret == -1) throw std::runtime_error("recvn failed.");
        }
        static ssize_t sendn_ex(int fd, const void *vptr, size_t n, int flags)
        {
            auto ret = sendn(fd, vptr, n, flags);
            if(ret == -1) throw std::runtime_error("sendn failed.");
            return ret;
        }
        static ssize_t recvall_ex(int fd, void **pvptr, size_t initSize, int flags) //never return -1
        {
            auto ret = recvall(fd, pvptr, initSize, flags);
            if(ret == -1) throw std::runtime_error("recvall failed.");
            return ret;
        }
    };
}
#else
#include <winsock2.h>
//WINsock version
namespace rlib {
    class sockIO 
    {
    private:
        static int WSASafeGetLastError()
        {
            int i;
            WSASetLastError(i = WSAGetLastError());
            return i;
        }
    public:
        static ssize_t recvn(SOCKET fd, char *vptr, size_t n, int flags) noexcept //Return -1 on error, read bytes on success, blocks until nbytes done.
        {
            size_t  nleft;
            ssize_t nread;
            char   *ptr;
        
            ptr = (char *)vptr;
            nleft = n;
            while (nleft > 0) {
                if ( (nread = recv(fd, ptr, nleft, flags)) == SOCKET_ERROR) {
                    if (WSASafeGetLastError() == WSAEINTR)
                        nread = 0;      /* and call read() again */
                    else
                        return (-1);
                } else if (nread == 0)
                    return (-1);              /* EOF */
            
                nleft -= nread;
                ptr += nread;
            }
            return (n);         /* return >= 0 */
        }
        static ssize_t sendn(SOCKET fd, const char *vptr, size_t n, int flags) noexcept //Return -1 on error, read bytes on success, blocks until nbytes done.
        {
            size_t nleft;
            ssize_t nwritten;
            const char *ptr;
        
            ptr = (const char *)vptr;
            nleft = n;
            while (nleft > 0) {
                if ( (nwritten = send(fd, ptr, nleft, flags)) <= 0) {
                    if (nwritten == SOCKET_ERROR && WSASafeGetLastError() == WSAEINTR)
                        nwritten = 0;   /* and call write() again */
                    else
                        return (-1);    /* error */
                 }
             
                 nleft -= nwritten;
                 ptr += nwritten;
            }
            return (n);
        }
        static ssize_t recvall(SOCKET fd, void **pvptr, size_t initSize, int flags) noexcept //Return -1 on error, read bytes on success. pvptr must be a malloc/calloced buffer, I'll malloc one if *pvptr is NULL.
        {
            size_t current = initSize ? initSize : 1024;
            void *vptr = *pvptr;
            if(vptr == NULL)
                vptr = malloc(current);
            void *currvptr = vptr;
        
            {
            _retry_1:
                ssize_t ret = recv(fd, (char *)currvptr, current / 2, flags); 
                if(ret == SOCKET_ERROR) {
                    if(WSASafeGetLastError() == WSAEINTR)
                        goto _retry_1;
                    return SOCKET_ERROR;
                }
                if(ret < current / 2)
                {
                    *pvptr = vptr;
                    return ret;
                }
                currvptr = (char *)vptr + current / 2;
            }   
        
            while(true)
            {
                ssize_t ret = recv(fd, (char *)currvptr, current / 2, flags); 
                if(ret == SOCKET_ERROR) {
                    if(WSASafeGetLastError() == WSAEINTR)
                        continue; //retry
                    return SOCKET_ERROR;
                }
                if(ret < current)
                {
                    *pvptr = vptr;
                    return ret + current / 2;
                }
        
                current *= 2;
                void *vptrBackup = vptr;
                if((vptr = realloc(vptr, current)) == NULL) {
                    free(vptrBackup);
                    WSASetLastError(WSAEMSGSIZE);
                    return SOCKET_ERROR;
                }   
                currvptr = (char *)vptr + current / 2;
            }
        }
        static void recvn_ex(SOCKET fd, char *vptr, size_t n, int flags) //never return error.
        {
            auto ret = recvn(fd, vptr, n, flags);
            if(ret == -1) throw std::runtime_error("recvn failed.");
        }
        static ssize_t sendn_ex(SOCKET fd, const char *vptr, size_t n, int flags)
        {
            auto ret = sendn(fd, vptr, n, flags);
            if(ret == -1) throw std::runtime_error("recvn failed.");
						return ret;
        }
        static ssize_t recvall_ex(SOCKET fd, void **pvptr, size_t initSize, int flags) //never return -1
        {
            auto ret = recvall(fd, pvptr, initSize, flags);
            if(ret == -1) throw std::runtime_error("recvn failed.");
            return ret;
        }
    };

    class fdIO
    {
    public:
        static ssize_t readn(SOCKET fd, void *vptr, size_t n) noexcept //Return -1 on error, read bytes on success, blocks until nbytes done.
        {
            return sockIO::recvn(fd, (char *)vptr, n, 0);
        }
        static ssize_t writen(SOCKET fd, const void *vptr, size_t n) noexcept //Return -1 on error, read bytes on success, blocks until nbytes done.
        {
            return sockIO::sendn(fd, (const char *)vptr, n, 0);
        }
        static ssize_t readall(SOCKET fd, void **pvptr, size_t initSize) noexcept //Return -1 on error, read bytes on success. pvptr must be a malloc/calloced buffer, I'll malloc one if *pvptr is NULL.
        {
            return sockIO::recvall(fd, pvptr, initSize, 0);
        }
        static void readn_ex(SOCKET fd, void *vptr, size_t n) //return read bytes.
        {
            return sockIO::recvn_ex(fd, (char *)vptr, n, 0);
        }
        static ssize_t writen_ex(SOCKET fd, const void *vptr, size_t n)
        {
            return sockIO::sendn_ex(fd, (const char *)vptr, n, 0);
        }
        static ssize_t readall_ex(SOCKET fd, void **pvptr, size_t initSize) //never return -1
        {
            return sockIO::recvall_ex(fd, pvptr, initSize, 0);
        }
    };
}

#endif


#endif
