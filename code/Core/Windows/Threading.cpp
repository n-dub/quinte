#include <Core/Threading.hpp>
#include <Core/Windows/Utils.hpp>
#include <Core/MemoryPool.hpp>

namespace quinte::threading
{
    namespace
    {
        struct ThreadDataImpl final
        {
            DWORD ID;
            HANDLE hThread;
            void* pUserData;
            ThreadFunction StartRoutine;
        };

        MemoryPool g_ThreadDataPool{ sizeof(ThreadDataImpl), 64 };
        SpinLock g_ThreadDataLock;


        inline ThreadDataImpl* AllocateThreadData()
        {
            const std::lock_guard lock{ g_ThreadDataLock };
            return memory::New<ThreadDataImpl>(&g_ThreadDataPool);
        }


        struct ThreadDataHolder final
        {
            ThreadDataImpl* pData = nullptr;

            ~ThreadDataHolder()
            {
                if (pData)
                {
                    const std::lock_guard lock{ g_ThreadDataLock };
                    memory::Delete(&g_ThreadDataPool, pData, 0);
                }
            }
        };

        thread_local ThreadDataHolder g_TLSThreadData;


        static DWORD WINAPI ThreadRoutineImpl(LPVOID lpParam)
        {
            auto* pData = static_cast<ThreadDataImpl*>(lpParam);
            g_TLSThreadData.pData = pData;
            pData->StartRoutine(pData->pUserData);
            return 0;
        }
    } // namespace


    EventHandle CreateAutoResetEvent(StringSlice name, bool initialState)
    {
        QU_Unused(name);
        return EventHandle{ reinterpret_cast<uint64_t>(CreateEventW(nullptr, FALSE, initialState ? TRUE : FALSE, nullptr)) };
    }


    EventHandle CreateManualResetEvent(StringSlice name, bool initialState)
    {
        QU_Unused(name);
        return EventHandle{ reinterpret_cast<uint64_t>(CreateEventW(nullptr, TRUE, initialState ? TRUE : FALSE, nullptr)) };
    }


    void WaitEvent(EventHandle event)
    {
        WaitForSingleObject(reinterpret_cast<HANDLE>(event.Value), INFINITE);
    }


    void CloseEvent(EventHandle& event)
    {
        CloseHandle(reinterpret_cast<HANDLE>(event.Value));
        event.Reset();
    }


    void* GetNativeEventHandle(EventHandle event)
    {
        return reinterpret_cast<void*>(event.Value);
    }


    ThreadHandle CreateThread(StringSlice name, ThreadFunction startRoutine, void* pUserData, Priority priority, size_t stackSize)
    {
        ThreadDataImpl* pData = AllocateThreadData();

        const HANDLE hThread = ::CreateThread(nullptr, stackSize, &ThreadRoutineImpl, pData, CREATE_SUSPENDED, &pData->ID);
        QU_Assert(hThread);

        pData->hThread = hThread;
        pData->pUserData = pUserData;
        pData->StartRoutine = startRoutine;

        windows::WideString<64> wideDesc{ name };
        const HRESULT hrDesc = SetThreadDescription(hThread, wideDesc.Data);
        windows::CheckHR(hrDesc);

        if (priority != Priority::Normal)
        {
            const BOOL priorityRes = SetThreadPriority(hThread, static_cast<int>(priority));
            QU_Assert(priorityRes);
        }

        const DWORD resumeRes = ResumeThread(hThread);
        QU_Assert(resumeRes != static_cast<DWORD>(-1));

        return ThreadHandle{ reinterpret_cast<uint64_t>(pData) };
    }


    void CloseThread(ThreadHandle& thread)
    {
        if (thread.Value == 0)
            return;

        const ThreadDataImpl data = *reinterpret_cast<ThreadDataImpl*>(thread.Value);
        const DWORD waitRes = WaitForSingleObject(data.hThread, INFINITE);
        QU_Assert(waitRes == WAIT_OBJECT_0);

        const BOOL closeRes = CloseHandle(data.hThread);
        QU_Assert(closeRes);
        thread.Reset();
    }
} // namespace quinte::threading
