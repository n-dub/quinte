#include <Core/TempAllocator.hpp>

namespace quinte::memory
{
    static thread_local TempAllocator g_TLSTempAllocator;


    void TempAllocator::NewPage()
    {
        Page* pCurrentPage = m_CurrentMarker.m_pPage;
        if (pCurrentPage == nullptr)
        {
            if (m_pFirstPage == nullptr)
                m_pFirstPage = static_cast<Page*>(platform::Allocate(kPageByteSize));

            m_CurrentMarker.m_pPage = m_pFirstPage;
            m_CurrentMarker.m_Offset = sizeof(Page);
            return;
        }

        if (pCurrentPage->pNext == nullptr)
        {
            Page* pNewPage = static_cast<Page*>(platform::Allocate(kPageByteSize));
            pCurrentPage->pNext = pNewPage;
        }

        m_CurrentMarker.m_pPage = pCurrentPage->pNext;
        m_CurrentMarker.m_Offset = sizeof(Page);
    }


    TempAllocator::~TempAllocator()
    {
        Page* pPage = m_pFirstPage;
        m_pFirstPage = nullptr;

        while (pPage)
        {
            Page* pOldPage = pPage;
            pPage = pPage->pNext;
            platform::Deallocate(pOldPage, kPageByteSize);
        }
    }


    TempAllocator* TempAllocator::GetForCurrentThread()
    {
        return &g_TLSTempAllocator;
    }


    void* TempAllocator::Allocate(size_t byteSize, size_t byteAlignment)
    {
        if (AlignUp(sizeof(Page), byteAlignment) + byteSize > kPageByteSize)
            return nullptr;

        if (!m_CurrentMarker.m_pPage)
            NewPage();

        const size_t newOffset = AlignUp(m_CurrentMarker.m_Offset, byteAlignment) + byteSize;
        if (newOffset > kPageByteSize)
            NewPage();

        const size_t oldOffset = m_CurrentMarker.m_Offset;
        m_CurrentMarker.m_Offset = newOffset;

        return reinterpret_cast<uint8_t*>(m_CurrentMarker.m_pPage) + oldOffset;
    }


    void TempAllocator::Maintain()
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
            platform::Deallocate(pOldPage, kPageByteSize);
        }
    }
} // namespace quinte::memory
