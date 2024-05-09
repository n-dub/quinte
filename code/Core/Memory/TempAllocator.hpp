#pragma once
#include <Core/Core.hpp>
#include <Core/Memory/LinearAllocator.hpp>

namespace quinte::memory
{
    class TempAllocator final
    {
        LinearAllocator m_ImplAllocator;

    public:
        using Marker = LinearAllocator::Marker;

        inline static constexpr size_t kPageByteSize = 1024 * 1024;

        inline TempAllocator()
            : m_ImplAllocator(kPageByteSize, VirtualMemoryResource::Get())
        {
        }

        static TempAllocator* GetForCurrentThread();

        inline void* Allocate(size_t byteSize, size_t byteAlignment)
        {
            return m_ImplAllocator.allocate(byteSize, byteAlignment);
        }

        inline void Maintain()
        {
            m_ImplAllocator.Maintain();
        }

        inline Marker GetMarker() const
        {
            return m_ImplAllocator.GetMarker();
        }

        inline void Restore(const Marker& marker)
        {
            m_ImplAllocator.Restore(marker);
        }
    };


    class TempAllocatorScope final : public std::pmr::memory_resource
    {
        struct DedicatedAllocation final
        {
            DedicatedAllocation* pNext = nullptr;
        };

        TempAllocator::Marker m_Marker;
        TempAllocator* m_pAllocator = nullptr;
        DedicatedAllocation* m_pFirstDedicated = nullptr;

        inline void* do_allocate(size_t byteSize, size_t byteAlignment) override
        {
            void* pointer = m_pAllocator->Allocate(byteSize, byteAlignment);
            if (pointer)
                return pointer;

            const size_t dedicatedOffset = AlignUp(sizeof(DedicatedAllocation), byteAlignment);
            const size_t dedicatedSize = dedicatedOffset + byteSize;

            auto* pDedicated = memory::DefaultAlloc<DedicatedAllocation>(dedicatedSize, byteAlignment);
            pDedicated->pNext = m_pFirstDedicated;
            m_pFirstDedicated = pDedicated;

            return reinterpret_cast<uint8_t*>(pDedicated) + dedicatedOffset;
        }

        inline void do_deallocate(void*, size_t, size_t) override {}

        inline bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
            return this == &other;
        }

    public:
        inline TempAllocatorScope()
            : m_pAllocator(TempAllocator::GetForCurrentThread())
        {
            m_Marker = m_pAllocator->GetMarker();
        }

        inline ~TempAllocatorScope() override
        {
            while (m_pFirstDedicated)
            {
                DedicatedAllocation* pOld = m_pFirstDedicated;
                m_pFirstDedicated = m_pFirstDedicated->pNext;
                memory::DefaultFree(pOld);
            }

            m_pAllocator->Restore(m_Marker);
        }
    };
} // namespace quinte::memory
