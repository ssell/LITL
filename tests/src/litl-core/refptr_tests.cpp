#include <catch2/catch_test_macros.hpp>
#include "litl-core/refPtr.hpp"

class Foo final : public LITL::Core::RefCounted
{
public:

    Foo(bool* destroyedFlag)
        : m_destroyedFlag(destroyedFlag)
    {
        *m_destroyedFlag = false;
    }

    ~Foo()
    {
        *m_destroyedFlag = true;
    }

private:

    bool* m_destroyedFlag;
};

class Bar final : public LITL::Core::RefCounted
{
public:

    Bar(uint32_t* a)
        : m_pa(a), m_pb(nullptr), m_pc(nullptr)
    {
        increment();
    }

    Bar(uint32_t* a, uint32_t* b)
        : m_pa(a), m_pb(b), m_pc(nullptr)
    {
        increment();
    }

    Bar(uint32_t* a, uint32_t* b, uint32_t* c)
        : m_pa(a), m_pb(b), m_pc(c)
    {
        increment();
    }

    ~Bar()
    {
        decrement();
    }

private:

    void increment()
    {
        if (m_pa != nullptr)
        {
            (*m_pa)++;
        }

        if (m_pb != nullptr)
        {
            (*m_pb)++;
        }

        if (m_pc != nullptr)
        {
            (*m_pc)++;
        }
    }

    void decrement()
    {
        if (m_pa != nullptr)
        {
            (*m_pa)--;
        }

        if (m_pb != nullptr)
        {
            (*m_pb)--;
        }

        if (m_pc != nullptr)
        {
            (*m_pc)--;
        }
    }

    uint32_t* m_pa;
    uint32_t* m_pb;
    uint32_t* m_pc;
};

TEST_CASE("Single Reference", "[core::refptr]")
{
    bool destroyedFlag = true;

    {
        auto fooPtr = LITL::Core::RefPtr<Foo>(new Foo(&destroyedFlag));
        REQUIRE(destroyedFlag == false);
    }

    REQUIRE(destroyedFlag == true);
}

TEST_CASE("Multiple References", "[core::refptr]")
{
    bool destroyedFlag = false;

    {
        auto a = LITL::Core::RefPtr<Foo>(new Foo(&destroyedFlag));
        REQUIRE(destroyedFlag == false);

        {
            auto b = LITL::Core::RefPtr<Foo>(a);
            REQUIRE(destroyedFlag == false);

            {
                auto c = b;
                REQUIRE(destroyedFlag == false);
            }

            REQUIRE(destroyedFlag == false);
        }

        REQUIRE(destroyedFlag == false);
    }

    REQUIRE(destroyedFlag == true);
}

TEST_CASE("make_refptr", "[core::refptr]")
{
    uint32_t a = 0;
    uint32_t b = 0;
    uint32_t c = 0;

    {
        auto p0 = LITL::Core::make_refptr<Bar>(&a);
        REQUIRE(a == 1); REQUIRE(b == 0); REQUIRE(c == 0);

        {
            auto p1 = p0;
            REQUIRE(a == 1); REQUIRE(b == 0); REQUIRE(c == 0);

            {
                auto p2 = LITL::Core::make_refptr<Bar>(&a, &b);
                REQUIRE(a == 2); REQUIRE(b == 1); REQUIRE(c == 0);

                auto p3 = p2;

                {
                    auto p4 = LITL::Core::make_refptr<Bar>(&a, &b, &c);
                    REQUIRE(a == 3); REQUIRE(b == 2); REQUIRE(c == 1);

                    {
                        auto p5(p4);
                        REQUIRE(a == 3); REQUIRE(b == 2); REQUIRE(c == 1);
                    }

                    REQUIRE(a == 3); REQUIRE(b == 2); REQUIRE(c == 1);
                }

                REQUIRE(a == 2); REQUIRE(b == 1); REQUIRE(c == 0);
            }

            REQUIRE(a == 1); REQUIRE(b == 0); REQUIRE(c == 0);
        }

        REQUIRE(a == 1); REQUIRE(b == 0); REQUIRE(c == 0);
    }

    REQUIRE(a == 0); REQUIRE(b == 0); REQUIRE(c == 0);
}