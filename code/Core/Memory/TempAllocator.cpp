#include <Core/Memory/TempAllocator.hpp>

namespace quinte::memory
{
    static thread_local TempAllocator g_TLSTempAllocator;


    TempAllocator* TempAllocator::GetForCurrentThread()
    {
        return &g_TLSTempAllocator;
    }
} // namespace quinte::memory
