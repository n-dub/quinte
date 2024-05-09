#include <Audio/Engine.hpp>
#include <Audio/Session.hpp>
#include <Audio/Transport.hpp>
#include <Core/Memory/TempAllocator.hpp>
#include <Graph/ExecutionGraph.hpp>

namespace quinte
{
    ExecutionGraph::~ExecutionGraph()
    {
        m_InitialNodes.clear();
        for (ExecutionGraphNode* pNode : m_AllNodes)
            pNode->~ExecutionGraphNode();

        m_AllNodes.clear();
        m_NodeAllocator.Clear();
    }


    void ExecutionGraph::Build()
    {
        m_InitialNodes.clear();
        for (ExecutionGraphNode* pNode : m_AllNodes)
            pNode->~ExecutionGraphNode();

        m_AllNodes.clear();
        m_NodeAllocator.Clear();
        m_NodeAllocator.Maintain();

        for (const TrackInfo& trackInfo : Interface<Session>::Get()->GetTrackList())
        {
            m_AllNodes.push_back(memory::New<ExecutionGraphNode>(&m_NodeAllocator));
            m_AllNodes.back()->Track = trackInfo.pTrack;
            m_InitialNodes.push_back(m_AllNodes.back());
        }
    }


    void ExecutionGraph::Run(const audio::EngineProcessInfo& processInfo)
    {
        AudioEngine* pEngine = Interface<AudioEngine>::Get();
        Transport* pTransport = Interface<Transport>::Get();
        if (!pTransport->IsActuallyRolling())
            return;

        uint64_t totalSampleCount = processInfo.LocalRange.GetLengthInSamples() * 2;

        memory::TempAllocatorScope temp;
        float* pTempBuf = memory::NewArray<float>(&temp, totalSampleCount);

        const auto globalRange = processInfo.LocalRange + processInfo.StartTime;
        for (ExecutionGraphNode* pNode : m_InitialNodes)
        {
            memory::Zero(pTempBuf, totalSampleCount);

            Track* pTrack = pNode->Track.Get();
            pTrack->GetPlaylist().Read(pTempBuf, globalRange);

            const float amp = pTrack->GetFader()->GetGain().GetAmplitude();
            const uint64_t firstSampleIndex = processInfo.LocalRange.GetFirstSampleIndex();
            const uint64_t length = processInfo.LocalRange.GetLengthInSamples();

            // Routing is not implemented yet so mix everything directly to the monitor ports for now
            pEngine->m_MonitorPorts.Left->GetBufferView()->MixWithGain(pTempBuf, amp, firstSampleIndex, length);
            pEngine->m_MonitorPorts.Right->GetBufferView()->MixWithGain(pTempBuf + length, amp, firstSampleIndex, length);
        }
    }
} // namespace quinte
