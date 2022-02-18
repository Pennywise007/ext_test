// checking dependency injection creating objects

#include "pch.h"

#include <ext/core/dependency_injection.h>
#include <ext/core/singleton.h>

struct Interface1
{};

struct Interface1Impl : Interface1
{};

struct Interface2
{};

struct Interface2Impl : Interface2
{};

struct CreatedObject
{
    explicit CreatedObject(std::shared_ptr<Interface1> i1, std::shared_ptr<Interface2> i2)
        : m_i1(std::move(i1))
        , m_i2(std::move(i2))
    {}

    std::shared_ptr<Interface1> m_i1;
    std::shared_ptr<Interface2> m_i2;
};

struct DependencyInjectionFixture : testing::Test
{
    void SetUp() override
    {
        m_serviceCollection.Clear();
    }

    ext::ServiceCollection& m_serviceCollection = ext::get_service<ext::ServiceCollection>();
};

TEST_F(DependencyInjectionFixture, Creating_WithoutRegistration)
{
#pragma warning (push)
#pragma warning (disable: 4834)
    ASSERT_THROW(ext::Create<CreatedObject>(m_serviceCollection.BuildServiceProvider()), std::exception);
#pragma warning (pop)
}

TEST_F(DependencyInjectionFixture, Creating_WithRegistration)
{
    m_serviceCollection.AddScoped<Interface1, Interface1Impl>();
    m_serviceCollection.AddScoped<Interface2, Interface2Impl>();

    const std::shared_ptr<CreatedObject> object = ext::Create<CreatedObject>(m_serviceCollection.BuildServiceProvider());
    ASSERT_NE(nullptr, object);
    EXPECT_NE(nullptr, object->m_i1);
    EXPECT_NE(nullptr, object->m_i2);
}
