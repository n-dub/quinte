#pragma once
#include <Core/Core.hpp>
#include <Core/StringSlice.hpp>

#ifdef CreateMutex
#    undef CreateMutex
#endif

namespace quinte::threading
{
    class SpinLock final
    {
        std::atomic<bool> m_Locked;

    public:
        inline void lock() noexcept
        {
            if (!m_Locked.exchange(true, std::memory_order_acquire))
                return;

            _mm_pause();

            while (true)
            {
                if (!m_Locked.exchange(true, std::memory_order_acquire))
                    return;

                uint32_t spinCount = 1;
                while (m_Locked.load(std::memory_order_relaxed))
                {
                    for (uint32_t i = 0; i < spinCount; ++i)
                        _mm_pause();

                    spinCount = Min(spinCount << 1, 32u);
                }
            }
        }

        inline bool try_lock() noexcept
        {
            return !m_Locked.load(std::memory_order_relaxed) && !m_Locked.exchange(true);
        }

        inline void unlock() noexcept
        {
            m_Locked.store(false, std::memory_order_release);
        }
    };


    struct MutexHandle final : TypedHandle<MutexHandle, uint64_t, 0>
    {
    };


    MutexHandle CreateMutex(uint32_t spinCount = 500);
    void LockMutex(MutexHandle mutex);
    bool TryLockMutex(MutexHandle mutex);
    void UnlockMutex(MutexHandle mutex);
    void CloseMutex(MutexHandle& mutex);


    class Mutex final : public NoCopy
    {
        MutexHandle m_Handle;

    public:
        inline Mutex()
        {
            m_Handle = CreateMutex();
        }

        inline Mutex(uint32_t spinCount)
        {
            m_Handle = CreateMutex(spinCount);
        }

        inline Mutex(Mutex&& other) noexcept
            : m_Handle(other.m_Handle)
        {
            other.m_Handle.Reset();
        }

        inline Mutex& operator=(Mutex&& other) noexcept
        {
            CloseMutex(m_Handle);
            m_Handle = other.m_Handle;
            other.m_Handle.Reset();
            return *this;
        }

        inline ~Mutex()
        {
            CloseMutex(m_Handle);
        }

        inline void lock()
        {
            LockMutex(m_Handle);
        }

        inline bool try_lock()
        {
            return TryLockMutex(m_Handle);
        }

        inline void unlock()
        {
            UnlockMutex(m_Handle);
        }
    };


    struct EventHandle final : TypedHandle<EventHandle, uint64_t, 0>
    {
    };


    EventHandle CreateAutoResetEvent(StringSlice name, bool initialState = false);
    EventHandle CreateManualResetEvent(StringSlice name, bool initialState = false);
    void WaitEvent(EventHandle event);
    void CloseEvent(EventHandle& event);
    void* GetNativeEventHandle(EventHandle event);


    class Event final : public NoCopy
    {
        EventHandle m_Handle;

    public:
        inline Event() = default;

        inline Event(EventHandle handle) noexcept
            : m_Handle(handle)
        {
        }

        inline Event(Event&& other) noexcept
            : m_Handle(other.m_Handle)
        {
            other.m_Handle.Reset();
        }

        inline Event& operator=(Event&& other) noexcept
        {
            Reset();
            m_Handle = other.m_Handle;
            other.m_Handle.Reset();
            return *this;
        }

        inline ~Event()
        {
            Reset();
        }

        inline void* GetNativeHandle() const
        {
            return GetNativeEventHandle(m_Handle);
        }

        inline operator bool() const
        {
            return static_cast<bool>(m_Handle);
        }

        inline void Wait() const
        {
            if (!m_Handle)
                return;

            WaitEvent(m_Handle);
        }

        inline void Reset()
        {
            if (!m_Handle)
                return;

            WaitEvent(m_Handle);
            CloseEvent(m_Handle);
        }

        inline static Event CreateAutoReset(StringSlice name, bool initialState = false)
        {
            return Event{ CreateAutoResetEvent(name, initialState) };
        }

        inline static Event CreateManualReset(StringSlice name, bool initialState = false)
        {
            return Event{ CreateManualResetEvent(name, initialState) };
        }
    };


    typedef void (*ThreadFunction)(void*);


    enum class Priority : int32_t
    {
        Lowest = -2,
        BelowNormal = -1,
        Normal = 0,
        AboveNormal = 1,
        Highest = 2,
    };


    struct ThreadHandle final : TypedHandle<ThreadHandle, uint64_t, 0>
    {
    };


    ThreadHandle CreateThread(StringSlice name, ThreadFunction startRoutine, void* pUserData = nullptr,
                              Priority priority = Priority::Normal, size_t stackSize = 0);
    void CloseThread(ThreadHandle& thread);


    class Thread final : public NoCopy
    {
        ThreadHandle m_Handle;

    public:
        inline Thread(StringSlice name, ThreadFunction startRoutine, void* pUserData = nullptr,
                      Priority priority = Priority::Normal, size_t stackSize = 0)
        {
            m_Handle = CreateThread(name, startRoutine, pUserData, priority, stackSize);
        }

        inline Thread(Thread&& other) noexcept
            : m_Handle(other.m_Handle)
        {
            other.m_Handle.Reset();
        }

        inline Thread& operator=(Thread&& other) noexcept
        {
            CloseThread(m_Handle);
            m_Handle = other.m_Handle;
            other.m_Handle.Reset();
            return *this;
        }

        inline ~Thread()
        {
            Join();
        }

        inline void Join()
        {
            CloseThread(m_Handle);
        }
    };
} // namespace quinte::threading
