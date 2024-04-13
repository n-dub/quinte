#pragma once
#include <Audio/Engine.hpp>
#include <Core/FixedVector.hpp>
#include <Core/Threading.hpp>
#include <array>
#include <mutex>

namespace quinte
{
    enum class BackendStreamMode : uint32_t
    {
        Output,
        Input,
        Duplex,
        None,
    };


    struct BackendStreamCallbackInfo final
    {
        audio::Callback Callback;
        threading::ThreadHandle Thread;
        void* pUserData;
    };


    struct BackendConversionInfo final
    {
        uint32_t ChannelCount;
        bool IsNeeded;

        struct
        {
            uint32_t Jump;
            audio::Format Format;
            FixedVector<int32_t, 2> Offset;
        } In, Out;
    };


    struct BackendStreamData final
    {
        std::mutex Mutex;
        BackendStreamCallbackInfo CallbackInfo = { 0 };
        std::atomic<audio::StreamState> State = audio::StreamState::Closed;
        BackendStreamMode Mode = BackendStreamMode::None;
        uint32_t SampleRate = 0;
        uint32_t BufferFrameCount = 0;
        uint32_t BufferCount = 0;
        std::array<audio::DeviceID, 2> DeviceID;
        std::array<uint32_t, 2> FirstChannelIndex;
        std::array<uint32_t, 2> Latency;
        std::array<BackendConversionInfo, 2> ConversionInfo;
        memory::unique_ptr<uint8_t[]> DeviceBuffer;
        std::array<memory::unique_ptr<uint8_t[]>, 2> UserBuffer;

        struct
        {
            audio::Format Format = audio::Format::None;
            std::array<uint32_t, 2> ChannelCount;
        } User;

        struct
        {
            std::array<audio::Format, 2> Format;
            std::array<uint32_t, 2> ChannelCount;
            std::array<bool, 2> Interleaved = { true, true };
        } Device;
    };


    struct BackendStreamOpenInfo final
    {
        audio::DeviceID DeviceID;
        BackendStreamMode Mode;
        uint32_t ChannelCount;
        uint32_t FirstChannelIndex;
        uint32_t SampleRate;
        audio::Format Format;
    };


    class AudioBackendBase : public IAudioAPI
    {
        mutable std::atomic<audio::ResultCode> m_LastResult = audio::ResultCode::Success;

    protected:
        std::pmr::vector<audio::DeviceDesc> m_Devices;
        BackendStreamData m_StreamData;

        audio::DeviceID::BaseType m_CurrentDeviceID = 0;
        double m_StreamTime = 0.0;

        inline audio::ResultCode Ret(audio::ResultCode code) const noexcept
        {
            m_LastResult.store(code, std::memory_order_release);
            return code;
        }

        inline virtual void TickStreamTime()
        {
            m_StreamTime += static_cast<double>(m_StreamData.BufferFrameCount) / m_StreamData.SampleRate;
        }

        inline virtual double GetStreamTime() const
        {
            return m_StreamTime;
        }

        void ClearStreamData();
        void InitializeConversionInfo(BackendStreamMode mode, uint32_t firstChannelIndex);
        void ConvertBuffer(uint8_t* pOutBuffer, const uint8_t* pInBuffer, const BackendConversionInfo& info);

        virtual audio::ResultCode OpenStreamImpl(const BackendStreamOpenInfo& openInfo, uint32_t& bufferFrameCount) = 0;

    public:
        inline audio::ResultCode GetLastResultCode() const
        {
            return m_LastResult.load(std::memory_order_acquire);
        }

        audio::ResultCode OpenStream(audio::StreamOpenInfo& openInfo) override;
        std::span<const audio::DeviceDesc> GetDevices() const override;
        audio::StreamState GetState() const override;
    };
} // namespace quinte
