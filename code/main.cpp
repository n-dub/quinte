#include <Application/Application.hpp>
#include <mimalloc.h>


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


    inline void do_deallocate(void* p, size_t size, size_t alignment) override
    {
        QU_Unused(size);
        return mi_free_aligned(p, alignment);
    }


    inline bool do_is_equal(const memory_resource&) const noexcept override
    {
        return false;
    }
};


static DefaultMemoryResource g_DefaultMemoryResource;


int main()
{
    using namespace quinte;

    auto app = memory::make_unique<quinte::Application>();

    if (!app->Initialize())
        return EXIT_FAILURE;

    return app->Run();
}
