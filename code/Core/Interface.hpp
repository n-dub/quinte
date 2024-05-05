#pragma once
#include <Core/Hash.hpp>

namespace quinte
{
    namespace detail
    {
        void RegisterInstance(uint64_t typeHash, void* pInstance);
        void UnregisterInstance(uint64_t typeHash, void* pInstance);
        void* FindInstance(uint64_t typeHash);
    } // namespace detail


    template<class T>
    class Interface final
    {
    public:
        static void Register(T* pInstance);
        static void Unregister(T* pInstance);

        static T* Get();

        struct Registrar
        {
            Registrar();
            virtual ~Registrar();
        };
    };


    template<class T>
    inline void Interface<T>::Register(T* pInstance)
    {
        // TODO: logging
        printf("Registered %.*s\n", (int)TypeName<T>.length(), TypeName<T>.data());
        static_assert(TypeNameHash<T> != 0);
        detail::RegisterInstance(TypeNameHash<T>, pInstance);
    }


    template<class T>
    inline void Interface<T>::Unregister(T* pInstance)
    {
        printf("Unregistered %.*s\n", (int)TypeName<T>.length(), TypeName<T>.data());
        detail::UnregisterInstance(TypeNameHash<T>, pInstance);
    }


    template<class T>
    inline T* Interface<T>::Get()
    {
        return static_cast<T*>(detail::FindInstance(TypeNameHash<T>));
    }


    template<class T>
    inline Interface<T>::Registrar::Registrar()
    {
        Interface<T>::Register(static_cast<T*>(this));
    }


    template<class T>
    inline Interface<T>::Registrar::~Registrar()
    {
        Interface<T>::Unregister(static_cast<T*>(this));
    }
} // namespace quinte
