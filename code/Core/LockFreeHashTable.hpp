#pragma once
#include <Core/Core.hpp>

namespace quinte
{
    class LockFreeHashTable final : public NoCopy
    {
        struct Entry final
        {
            std::atomic<uint64_t> Key;
            std::atomic<void*> Value;
        };

        Entry* m_pEntries;
        size_t m_EntryCount;
        std::pmr::memory_resource* m_pAllocator;

    public:
        inline static constexpr size_t kEntrySize = sizeof(Entry);


        inline LockFreeHashTable(uint32_t maxEntries, std::pmr::memory_resource* pAllocator = nullptr)
            : m_EntryCount(maxEntries)
            , m_pAllocator(pAllocator)
        {
            QU_AssertDebug(maxEntries > 0);
            QU_AssertDebug((maxEntries & (maxEntries - 1)) == 0);

            if (m_pAllocator == nullptr)
                m_pAllocator = std::pmr::get_default_resource();

            m_pEntries = memory::Alloc<Entry>(m_pAllocator, maxEntries * sizeof(Entry));
            memory::Zero(m_pEntries, maxEntries);
        }

        inline LockFreeHashTable(LockFreeHashTable&& other)
            : m_pEntries(other.m_pEntries)
            , m_EntryCount(other.m_EntryCount)
            , m_pAllocator(other.m_pAllocator)
        {
            other.m_pEntries = nullptr;
        }

        inline LockFreeHashTable& operator=(LockFreeHashTable&& other)
        {
            if (m_pEntries)
                memory::SafeFree(m_pAllocator, m_pEntries, m_EntryCount * sizeof(Entry));

            m_pEntries = other.m_pEntries;
            m_EntryCount = other.m_EntryCount;
            m_pAllocator = other.m_pAllocator;
            other.m_pEntries = nullptr;
            return *this;
        }

        inline ~LockFreeHashTable()
        {
            if (m_pEntries)
                memory::SafeFree(m_pAllocator, m_pEntries, m_EntryCount * sizeof(Entry));
        }

        inline std::pair<bool, void*> Exchange(uint64_t key, void* value)
        {
            const uint64_t startIndex = DefaultHash(key) & (m_EntryCount - 1);

            // overflow detection
            bool firstLoop = true;
            for (uint64_t entryIndex = startIndex;; ++entryIndex)
            {
                entryIndex &= m_EntryCount - 1;
                if (entryIndex == startIndex && !firstLoop) [[unlikely]]
                    return { false, nullptr };
                firstLoop = false;

                Entry& entry = m_pEntries[entryIndex];
                const uint64_t probedKey = entry.Key.load(std::memory_order_relaxed);
                if (probedKey != key)
                {
                    if (probedKey != 0)
                        continue;

                    uint64_t expected = 0;
                    if (!entry.Key.compare_exchange_strong(expected, key)) [[unlikely]]
                        continue;
                }

                return { true, entry.Value.exchange(value, std::memory_order_relaxed) };
            }
        }

        inline void* Get(uint64_t key) const
        {
            const uint64_t startIndex = DefaultHash(key) & (m_EntryCount - 1);

            // overflow detection
            bool firstLoop = true;
            for (uint64_t entryIndex = startIndex;; ++entryIndex)
            {
                entryIndex &= m_EntryCount - 1;
                if (entryIndex == startIndex && !firstLoop) [[unlikely]]
                    return nullptr;

                const Entry& entry = m_pEntries[entryIndex];
                const uint64_t probedKey = entry.Key.load(std::memory_order_relaxed);
                if (probedKey == key)
                    return entry.Value;
                if (probedKey == 0)
                    return nullptr;
            }
        }
    };
} // namespace quinte
