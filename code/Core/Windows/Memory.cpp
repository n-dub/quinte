#include <Core/Memory.hpp>
#include <Core/Windows/Utils.hpp>

namespace quinte::memory::platform
{
    void* Allocate(size_t byteSize)
    {
        QU_Assert(byteSize >= kVirtualAllocationGranularity);
        QU_Assert(byteSize % kVirtualAllocationGranularity == 0);
        return VirtualAlloc(nullptr, byteSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    }


    void Deallocate(void* pointer, size_t byteSize)
    {
        // Size is required on some platforms, but not on Windows with MEM_RELEASE.
        QU_Unused(byteSize);

        const BOOL result = VirtualFree(pointer, 0, MEM_RELEASE);
        QU_Assert(result);
    }
} // namespace quinte::memory::platform
