#include <Core/Core.hpp>
#include <Core/Memory/LinearAllocator.hpp>

namespace quinte::memory
{
    void LinearAllocator::NewPage()
    {
        Page* pCurrentPage = m_CurrentMarker.m_pPage;
        if (pCurrentPage == nullptr)
        {
            if (m_pFirstPage == nullptr)
                m_pFirstPage = Alloc<Page>(m_pPageAllocator, m_PageByteSize);

            m_CurrentMarker.m_pPage = m_pFirstPage;
            m_CurrentMarker.m_Offset = sizeof(Page);
            return;
        }

        if (pCurrentPage->pNext == nullptr)
        {
            Page* pNewPage = Alloc<Page>(m_pPageAllocator, m_PageByteSize);
            pCurrentPage->pNext = pNewPage;
        }

        m_CurrentMarker.m_pPage = pCurrentPage->pNext;
        m_CurrentMarker.m_Offset = sizeof(Page);
    }


    LinearAllocator::LinearAllocator(size_t pageByteSize, std::pmr::memory_resource* pPageAllocator)
        : m_PageByteSize(pageByteSize)
        , m_pPageAllocator(pPageAllocator)
    {
        QU_AssertDebug(pageByteSize > 0);
        if (m_pPageAllocator == nullptr)
        {
            if (pageByteSize >= platform::kVirtualAllocationGranularity
                && (pageByteSize % platform::kVirtualAllocationGranularity == 0))
            {
                m_pPageAllocator = VirtualMemoryResource::Get();
            }
            else
            {
                m_pPageAllocator = std::pmr::get_default_resource();
            }
        }
    }


    LinearAllocator::~LinearAllocator()
    {
        Page* pPage = m_pFirstPage;
        m_pFirstPage = nullptr;

        while (pPage)
        {
            Page* pOldPage = pPage;
            pPage = pPage->pNext;
            m_pPageAllocator->deallocate(pOldPage, m_PageByteSize);
        }
    }


    void* LinearAllocator::do_allocate(size_t byteSize, size_t byteAlignment)
    {
        if (AlignUp(sizeof(Page), byteAlignment) + byteSize > m_PageByteSize) [[unlikely]]
            return nullptr;

        if (!m_CurrentMarker.m_pPage) [[unlikely]]
            NewPage();

        const size_t newOffset = AlignUp(m_CurrentMarker.m_Offset, byteAlignment) + byteSize;
        if (newOffset > m_PageByteSize) [[unlikely]]
            NewPage();

        const size_t oldOffset = m_CurrentMarker.m_Offset;
        m_CurrentMarker.m_Offset = newOffset;

        return reinterpret_cast<uint8_t*>(m_CurrentMarker.m_pPage) + oldOffset;
    }


    void LinearAllocator::Maintain()
    {
        Page* pPage = m_CurrentMarker.m_pPage;
        if (pPage == nullptr)
            return;

        pPage = pPage->pNext;
        m_CurrentMarker.m_pPage->pNext = nullptr;

        while (pPage)
        {
            Page* pOldPage = pPage;
            pPage = pPage->pNext;
            m_pPageAllocator->deallocate(pOldPage, m_PageByteSize);
        }
    }
} // namespace quinte::memory
