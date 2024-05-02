#include <Core/Core.hpp>
#include <gtest/gtest.h>

using namespace quinte;

struct DummyObject final : memory::RefCountedObjectBase
{
    int32_t* m_pCount = nullptr;

    inline DummyObject(int32_t* pCounter)
        : m_pCount(pCounter)
    {
        *m_pCount += 1;
    }

    ~DummyObject() override
    {
        *m_pCount -= 1;
    }
};

TEST(RefCounter, Basic)
{
    int32_t cnt = 0;

    {
        const Rc<DummyObject> ptr = Rc<DummyObject>::DefaultNew(&cnt);
        EXPECT_EQ(cnt, 1);
        EXPECT_EQ(ptr->GetRefCount(), 1);
        EXPECT_EQ(*ptr.GetAddressOf(), ptr.Get());
    }

    EXPECT_EQ(cnt, 0);
}

TEST(RefCounter, CopyConstructor)
{
    int32_t cnt = 0;

    {
        const Rc<DummyObject> ptr = Rc<DummyObject>::DefaultNew(&cnt);
        EXPECT_EQ(cnt, 1);
        EXPECT_EQ(ptr->GetRefCount(), 1);

        {
            const Rc<DummyObject> other = ptr;
            EXPECT_EQ(cnt, 1);
            EXPECT_EQ(ptr.Get(), other.Get());
            EXPECT_EQ(other->GetRefCount(), 2);
            EXPECT_EQ(ptr->GetRefCount(), 2);
        }

        EXPECT_EQ(cnt, 1);
        EXPECT_EQ(ptr->GetRefCount(), 1);
    }

    EXPECT_EQ(cnt, 0);
}

TEST(RefCounter, MoveConstructor)
{
    int32_t cnt = 0;

    {
        Rc<DummyObject> ptr = Rc<DummyObject>::DefaultNew(&cnt);
        EXPECT_EQ(cnt, 1);
        EXPECT_EQ(ptr->GetRefCount(), 1);

        {
            const Rc<DummyObject> other = std::move(ptr);
            EXPECT_EQ(cnt, 1);
            EXPECT_EQ(other->GetRefCount(), 1);
        }

        EXPECT_EQ(cnt, 0);
    }

    EXPECT_EQ(cnt, 0);
}

TEST(RefCounter, CopyAssign)
{
    int32_t cnt1 = 0;
    int32_t cnt2 = 0;

    {
        const Rc<DummyObject> ptr = Rc<DummyObject>::DefaultNew(&cnt1);
        EXPECT_EQ(cnt1, 1);
        EXPECT_EQ(ptr->GetRefCount(), 1);

        {
            Rc<DummyObject> other = Rc<DummyObject>::DefaultNew(&cnt2);
            EXPECT_EQ(cnt1, 1);
            EXPECT_EQ(cnt2, 1);
            EXPECT_EQ(ptr->GetRefCount(), 1);
            EXPECT_EQ(other->GetRefCount(), 1);

            other = ptr;
            EXPECT_EQ(cnt1, 1);
            EXPECT_EQ(cnt2, 0);
            EXPECT_EQ(ptr->GetRefCount(), 2);
            EXPECT_EQ(other->GetRefCount(), 2);
        }

        EXPECT_EQ(cnt1, 1);
        EXPECT_EQ(cnt2, 0);
        EXPECT_EQ(ptr->GetRefCount(), 1);
    }

    EXPECT_EQ(cnt1, 0);
    EXPECT_EQ(cnt2, 0);
}

TEST(RefCounter, MoveAssign)
{
    int32_t cnt1 = 0;
    int32_t cnt2 = 0;

    {
        Rc<DummyObject> ptr = Rc<DummyObject>::DefaultNew(&cnt1);
        EXPECT_EQ(cnt1, 1);
        EXPECT_EQ(ptr->GetRefCount(), 1);

        {
            Rc<DummyObject> other = Rc<DummyObject>::DefaultNew(&cnt2);
            EXPECT_EQ(cnt1, 1);
            EXPECT_EQ(cnt2, 1);
            EXPECT_EQ(ptr->GetRefCount(), 1);
            EXPECT_EQ(other->GetRefCount(), 1);

            other = std::move(ptr);
            EXPECT_EQ(cnt1, 1);
            EXPECT_EQ(cnt2, 0);
            EXPECT_EQ(other->GetRefCount(), 1);
        }

        EXPECT_EQ(cnt1, 0);
        EXPECT_EQ(cnt2, 0);
    }

    EXPECT_EQ(cnt1, 0);
    EXPECT_EQ(cnt2, 0);
}

TEST(RefCounter, ReleaseAndGetAddressOf)
{
    int32_t cnt = 0;
    {
        Rc<DummyObject> ptr = Rc<DummyObject>::DefaultNew(&cnt);
        EXPECT_EQ(cnt, 1);

        DummyObject** ppResetPtr = ptr.ReleaseAndGetAddressOf();
        EXPECT_EQ(cnt, 0);

        *ppResetPtr = Rc<DummyObject>::DefaultNew(&cnt);
        (*ppResetPtr)->AddRef();
        EXPECT_EQ(cnt, 1);
    }

    EXPECT_EQ(cnt, 0);
}
