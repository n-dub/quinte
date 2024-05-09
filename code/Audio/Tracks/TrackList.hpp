#pragma once
#include <Audio/Tracks/Track.hpp>

namespace quinte
{
    struct TrackInfo final
    {
        Rc<Track> pTrack;
        uint32_t ID;
        uint32_t Color;
    };


    class TrackList final
    {
        std::pmr::vector<TrackInfo> m_Tracks;
        std::pmr::vector<uint32_t> m_TracksOrder;
        uint32_t m_CurrentTrackID = 0;

    public:
        class Iterator final
        {
            friend class TrackList;

            TrackList* m_pTrackList;
            size_t m_Index;

            inline Iterator(TrackList* pTrackList, size_t index)
                : m_pTrackList(pTrackList)
                , m_Index(index)
            {
            }

        public:
            inline TrackInfo& operator*()
            {
                return m_pTrackList->m_Tracks[m_pTrackList->m_TracksOrder[m_Index]];
            }

            inline TrackInfo* operator->()
            {
                return &m_pTrackList->m_Tracks[m_pTrackList->m_TracksOrder[m_Index]];
            }

            inline Iterator& operator++()
            {
                ++m_Index;
                return *this;
            }

            inline Iterator operator++(int)
            {
                const Iterator temp = *this;
                ++m_Index;
                return temp;
            }

            inline Iterator& operator--()
            {
                --m_Index;
                return *this;
            }

            inline Iterator operator--(int)
            {
                const Iterator temp = *this;
                --m_Index;
                return temp;
            }

            inline bool operator==(const Iterator& other) const = default;
        };

        inline TrackInfo& operator[](size_t index)
        {
            return m_Tracks[m_TracksOrder[index]];
        }

        inline void AddTrack(Track* pTrack, uint32_t trackColor)
        {
            const TrackInfo trackInfo{ .pTrack = pTrack, .ID = m_CurrentTrackID++, .Color = trackColor };
            m_TracksOrder.push_back(m_Tracks.size());
            m_Tracks.push_back(trackInfo);
        }

        [[nodiscard]] inline size_t Size() const
        {
            return m_Tracks.size();
        }

        [[nodiscard]] inline Iterator begin()
        {
            return Iterator{ this, 0 };
        }

        [[nodiscard]] inline Iterator end()
        {
            return Iterator{ this, m_Tracks.size() };
        }
    };
} // namespace quinte
