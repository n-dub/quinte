#pragma once
#include <Core/Memory/LinearAllocator.hpp>
#include <Graph/ExecutionGraphNode.hpp>

namespace quinte
{
    namespace audio
    {
        struct EngineProcessInfo;
    }


    class ExecutionGraph final
    {
        memory::LinearAllocator m_NodeAllocator;
        std::pmr::vector<ExecutionGraphNode*> m_InitialNodes;
        std::pmr::vector<ExecutionGraphNode*> m_AllNodes;

    public:
        ~ExecutionGraph();

        void Build();
        void Run(const audio::EngineProcessInfo& processInfo);
    };
} // namespace quinte
