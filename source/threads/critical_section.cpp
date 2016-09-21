/*!
    \file critical_section.cpp
    \brief Critical section synchronization primitive implementation
    \author Ivan Shynkarenka
    \date 27.01.2016
    \copyright MIT License
*/

#include "threads/critical_section.h"

#include "threads/thread.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#undef Yield
#elif defined(unix) || defined(__unix) || defined(__unix__)
#include "errors/exceptions.h"
#include "errors/fatal.h"
#include <pthread.h>
#endif

namespace CppCommon {

class CriticalSection::Impl
{
public:
    Impl()
    {
#if defined(_WIN32) || defined(_WIN64)
        InitializeCriticalSection(&_lock);
#elif defined(unix) || defined(__unix) || defined(__unix__)
        pthread_mutexattr_t mutex_attribute;
        int result = pthread_mutexattr_init(&mutex_attribute);
        if (result != 0)
            throwex SystemException("Failed to initialize a mutex attribute!", result);
        result = pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
        if (result != 0)
            throwex SystemException("Failed to set a mutex recursive attribute!", result);
        result = pthread_mutex_init(&_lock, &mutex_attribute);
        if (result != 0)
            throwex SystemException("Failed to initialize a mutex!", result);
        result = pthread_mutexattr_destroy(&mutex_attribute);
        if (result != 0)
            throwex SystemException("Failed to destroy a mutex attribute!", result);
#endif
    }

    ~Impl()
    {
#if defined(_WIN32) || defined(_WIN64)
        DeleteCriticalSection(&_lock);
#elif defined(unix) || defined(__unix) || defined(__unix__)
        int result = pthread_mutex_destroy(&_lock);
        if (result != 0)
            fatality(SystemException("Failed to destroy a mutex!", result));
#endif
    }

    bool TryLock()
    {
#if defined(_WIN32) || defined(_WIN64)
        return (TryEnterCriticalSection(&_lock) != 0);
#elif defined(unix) || defined(__unix) || defined(__unix__)
        int result = pthread_mutex_trylock(&_lock);
        if ((result != 0) && (result != EBUSY))
            throwex SystemException("Failed to try lock a mutex!", result);
        return (result == 0);
#endif
    }

    void Lock()
    {
#if defined(_WIN32) || defined(_WIN64)
        EnterCriticalSection(&_lock);
#elif defined(unix) || defined(__unix) || defined(__unix__)
        int result = pthread_mutex_lock(&_lock);
        if (result != 0)
            throwex SystemException("Failed to lock a mutex!", result);
#endif
    }

    void Unlock()
    {
#if defined(_WIN32) || defined(_WIN64)
        LeaveCriticalSection(&_lock);
#elif defined(unix) || defined(__unix) || defined(__unix__)
        int result = pthread_mutex_unlock(&_lock);
        if (result != 0)
            throwex SystemException("Failed to unlock a mutex!", result);
#endif
    }

private:
#if defined(_WIN32) || defined(_WIN64)
    CRITICAL_SECTION _lock;
#elif defined(unix) || defined(__unix) || defined(__unix__)
    pthread_mutex_t _lock;
#endif
};

CriticalSection::CriticalSection() : _pimpl(std::make_unique<Impl>())
{
}

CriticalSection::CriticalSection(CriticalSection&& cs) noexcept : _pimpl(std::move(cs._pimpl))
{
}

CriticalSection::~CriticalSection()
{
}

CriticalSection& CriticalSection::operator=(CriticalSection&& cs) noexcept
{
    _pimpl = std::move(cs._pimpl);
    return *this;
}

bool CriticalSection::TryLock()
{
    return _pimpl->TryLock();
}

bool CriticalSection::TryLockFor(const Timespan& timespan)
{
    // Calculate a finish timestamp
    Timestamp finish = NanoTimestamp() + timespan;

    // Try to acquire critical section at least one time
    if (TryLock())
        return true;
    else
    {
        // Try lock or yield for the given timespan
        while (NanoTimestamp() < finish)
        {
            if (TryLock())
                return true;
            else
                Thread::Yield();
        }

        // Failed to acquire critical section
        return false;
    }
}

void CriticalSection::Lock()
{
    _pimpl->Lock();
}

void CriticalSection::Unlock()
{
    _pimpl->Unlock();
}

} // namespace CppCommon