#include "tests.hpp"
#include "litl-core/refPtr.hpp"

namespace litl::tests
{
    class Foo final : public RefCounted
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

    class Bar final : public RefCounted
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

    LITL_TEST_CASE("Single Reference", "[core::refptr]")
    {
        bool destroyedFlag = true;

        {
            auto fooPtr = RefPtr<Foo>(new Foo(&destroyedFlag));
            REQUIRE(destroyedFlag == false);
        }

        REQUIRE(destroyedFlag == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Multiple References", "[core::refptr]")
    {
        bool destroyedFlag = false;

        {
            auto a = RefPtr<Foo>(new Foo(&destroyedFlag));
            REQUIRE(destroyedFlag == false);

            {
                auto b = RefPtr<Foo>(a);
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
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("make_refptr", "[core::refptr]")
    {
        uint32_t a = 0;
        uint32_t b = 0;
        uint32_t c = 0;

        {
            auto p0 = make_refptr<Bar>(&a);
            REQUIRE(a == 1); REQUIRE(b == 0); REQUIRE(c == 0);

            {
                auto p1 = p0;
                REQUIRE(a == 1); REQUIRE(b == 0); REQUIRE(c == 0);

                {
                    auto p2 = make_refptr<Bar>(&a, &b);
                    REQUIRE(a == 2); REQUIRE(b == 1); REQUIRE(c == 0);

                    auto p3 = p2;

                    {
                        auto p4 = make_refptr<Bar>(&a, &b, &c);
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
    } LITL_END_TEST_CASE

    Foo* createNewFoo(bool* destroyedFlag)
    {
        return new Foo(destroyedFlag);
    }

    LITL_TEST_CASE("Indirect Creation", "[core::refptr]")
    {
        bool destroyedFlag = false;

        {
            auto refPtr = RefPtr<Foo>(createNewFoo(&destroyedFlag));
            REQUIRE(destroyedFlag == false);
        }

        REQUIRE(destroyedFlag == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("nullptr", "[core::refptr]")
    {
        {
            auto refPtr = RefPtr<Foo>(nullptr);
        }

        // The goal is to not crash when the above refptr goes out of scope. Make sure it doesn't try to delete nullptr.
        REQUIRE(true == true);
    } LITL_END_TEST_CASE
}