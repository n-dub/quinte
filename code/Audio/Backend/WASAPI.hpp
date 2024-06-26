﻿#pragma once
#include <Audio/Backend/BackendBase.hpp>
#include <Core/String.hpp>
#include <Core/Platform/Windows/Utils.hpp>
#include <array>

#ifndef INITGUID
#    define INITGUID
#endif

#include <mfapi.h>
#include <mferror.h>
#include <mfplay.h>
#include <mftransform.h>
#include <wmcodecdsp.h>

#include <audioclient.h>
#include <avrt.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>

namespace quinte
{
    class AudioBackendWASAPI final : public AudioBackendBase
    {
        using BackendDeviceID = FixStr512;

        struct Resampler final
        {
            audio::Format Format;
            uint32_t ChannelCount;
            uint32_t InSampleRate;
            uint32_t OutSampleRate;

            Rc<IUnknown> pTransformUnk;
            Rc<IMFTransform> pTransform;

            Rc<IMFMediaType> pMediaType;
            Rc<IMFMediaType> pInMediaType;
            Rc<IMFMediaType> pOutMediaType;

            Rc<IWMResamplerProps> pResamplerProps;

            Resampler(audio::Format format, uint32_t channelCount, uint32_t inSampleRate, uint32_t outSampleRate);
            ~Resampler();

            uint32_t Convert(uint8_t* pOutBuffer, const uint8_t* pInBuffer, uint32_t inSampleCount,
                             uint32_t maxOutSampleCount = std::numeric_limits<uint32_t>::max());
        };

        struct DeviceDataFlowInfo final
        {
            BackendDeviceID ID;
            EDataFlow DataFlow = eAll;
            bool IsDefault = false;
        };

        struct ClientHandle
        {
            Rc<IAudioClient> AudioClient;
            threading::Event Event;
        };

        struct : ClientHandle
        {
            Rc<IAudioCaptureClient> CaptureClient;
        } m_CaptureHandle;

        struct : ClientHandle
        {
            Rc<IAudioRenderClient> RenderClient;
        } m_RenderHandle;

        windows::CoInitializeScope m_CoInitScope;

        Rc<IMMDeviceEnumerator> m_DeviceEnumerator;
        std::pmr::vector<DeviceDataFlowInfo> m_BackendDevices;

        audio::ResultCode EnumerateDevicesImpl(EDataFlow dataFlow);
        bool GetDeviceInfoImpl(LPCWSTR deviceID, EDataFlow dataFlow, audio::DeviceDesc& deviceDesc);

        static void RunThread(void* pUserData);
        void RunThreadImpl();

    protected:
        audio::ResultCode OpenStreamImpl(const BackendStreamOpenInfo& openInfo, uint32_t& bufferFrameCount) override;

    public:
        AudioBackendWASAPI();
        ~AudioBackendWASAPI();

        void CloseStream() override;

        audio::APIKind GetKind() const override;
        audio::ResultCode UpdateDeviceList() override;
        audio::DeviceID GetDefautInputDevice() const override;
        audio::DeviceID GetDefaultOutputDevice() const override;
        audio::ResultCode StartStream() override;
        audio::ResultCode StopStream() override;
        audio::ResultCode AbortStream() override;
    };
} // namespace quinte
