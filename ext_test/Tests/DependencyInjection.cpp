// checking dependency injection creating objects

#include "pch.h"

#include <ext/core/dependency_injection.h>

struct Interface1
{};

struct Interface1Impl : Interface1
{};

struct Interface2
{};

struct Interface2Impl : Interface2
{};

struct ConstructibleObject
{
    explicit ConstructibleObject(std::shared_ptr<Interface1> i1, std::shared_ptr<Interface2> i2)
        : m_i1(std::move(i1))
        , m_i2(std::move(i2))
    {}

    std::shared_ptr<Interface1> m_i1;
    std::shared_ptr<Interface2> m_i2;
};

TEST(ConstructibleObject, Creating_WithoutRegistration)
{
    ext::ServiceCollection collection;
#pragma warning (push)
#pragma warning (disable: 4834)
    ASSERT_THROW(ext::Create<ConstructibleObject>(collection.BuildServiceProvider()), std::exception);
#pragma warning (pop)
}

TEST(ConstructibleObject, Creating_WithRegistration)
{
    ext::ServiceCollection collection;
    collection.AddScoped<Interface1, Interface1Impl>();
    collection.AddScoped<Interface2, Interface2Impl>();

    const std::shared_ptr<ConstructibleObject> object = ext::Create<ConstructibleObject>(collection.BuildServiceProvider());
    ASSERT_NE(nullptr, object);
    EXPECT_NE(nullptr, object->m_i1);
    EXPECT_NE(nullptr, object->m_i2);
}
