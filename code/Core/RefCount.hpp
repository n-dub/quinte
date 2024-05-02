#pragma once
#include <Core/Memory.hpp>
#include <atomic>
#include <concepts>
#include <memory_resource>

namespace quinte
{
    namespace detail
    {
        class RcBase;
    }


    namespace memory
    {
        class RefCountedObjectBase
        {
            friend class ::quinte::detail::RcBase;

            std::atomic<uint32_t> m_RefCount = 0;
            uint32_t m_AllocationSize = 0;
            std::pmr::memory_resource* m_pAllocator = nullptr;

        public:
            inline uint32_t AddRef()
            {
                return ++m_RefCount;
            }

            template<std::invocable F>
            inline uint32_t Release(F&& destroyCallback)
            {
                const uint32_t refCount = --m_RefCount;
                if (refCount == 0)
                {
                    destroyCallback();
                    m_pAllocator->deallocate(this, m_AllocationSize, memory::kDefaultAlignment);
                }

                return refCount;
            }
        };
    } // namespace memory


    namespace detail
    {
        template<class T1, class = void>
        struct IsComplete : std::false_type
        {
        };


        template<class T1>
        struct IsComplete<T1, decltype(void(sizeof(T1)))> : std::true_type
        {
        };


        template<class TBase, class TDerived, bool>
        struct IsBaseOf : std::true_type
        {
        };


        template<class TBase, class TDerived>
        struct IsBaseOf<TBase, TDerived, true> : std::is_base_of<TBase, TDerived>
        {
        };


        template<class T>
        class PtrRef final
        {
            T* m_Ptr;

        public:
            using ValueType = typename T::ValueType;

            inline explicit PtrRef(T* ptr)
                : m_Ptr(ptr)
            {
            }

            inline operator ValueType**()
            {
                return m_Ptr->ReleaseAndGetAddressOf();
            }

            inline ValueType* operator*()
            {
                return m_Ptr->Get();
            }
        };


        class RcBase
        {
        protected:
            inline void SetupRefCounter(memory::RefCountedObjectBase* pObject, std::pmr::memory_resource* pAllocator,
                                        uint32_t allocationSize)
            {
                pObject->m_AllocationSize = allocationSize;
                pObject->m_pAllocator = pAllocator;
            }
        };
    } // namespace detail


    template<class T>
    concept RefCountedObject = (alignof(T) <= memory::kDefaultAlignment)
        && detail::IsBaseOf<memory::RefCountedObjectBase, T, detail::IsComplete<T>::value>::value;


    template<RefCountedObject T>
    class Rc final : public detail::RcBase
    {
        T* m_pObject = nullptr;

        inline void InternalAddRef() const
        {
            if (m_pObject)
                m_pObject->AddRef();
        }

        inline uint32_t InternalRelease()
        {
            uint32_t result = 0;
            if (m_pObject)
            {
                result = m_pObject->Release([m_pObject] {
                    m_pObject->~T();
                });

                m_pObject = nullptr;
            }

            return result;
        }

    public:
        inline Rc() = default;

        inline ~Rc()
        {
            InternalRelease();
        }

        inline explicit Rc(T* pObject)
            : m_pObject(pObject)
        {
            InternalAddRef();
        }

        template<std::derived_from<T> TOther>
        inline Rc(const Rc<TOther>& other)
            : m_pObject(other.Get())
        {
            InternalAddRef();
        }

        template<std::derived_from<T> TOther>
        inline Rc(Rc<TOther>&& other)
            : m_pObject(other.Detach())
        {
        }

        template<std::derived_from<T> TOther>
        inline Rc& operator=(const Rc<TOther>& other)
        {
            if (static_cast<RcBase*>(this) == static_cast<RcBase*>(&other))
                return;

            Attach(other.Get());
            InternalAddRef();
            return *this;
        }

        template<std::derived_from<T> TOther>
        inline Rc& operator=(Rc<TOther>&& other)
        {
            Attach(other.Detach());
            return *this;
        }

        //! \brief Release a reference and reset to null.
        inline void Reset()
        {
            InternalRelease();
        }

        //! \brief Get pointer to the stored pointer.
        inline T* const* GetAddressOf() const
        {
            return &m_pObject;
        }

        //! \brief Get pointer to the stored pointer.
        inline T** GetAddressOf()
        {
            return &m_pObject;
        }

