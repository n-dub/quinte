#include <Core/Memory/MemoryPool.hpp>

namespace quinte
{
    void* MemoryPool::do_allocate(size_t byteSize, size_t byteAlignment)
    {
        QU_AssertDebug(byteSize <= m_ElementByteSize);
        QU_AssertDebug(byteAlignment <= memory::kDefaultAlignment);

#if QU_DEBUG
        ++m_AllocationCount;
#endif

        void* pResult;
        if (!m_pFreeList)
        {
            pResult = AllocateFromPage(m_pPageList);
            if (!pResult) [[unlikely]]
            {
                Page* pPage = AllocatePage();
                pResult = AllocateFromPage(pPage);
            }

            return pResult;
        }

        pResult = m_pFreeList;
        m_pFreeList = *static_cast<void**>(pResult);
        return pResult;
    }


    void MemoryPool::do_deallocate(void* ptr, size_t, size_t)
    {
        QU_AssertDebug(m_AllocationCount-- > 0);
        *static_cast<void**>(ptr) = m_pFreeList;
        m_pFreeList = ptr;
    }


    bool MemoryPool::do_is_equal(const memory_resource& other) const noexcept
    {
        return this == &other;
    }
} // namespace quinte
