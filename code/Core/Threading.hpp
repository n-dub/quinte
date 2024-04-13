#pragma once
#include <Core/Core.hpp>
#include <Core/StringSlice.hpp>

namespace quinte::threading
{
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
