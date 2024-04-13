#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <combaseapi.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <Core/FixedString.hpp>
#include <Core/StringSlice.hpp>

namespace quinte::windows
{
    inline bool CheckHR(HRESULT hr)
    {
        QU_Assert(SUCCEEDED(hr));
        return SUCCEEDED(hr);
    }


    template<size_t TSize>
    struct WideString
    {
        WCHAR Data[TSize + 1];

        inline WideString(StringSlice str)
        {
            int count = MultiByteToWideChar(CP_UTF8, 0, str.Data(), static_cast<int>(str.Size()), nullptr, 0);
            QU_Assert(count >= 0 && count <= TSize);

            MultiByteToWideChar(CP_UTF8, 0, str.Data(), static_cast<int>(str.Size()), Data, count);
            Data[count] = 0;
        }
    };

    using WideStr = WideString<512>;
    using WidePath = WideString<MAX_PATH>;


    template<size_t TCapacity>
    inline void WideStringToUTF8(LPCWSTR wideString, FixedString<TCapacity>& utfString)
    {
        int count = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, nullptr, 0, nullptr, nullptr);
        QU_Assert(count >= 0 && count <= TCapacity);

        utfString.Resize(count, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wideString, -1, utfString.Data(), count, nullptr, nullptr);
    }


    inline void SetThreadProAudio()
    {
        using AvSetMmThreadCharacteristicsProc = HANDLE(__stdcall*)(LPCWSTR, LPDWORD);

        const HMODULE avrtModuleHandle = LoadLibraryW(L"AVRT.dll");
        if (!avrtModuleHandle)
            return;

        const auto avSetMmThreadCharacteristics =
            reinterpret_cast<AvSetMmThreadCharacteristicsProc>(GetProcAddress(avrtModuleHandle, "AvSetMmThreadCharacteristicsW"));

        DWORD taskIndex = 0;
        avSetMmThreadCharacteristics(L"Pro Audio", &taskIndex);
        FreeLibrary(avrtModuleHandle);
    }


    template<class T>
    class CoTaskMemPtr final : public NoCopyMove
    {
        T* m_Ptr;

    public:
        inline CoTaskMemPtr()
            : m_Ptr(nullptr)
        {
        }

        inline CoTaskMemPtr(T* ptr)
            : m_Ptr(ptr)
        {
        }

        inline ~CoTaskMemPtr()
        {
            if (m_Ptr)
                CoTaskMemFree(m_Ptr);
        }

        inline T* operator->() const noexcept
        {
            return m_Ptr;
        }

        inline T* Get() const noexcept
        {
            return m_Ptr;
        }

        inline T** GetAddressOf() noexcept
        {
            QU_Assert(m_Ptr == nullptr);
            return &m_Ptr;
        }

        inline explicit operator bool() const noexcept
        {
            return m_Ptr;
        }
    };


    class CoInitializeScope final : public NoCopyMove
    {
        bool m_Initialized;

    public:
        inline CoInitializeScope()
        {
            const HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            m_Initialized = !FAILED(hr);
        }

        ~CoInitializeScope()
        {
            if (!m_Initialized)
                return;

            CoUninitialize();
        }
    };


    class Event final : public NoCopy
    {
        HANDLE m_Handle = nullptr;

    public:
        inline Event() = default;

        inline Event(HANDLE handle) noexcept
            : m_Handle(handle)
        {
        }

        inline Event(Event&& other) noexcept
            : m_Handle(other.m_Handle)
        {
            other.m_Handle = nullptr;
        }

        inline Event& operator=(Event&& other) noexcept
        {
            Reset();
            m_Handle = other.m_Handle;
            other.m_Handle = nullptr;
            return *this;
        }

        inline ~Event()
        {
            Reset();
        }

        inline HANDLE GetHandle() const
        {
            return m_Handle;
        }

        inline operator bool() const
        {
            return m_Handle;
        }

        inline void Wait() const
        {
            if (m_Handle == nullptr)
                return;

            WaitForSingleObject(m_Handle, INFINITE);
        }

        inline void Reset()
        {
            if (m_Handle == nullptr)
                return;

            WaitForSingleObject(m_Handle, INFINITE);
            CloseHandle(m_Handle);
            m_Handle = nullptr;
        }

        inline static Event CreateAutoReset(StringSlice name, bool initialState = false)
        {
            // Copy the string to guarantee null-termination.
            const FixStr32 nameCopy{ name };
            return Event{ CreateEventA(nullptr, FALSE, initialState ? TRUE : FALSE, nameCopy.Data()) };
        }

        inline static Event CreateManualReset(StringSlice name, bool initialState = false)
        {
            const FixStr32 nameCopy{ name };
            return Event{ CreateEventA(nullptr, TRUE, initialState ? TRUE : FALSE, nameCopy.Data()) };
        }
    };
} // namespace quinte::windows
