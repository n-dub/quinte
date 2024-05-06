#pragma once
#include <Audio/Base.hpp>

namespace quinte
{
    namespace audio
    {
        struct PanValue final
        {
            enum class Pos
            {
                Center,
                Left,
                Right,
            };

            float Value;

            inline constexpr explicit PanValue(float value)
                : Value(value)
            {
            }

            inline constexpr explicit PanValue(Pos pos)
            {
                if (pos == Pos::Center)
                {
                    Value = 0.0f;
                    return;
                }

                Value = pos == Pos::Right ? 1.0f : -1.0f;
            }

            [[nodiscard]] inline constexpr Pos GetPos() const
            {
                if (Value == 0.0f)
                    return Pos::Center;

                return Value < 0 ? Pos::Left : Pos::Right;
            }

            [[nodiscard]] inline constexpr uint32_t GetLeftPercent() const
            {
                return static_cast<uint32_t>(Value * -100.0f);
            }

            [[nodiscard]] inline constexpr uint32_t GetRightPercent() const
            {
                return static_cast<uint32_t>(Value * -100.0f);
            }
        };


        struct GainValue final
        {
            float Amplitude;

            [[nodiscard]] inline constexpr float GetAmplitude() const
            {
                return Amplitude;
            }

            [[nodiscard]] inline constexpr float GetFader() const
            {
                return ConvertAmplitudeToFader(Amplitude);
            }

            [[nodiscard]] inline constexpr float GetDBFS() const
            {
                return ConvertAmplitudeToDBFS(Amplitude);
            }
        };
    } // namespace audio


    class Fader final
    {
        audio::DataType m_DataType;
        bool m_Muted = false;
        bool m_Soloed = false;
        std::atomic<audio::GainValue> m_Gain = audio::GainValue{ 1.0f };
        std::atomic<audio::PanValue> m_Pan = audio::PanValue{ audio::PanValue::Pos::Center };

    public:
        inline Fader(audio::DataType dataType)
            : m_DataType(dataType)
        {
        }

        [[nodiscard]] inline bool IsMuted() const
        {
            return m_Muted;
        }

        [[nodiscard]] inline bool IsSoloed() const
        {
            return m_Soloed;
        }

        [[nodiscard]] inline audio::GainValue GetGain() const
        {
            return m_Gain;
        }

        [[nodiscard]] inline audio::PanValue GetPan() const
        {
            return m_Pan;
        }

        inline void SetMute(bool value)
        {
            m_Muted = value;
        }

        inline void SetSolo(bool value)
        {
            m_Soloed = value;
        }

        inline void SetGain(audio::GainValue gain)
        {
            m_Gain = gain;
        }

        inline void SetPan(audio::PanValue pan)
        {
            m_Pan = pan;
        }
    };
} // namespace quinte
