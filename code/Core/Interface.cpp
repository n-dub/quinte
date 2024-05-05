#include <Core/Interface.hpp>
#include <Core/LockFreeHashTable.hpp>

namespace quinte::detail
{
    namespace
    {
        inline constexpr size_t kRegistrySize = memory::platform::kVirtualAllocationGranularity / LockFreeHashTable::kEntrySize;

        struct InterfaceRegistryImpl final
        {
            LockFreeHashTable m_Instances{ kRegistrySize, memory::VirtualMemoryResource::Get() };


            inline void RegisterInstance(uint64_t typeHash, void* pInstance)
            {
                const auto [success, prevValue] = m_Instances.Exchange(typeHash, pInstance);
                QU_AssertDebug(success && prevValue == nullptr);
            }

            inline void UnregisterInstance(uint64_t typeHash, void* pInstance)
            {
                const auto [success, prevValue] = m_Instances.Exchange(typeHash, nullptr);
                QU_AssertDebug(success && prevValue == pInstance);
            }

            inline void* FindInstance(uint64_t typeHash) const
            {
                return m_Instances.Get(typeHash);
            }
        };
    } // namespace

    static InterfaceRegistryImpl g_InterfaceRegistry;


    void RegisterInstance(uint64_t typeHash, void* pInstance)
    {
        g_InterfaceRegistry.RegisterInstance(typeHash, pInstance);
    }


    void UnregisterInstance(uint64_t typeHash, void* pInstance)
    {
        g_InterfaceRegistry.UnregisterInstance(typeHash, pInstance);
    }


    void* FindInstance(uint64_t typeHash)
    {
        return g_InterfaceRegistry.FindInstance(typeHash);
    }
} // namespace quinte::detail
