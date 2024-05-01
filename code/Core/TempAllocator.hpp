#pragma once
#include <Core/Core.hpp>

namespace quinte::memory
{
    class TempAllocator final
    {
        struct Page final
        {
            Page* pNext = nullptr;
        };

    public:
        class Marker final
        {
            friend class TempAllocator;

            Page* m_pPage = nullptr;
            size_t m_Offset = sizeof(Page);
        };

        inline static constexpr size_t kPageByteSize = 16_MB;

    private:
        Page* m_pFirstPage = nullptr;
        Marker m_CurrentMarker;

        void NewPage();

    public:
        ~TempAllocator();

        static TempAllocator* GetForCurrentThread();

        void* Allocate(size_t byteSize, size_t byteAlignment);
        void Maintain();

        inline Marker GetMarker() const
        {
            return m_CurrentMarker;
        }

        inline void Restore(const Marker& marker)
        {
            m_CurrentMarker = marker;
        }
    };


    class TempAllocatorScope final : public std::pmr::memory_resource
    {
        struct DedicatedAllocation final
        {
            DedicatedAllocation* pNext = nullptr;
            size_t Size = 0;
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
            pointer = platform::Allocate(dedicatedSize);

            auto* pDedicated = static_cast<DedicatedAllocation*>(pointer);
            pDedicated->Size = dedicatedSize;
            pDedicated->pNext = m_pFirstDedicated;
            m_pFirstDedicated = pDedicated;

            return static_cast<uint8_t*>(pointer) + dedicatedOffset;
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
                platform::Deallocate(pOld, pOld->Size);
            }

            m_pAllocator->Restore(m_Marker);
        }
    };
} // namespace quinte::memory
