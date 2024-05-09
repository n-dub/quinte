#pragma once
#include <Audio/Base.hpp>
#include <Audio/Sources/AudioSource.hpp>
#include <Core/String.hpp>

namespace quinte
{
    class AudioClip final
    {
        String m_Name;
        Rc<AudioSource> m_pSource;
        audio::TimePos64 m_Position;
        audio::TimeRange32 m_SourceRange;

        //
        // m_SourceRange is only to allow trimming:
        //
        // | Timeline                                  |
        // |            | Source        |              |
        // |                | Clip   |                 |
        //                  ^        ^  ^
        //                  a        b  c
        //
        // a = m_Position;
        // b = m_Position + m_SourceRange.Length;
        // c = m_Position - m_SourceRange.StartPos + m_pSource->Length();
        //

    public:
        inline AudioClip(StringSlice name, AudioSource* pSource, audio::TimePos64 firstSamplePosition)
            : m_Name(name)
            , m_pSource(pSource)
            , m_Position(firstSamplePosition)
            , m_SourceRange(0, pSource->GetLength())
        {
        }

        inline AudioClip(AudioSource* pSource, audio::TimePos64 firstSamplePosition)
            : m_pSource(pSource)
            , m_Position(firstSamplePosition)
            , m_SourceRange(0, pSource->GetLength())
        {
        }

        [[nodiscard]] inline uint64_t Read(float* pDestination, audio::TimeRange64 range) const
        {
            QU_AssertDebug(range.GetFirstSampleIndex() >= m_Position.GetSampleIndex());
            const uint64_t offset = range.GetFirstSampleIndex() - m_Position.GetSampleIndex();
            const uint64_t sourceOffset = m_SourceRange.GetFirstSampleIndex() + offset;
            return m_pSource->Read(pDestination, sourceOffset, range.GetLengthInSamples());
        }

        [[nodiscard]] inline StringSlice GetName() const
        {
            return m_Name;
        }

        //! \brief Position of the clip on the session timeline.
        [[nodiscard]] inline audio::TimePos64 GetPosition() const
        {
            return m_Position;
        }

        //! \brief Position of the end of the clip on the session timeline.
        [[nodiscard]] inline audio::TimePos64 GetEndPosition() const
        {
            return m_Position.SampleIndex + m_SourceRange.Length;
        }

        //! \brief Subrange of the source used by the clip.
        [[nodiscard]] inline audio::TimeRange32 GetSourceRange() const
        {
            return m_SourceRange;
        }

        [[nodiscard]] inline audio::DataType GetDataType() const
        {
            return m_pSource->GetDataType();
        }
    };
} // namespace quinte
