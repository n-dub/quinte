#include <Audio/Engine.hpp>
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Session.hpp>
#include <Audio/Transport.hpp>
#include <Core/Memory/TempAllocator.hpp>
#include <Graph/ExecutionGraph.hpp>

namespace quinte
{
    void ExecutionGraph::ProcessNode(const audio::EngineProcessInfo& processInfo, ExecutionGraphNode* pNode)
    {
        Transport* pTransport = Interface<Transport>::Get();
        PortManager* pPortManager = Interface<PortManager>::Get();

        const auto globalRange = processInfo.LocalRange + processInfo.StartTime;

        Track* pTrack = pNode->Track.Get();
        const float amp = pTrack->GetFader()->GetGain().GetAmplitude();
        const uint64_t firstSampleIndex = processInfo.LocalRange.GetFirstSampleIndex();
        const uint64_t length = processInfo.LocalRange.GetLengthInSamples();

        // The first two ports are for the clips, the others only participate in sends/receives

        const std::span<const Rc<Port>> inputPorts = pTrack->GetInputPorts();
        for (uint32_t channelIndex = 0; channelIndex < inputPorts.size(); ++channelIndex)
        {
            Port* pPort = inputPorts[channelIndex].Get();
            pPort->GetBufferView()->Clear(firstSampleIndex, length);

            QU_AssertDebugMsg(pPort->GetDataType() == audio::DataType::Audio, "not implemented");
            AudioPort* pAudioPort = static_cast<AudioPort*>(pPort);
            AudioBufferView* pAudioBuffer = pAudioPort->GetBufferView();

            if (pTransport->IsActuallyRolling())
                pTrack->GetPlaylist().Read(pAudioBuffer, firstSampleIndex, globalRange, channelIndex);

            for (const audio::PortHandle sourceHandle : pPort->GetSources())
            {
                Port* pSource = pPortManager->FindPortByHandle(sourceHandle);
                if ((pSource->GetDesc().Flags & audio::PortFlags::RecordingOnly) == audio::PortFlags::RecordingOnly)
                {
                    if (!pTrack->IsRecordArmed())
                        continue;
                }

                pAudioBuffer->Mix(pSource->GetBufferView(), firstSampleIndex, firstSampleIndex, length);
            }
        }

        // TODO: we should connect the input ports to the output ports directly
        // and generalize port processing, because later we will connect the plug-ins in between.

        const std::span<const Rc<Port>> outputPorts = pTrack->GetOutputPorts();
        for (uint32_t channelIndex = 0; channelIndex < outputPorts.size(); ++channelIndex)
        {
            Port* pOutputPort = outputPorts[channelIndex].Get();
            QU_AssertDebugMsg(pOutputPort->GetDataType() == audio::DataType::Audio, "not implemented");
            AudioPort* pAudioPort = static_cast<AudioPort*>(pOutputPort);
            pAudioPort->GetBufferView()->Clear(firstSampleIndex, length);

            if (channelIndex < inputPorts.size())
            {
                Port* pInputPort = inputPorts[channelIndex].Get();
                pAudioPort->GetBufferView()->MixWithGain(pInputPort->GetBufferView(), amp, 0, firstSampleIndex, length);
            }
        }

        for (ExecutionGraphNode* pOutgoingNode : pNode->Outgoing)
        {
            if (pOutgoingNode->Trigger())
                ProcessNode(processInfo, pOutgoingNode);
        }
    }


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

        Session* pSession = Interface<Session>::Get();
        ExecutionGraphNode* pMasterNode = memory::New<ExecutionGraphNode>(&m_NodeAllocator);
        m_AllNodes.push_back(pMasterNode);
        m_AllNodes.back()->Track = pSession->m_pMasterTrack;

        for (const TrackInfo& trackInfo : pSession->GetTrackList())
        {
            m_AllNodes.push_back(memory::New<ExecutionGraphNode>(&m_NodeAllocator));
            m_AllNodes.back()->Track = trackInfo.pTrack;
            m_InitialNodes.push_back(m_AllNodes.back());
            m_InitialNodes.back()->Outgoing.push_back(pMasterNode);

            // Master track depends on all the other tracks.
            // Once they finish processing, master processing is triggered.
            pMasterNode->InitialDependencyCount++;
        }
    }


    void ExecutionGraph::Run(const audio::EngineProcessInfo& processInfo)
    {
        for (ExecutionGraphNode* pNode : m_AllNodes)
            pNode->DependencyCount = pNode->InitialDependencyCount;
        for (ExecutionGraphNode* pNode : m_InitialNodes)
            ProcessNode(processInfo, pNode);

        PortManager* pPortManager = Interface<PortManager>::Get();
        const StereoPorts& monitorPorts = pPortManager->GetMonitorPorts();
        Port* ports[] = { monitorPorts.Left.Get(), monitorPorts.Right.Get() };

        const uint64_t firstSampleIndex = processInfo.LocalRange.GetFirstSampleIndex();
        const uint64_t length = processInfo.LocalRange.GetLengthInSamples();
        for (Port* pPort : ports)
        {
            pPort->GetBufferView()->Clear(firstSampleIndex, length);
            QU_AssertDebug(pPort->GetDataType() == audio::DataType::Audio);
            AudioPort* pAudioPort = static_cast<AudioPort*>(pPort);
            AudioBufferView* pAudioBuffer = pAudioPort->GetBufferView();
            for (const audio::PortHandle sourceHandle : pPort->GetSources())
            {
                Port* pSource = pPortManager->FindPortByHandle(sourceHandle);
                pAudioBuffer->Mix(pSource->GetBufferView(), firstSampleIndex, firstSampleIndex, length);
            }
        }
    }
} // namespace quinte
