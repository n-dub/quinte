#include <Core/Memory.hpp>
#include <mimalloc.h>

namespace quinte::memory::detail
{
    namespace
    {
        struct DefaultMemoryResource final : std::pmr::memory_resource
        {
            inline DefaultMemoryResource()
            {
                std::pmr::set_default_resource(this);
            }


            inline ~DefaultMemoryResource()
            {
                std::pmr::set_default_resource(std::pmr::new_delete_resource());
            }


            inline void* do_allocate(size_t size, size_t alignment) override
            {
                return mi_malloc_aligned(size, alignment);
            }


            inline void do_deallocate(void* p, size_t, size_t) override
            {
                mi_free(p);
            }


            inline bool do_is_equal(const memory_resource&) const noexcept override
            {
                return false;
            }
        };
    } // namespace


    static DefaultMemoryResource g_DefaultMemoryResource;


    void* DefaultAlloc(size_t byteSize)
    {
        return mi_malloc(byteSize);
    }


    void* DefaultAlloc(size_t byteSize, size_t byteAlignment)
    {
        return mi_malloc_aligned(byteSize, byteAlignment);
    }


    void DefaultFree(void* ptr)
    {
        mi_free(ptr);
    }
} // namespace quinte::memory::detail
