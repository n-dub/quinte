#pragma once
#include <Audio/Base.hpp>
#include <Audio/Tracks/AudioClip.hpp>
#include <Audio/Tracks/Fader.hpp>
#include <Audio/Tracks/Playlist.hpp>
#include <Core/String.hpp>

namespace quinte
{
    class Track final : public memory::RefCountedObjectBase
    {
        Playlist m_Playlist;
        [[maybe_unused]] audio::DataType m_InputDataType;
        [[maybe_unused]] audio::DataType m_OutputDataType;
        bool m_Monitored = false;
        bool m_RecordArmed = false;
        Fader m_Fader;
        String m_Name;

    public:
        inline Track(audio::DataType inputDataType, audio::DataType outputDataType)
            : m_InputDataType(inputDataType)
            , m_OutputDataType(outputDataType)
            , m_Fader(outputDataType)
        {
        }

        [[nodiscard]] inline Playlist& GetPlaylist()
        {
            return m_Playlist;
        }

        [[nodiscard]] inline bool IsMonitored() const
        {
            return m_Monitored;
        }

        [[nodiscard]] inline bool IsRecordArmed() const
        {
            return m_RecordArmed;
        }

        [[nodiscard]] inline StringSlice GetName() const
        {
            return m_Name;
        }

        [[nodiscard]] inline Fader* GetFader()
        {
            return &m_Fader;
        }

        inline void SetMonitor(bool value)
        {
            m_Monitored = value;
        }

        inline void SetRecordArm(bool value)
        {
            m_RecordArmed = value;
        }

        inline void SetName(StringSlice name)
        {
            m_Name = name;
        }
    };
} // namespace quinte