        //! \brief Release a reference and get pointer to the stored pointer.
        //!
        //! It is the same as using unary '&' operator.
        inline T** ReleaseAndGetAddressOf()
        {
            InternalRelease();
            return &m_pObject;
        }

        //! \brief Attach a pointer and do not add strong reference.
        inline void Attach(T* pObject)
        {
            InternalRelease();
            m_pObject = pObject;
        }

        //! \brief Detach the stored pointer: reset to null without decrementing the reference counter.
        inline T* Detach()
        {
            T* ptr = m_pObject;
            m_pObject = nullptr;
            return ptr;
        }

        //! \brief Get underlying raw pointer.
        inline T* Get() const
        {
            return m_pObject;
        }

        inline detail::PtrRef<Rc<T>> operator&()
        {
            return detail::PtrRef<Rc<T>>(this);
        }

        inline T& operator*()
        {
            return *Get();
        }

        inline T* operator->()
        {
            return Get();
        }

        inline const T& operator*() const
        {
            return *Get();
        }

        inline const T* operator->() const
        {
            return Get();
        }

        inline explicit operator bool() const
        {
            return Get();
        }

        template<class... TArgs>
        inline static Rc<T> New(std::pmr::memory_resource* pAllocator, TArgs&&... args)
        {
            T* ptr = new (pAllocator->allocate(sizeof(T), memory::kDefaultAlignment)) T(std::forward<TArgs>(args)...);
            SetupRefCounter(ptr, pAllocator, static_cast<uint32_t>(sizeof(T)));
            return Rc<T>{ ptr };
        }

        template<class... TArgs>
        inline static Rc<T> DefaultNew(TArgs&&... args)
        {
            std::pmr::memory_resource* pAllocator = std::pmr::get_default_resource();
            T* ptr = new (pAllocator->allocate(sizeof(T), memory::kDefaultAlignment)) T(std::forward<TArgs>(args)...);
            SetupRefCounter(ptr, pAllocator, static_cast<uint32_t>(sizeof(T)));
            return Rc<T>{ ptr };
        }
    };


    template<RefCountedObject T>
    inline bool operator==(const Rc<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() == nullptr;
    }


    template<RefCountedObject T>
    inline bool operator!=(const Rc<T>& lhs, std::nullptr_t)
    {
        return !(lhs == nullptr);
    }


    template<RefCountedObject T>
    inline bool operator==(std::nullptr_t, const Rc<T>& rhs)
    {
        return rhs.Get() == nullptr;
    }


    template<RefCountedObject T>
    inline bool operator!=(std::nullptr_t, const Rc<T>& rhs)
    {
        return !(nullptr == rhs);
    }


    template<RefCountedObject T1, RefCountedObject T2>
    requires std::derived_from<T1, T2> || std::derived_from<T2, T1>
    inline bool operator==(const Rc<T1>& lhs, T2* rhs)
    {
        return static_cast<memory::RefCountedObjectBase*>(lhs.Get()) == static_cast<memory::RefCountedObjectBase*>(rhs);
    }


    template<RefCountedObject T1, RefCountedObject T2>
    requires std::derived_from<T1, T2> || std::derived_from<T2, T1>
    inline bool operator!=(const Rc<T1>& lhs, T2* rhs)
    {
        return !(lhs == rhs);
    }


    template<RefCountedObject T1, RefCountedObject T2>
    requires std::derived_from<T1, T2> || std::derived_from<T2, T1>
    inline bool operator==(T1* lhs, const Rc<T2>& rhs)
    {
        return rhs == lhs;
    }


    template<RefCountedObject T1, RefCountedObject T2>
    requires std::derived_from<T1, T2> || std::derived_from<T2, T1>
    inline bool operator!=(T1* lhs, const Rc<T2>& rhs)
    {
        return !(lhs == rhs);
    }


    template<RefCountedObject T1, RefCountedObject T2>
    requires std::derived_from<T1, T2> || std::derived_from<T2, T1>
    inline bool operator==(const Rc<T1>& lhs, const Rc<T2>& rhs)
    {
        return lhs.Get() == rhs.Get();
    }


    template<RefCountedObject T1, RefCountedObject T2>
    requires std::derived_from<T1, T2> || std::derived_from<T2, T1>
    inline bool operator!=(const Rc<T1>& lhs, const Rc<T2>& rhs)
    {
        return !(lhs == rhs);
    }
} // namespace quinte
