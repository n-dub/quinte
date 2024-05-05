#include <Core/Interface.hpp>
#include <Core/Threading.hpp>
#include <unordered_map>

namespace quinte::detail
{
    namespace
    {
        struct InterfaceRegistryImpl final
        {
            threading::SpinLock m_Mutex;
            std::pmr::unordered_map<uint64_t, void*> m_Instances;


            inline void RegisterInstance(uint64_t typeHash, void* pInstance)
            {
                const std::lock_guard lock{ m_Mutex };
                QU_AssertDebug(m_Instances[typeHash] == nullptr);
                m_Instances[typeHash] = pInstance;
            }

            inline void UnregisterInstance(uint64_t typeHash, void* pInstance)
            {
                const std::lock_guard lock{ m_Mutex };
                QU_AssertDebug(m_Instances[typeHash] == pInstance);
                m_Instances[typeHash] = nullptr;
            }

            inline void* FindInstance(uint64_t typeHash)
            {
                const std::lock_guard lock{ m_Mutex };
                return m_Instances[typeHash];
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
