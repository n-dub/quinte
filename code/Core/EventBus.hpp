#pragma once
#include <Core/FixedVector.hpp>
#include <Core/Interface.hpp>

namespace quinte
{
    template<class TEvent>
    class EventHandler : public TEvent
    {
    protected:
        inline EventHandler()
        {
            RegisterBus();
        }

        inline ~EventHandler()
        {
            UnregisterBus();
        }

        inline void RegisterBus();
        inline void UnregisterBus();
    };


    template<class TEvent>
    class EventBus final : public Interface<EventBus<TEvent>>::Registrar
    {
        SmallVector<EventHandler<TEvent>*> m_Handlers;

        template<class F, class... Args>
        inline void SendEventInternal(F&& function, Args&&... args);

        inline void RegisterHandlerInternal(EventHandler<TEvent>* handler);
        inline void UnregisterHandlerInternal(EventHandler<TEvent>* handler);

        static EventBus* Get();

    public:
        using Handler = EventHandler<TEvent>;

        inline EventBus() = default;

        inline static void RegisterHandler(EventHandler<TEvent>* handler);
        inline static void UnregisterHandler(EventHandler<TEvent>* handler);

        template<class F, class... Args>
        inline static void SendEvent(F&& function, Args&&... args);
    };


    template<class TEvent>
    template<class F, class... Args>
    void EventBus<TEvent>::SendEventInternal(F&& function, Args&&... args)
    {
        for (Handler* pHandler : m_Handlers)
        {
            std::invoke(function, pHandler, std::forward<Args>(args)...);
        }
    }


    template<class TEvent>
    template<class F, class... Args>
    void EventBus<TEvent>::SendEvent(F&& function, Args&&... args)
    {
        Get()->SendEventInternal(std::forward<F>(function), std::forward<Args>(args)...);
    }


    template<class TEvent>
    void EventBus<TEvent>::RegisterHandlerInternal(Handler* handler)
    {
        m_Handlers.push_back(handler);
    }


    template<class TEvent>
    void EventBus<TEvent>::UnregisterHandlerInternal(Handler* handler)
    {
        const auto it = std::find(m_Handlers.begin(), m_Handlers.end(), handler);
        QU_AssertDebug(it != m_Handlers.end());
        m_Handlers.erase(it);
    }


    template<class TEvent>
    void EventBus<TEvent>::RegisterHandler(EventHandler<TEvent>* handler)
    {
        Get()->RegisterHandlerInternal(handler);
    }


    template<class TEvent>
    void EventBus<TEvent>::UnregisterHandler(EventHandler<TEvent>* handler)
    {
        Get()->UnregisterHandlerInternal(handler);
    }


    template<class TEvent>
    EventBus<TEvent>* EventBus<TEvent>::Get()
    {
        return Interface<EventBus<TEvent>>::Get();
    }


    template<class TEvent>
    void EventHandler<TEvent>::RegisterBus()
    {
        EventBus<TEvent>::RegisterHandler(this);
    }


    template<class TEvent>
    void EventHandler<TEvent>::UnregisterBus()
    {
        EventBus<TEvent>::UnregisterHandler(this);
    }
} // namespace quinte
