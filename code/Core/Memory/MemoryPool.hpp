#pragma once
#include <Core/Core.hpp>

namespace quinte
{
    class MemoryPool final : public std::pmr::memory_resource
    {
        struct Page final
        {
            Page* pNext;
            void* pCurrent;
        };

        static_assert(sizeof(Page) == memory::kDefaultAlignment);

        size_t m_ElementByteSize = 0;
        size_t m_PageByteSize = 0;

        Page* m_pPageList = nullptr;
        void* m_pFreeList = nullptr;

        std::pmr::memory_resource* m_pPageAllocator = nullptr;

#if QU_DEBUG
        // Used to ensure we don't destroy or reinitialize pools that are still in use.
        uint32_t m_AllocationCount = 0;
#endif

        inline Page* AllocatePage()
        {
            Page* pResult = static_cast<Page*>(m_pPageAllocator->allocate(m_PageByteSize));
            pResult->pNext = m_pPageList;
            pResult->pCurrent = pResult + 1;
            m_pPageList = pResult;
            return pResult;
        }

        inline void* AllocateFromPage(Page* pPage) const
        {
            if (!pPage) [[unlikely]]
                return nullptr;

            const uint8_t* pPageEnd = reinterpret_cast<const uint8_t*>(pPage) + m_PageByteSize;
            const size_t elementByteSize = m_ElementByteSize;

            uint8_t* pResult = static_cast<uint8_t*>(pPage->pCurrent);
            if (pResult + elementByteSize > pPageEnd) [[unlikely]]
                return nullptr;

            pPage->pCurrent = pResult + elementByteSize;
            return pResult;
        }

        inline void FreePages()
        {
#if QU_DEBUG
            QU_AssertDebug(m_AllocationCount == 0);
#endif

            Page* pPage = m_pPageList;
            while (pPage)
            {
                Page* pNext = pPage->pNext;
                m_pPageAllocator->deallocate(pPage, m_PageByteSize);
                pPage = pNext;
            }
        }

    protected:
        void* do_allocate(size_t byteSize, size_t byteAlignment) override;
        void do_deallocate(void* ptr, size_t, size_t) override;
        bool do_is_equal(const memory_resource& other) const noexcept override;

    public:
        inline MemoryPool() = default;

        inline MemoryPool(size_t elementByteSize, uint32_t pageElementCount, std::pmr::memory_resource* pPageAllocator = nullptr)
        {
            Initialize(elementByteSize, pageElementCount, pPageAllocator);
        }

        inline ~MemoryPool() override
        {
            FreePages();
        }

        inline void Initialize(size_t elementByteSize, uint32_t pageElementCount,
                               std::pmr::memory_resource* pPageAllocator = nullptr)
        {
            QU_AssertDebugMsg(m_ElementByteSize == 0, "Pool already initialized");
            QU_AssertDebug(elementByteSize > 0);
            QU_AssertDebug(pageElementCount > 0);

            m_ElementByteSize = AlignUp<memory::kDefaultAlignment>(elementByteSize);
            m_PageByteSize = pageElementCount * m_ElementByteSize + sizeof(Page);
            m_pPageAllocator = pPageAllocator;

            if (m_pPageAllocator == nullptr)
                m_pPageAllocator = std::pmr::get_default_resource();
        }

        inline void Deinitialize()
        {
            FreePages();
            m_ElementByteSize = 0;
            m_PageByteSize = 0;
            m_pPageList = nullptr;
            m_pFreeList = nullptr;
            m_pPageAllocator = nullptr;
        }

        inline void Reinitialize(size_t elementByteSize, uint32_t pageElementCount,
                                 std::pmr::memory_resource* pPageAllocator = nullptr)
        {
            Deinitialize();
            Initialize(elementByteSize, pageElementCount, pPageAllocator);
        }
    };
} // namespace quinte
