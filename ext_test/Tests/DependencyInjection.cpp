// checking dependency injection creating objects

#include "pch.h"

#include <ext/core/dependency_injection.h>
#include <ext/core/singleton.h>

struct Interface1
{
    virtual ~Interface1() = default;
};

struct Interface1Impl : Interface1
{};

struct Interface1Impl2 : Interface1
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
    ASSERT_THROW(ext::CreateObject<CreatedObject>(m_serviceCollection.BuildServiceProvider()), ext::di::not_registered);
#pragma warning (pop)
}

TEST_F(DependencyInjectionFixture, Creating_WithRegistration)
{
    m_serviceCollection.RegisterScoped<Interface1, Interface1Impl>();
    m_serviceCollection.RegisterScoped<Interface2, Interface2Impl>();

    const std::shared_ptr<CreatedObject> object = ext::CreateObject<CreatedObject>(m_serviceCollection.BuildServiceProvider());
    ASSERT_NE(nullptr, object);
    EXPECT_NE(nullptr, object->m_i1);
    EXPECT_NE(nullptr, object->m_i2);
}

TEST_F(DependencyInjectionFixture, Creating_Scope)
{
    m_serviceCollection.RegisterScoped<Interface1, Interface1Impl>();
    m_serviceCollection.RegisterScoped<Interface2, Interface2Impl>();

    EXPECT_NE(m_serviceCollection.BuildServiceProvider().get(), m_serviceCollection.BuildServiceProvider()->CreateScope().get());
    EXPECT_NE(nullptr, m_serviceCollection.BuildServiceProvider()->CreateScope());
}

TEST_F(DependencyInjectionFixture, Check_Singleton)
{
    m_serviceCollection.RegisterSingleton<Interface1, Interface1Impl>();
    const auto serviceProvider = m_serviceCollection.BuildServiceProvider();

    const std::shared_ptr<Interface1> interface1 = ext::GetInterface<Interface1>(serviceProvider);
    const std::shared_ptr<Interface1> interface2 = ext::GetInterface<Interface1>(serviceProvider);
    const std::shared_ptr<Interface1Impl> object = ext::CreateObject<Interface1Impl>(serviceProvider);
    EXPECT_NE(nullptr, interface1);
    EXPECT_NE(nullptr, interface2);
    EXPECT_NE(nullptr, object);
    EXPECT_EQ(interface1.get(), interface2.get());
    EXPECT_NE(interface1.get(), object.get());

    EXPECT_EQ(interface1.get(), ext::GetInterface<Interface1>(m_serviceCollection.BuildServiceProvider()).get());
    EXPECT_EQ(interface1.get(), ext::GetInterface<Interface1>(serviceProvider->CreateScope()).get());

#pragma warning (push)
#pragma warning (disable: 4834)
    ASSERT_THROW(ext::GetInterface<Interface2>(serviceProvider), ext::di::not_registered);
#pragma warning (pop)
}

TEST_F(DependencyInjectionFixture, Check_Scoped)
{
    m_serviceCollection.RegisterScoped<Interface1, Interface1Impl>();
    const auto serviceProviderScope1 = m_serviceCollection.BuildServiceProvider();

    const std::shared_ptr<Interface1> interface1InScope1 = ext::GetInterface<Interface1>(serviceProviderScope1);
    const std::shared_ptr<Interface1> interface2InScope1 = ext::GetInterface<Interface1>(serviceProviderScope1);
    const std::shared_ptr<Interface1Impl> objectInScope1 = ext::CreateObject<Interface1Impl>(serviceProviderScope1);
    EXPECT_NE(nullptr, interface1InScope1);
    EXPECT_NE(nullptr, interface2InScope1);
    EXPECT_NE(nullptr, objectInScope1);
    EXPECT_EQ(interface1InScope1.get(), interface2InScope1.get());
    EXPECT_NE(interface1InScope1.get(), objectInScope1.get());

    auto checkScope = [&](const ext::ServiceProvider::Ptr& serviceProvider)
    {
        const std::shared_ptr<Interface1> interface1InScope2 = ext::GetInterface<Interface1>(serviceProvider);
        const std::shared_ptr<Interface1> interface2InScope2 = ext::GetInterface<Interface1>(serviceProvider);
        const std::shared_ptr<Interface1Impl> objectInScope2 = ext::CreateObject<Interface1Impl>(serviceProvider);
        EXPECT_NE(nullptr, interface1InScope2);
        EXPECT_NE(nullptr, interface2InScope2);
        EXPECT_NE(nullptr, objectInScope2);
        EXPECT_EQ(interface1InScope2.get(), interface2InScope2.get());
        EXPECT_NE(interface1InScope2.get(), objectInScope2.get());

        EXPECT_NE(interface1InScope1.get(), interface2InScope2.get()) << "Object should be different in defferent scopes";
    };
    checkScope(m_serviceCollection.BuildServiceProvider());
    checkScope(serviceProviderScope1->CreateScope());
}

TEST_F(DependencyInjectionFixture, Check_Transient)
{
    m_serviceCollection.RegisterTransient<Interface1, Interface1Impl>();
    const auto serviceProviderScope1 = m_serviceCollection.BuildServiceProvider();

    const std::shared_ptr<Interface1> interface1InScope1 = ext::GetInterface<Interface1>(serviceProviderScope1);
    const std::shared_ptr<Interface1> interface2InScope1 = ext::GetInterface<Interface1>(serviceProviderScope1);
    const std::shared_ptr<Interface1Impl> objectInScope1 = ext::CreateObject<Interface1Impl>(serviceProviderScope1);
    EXPECT_NE(nullptr, interface1InScope1);
    EXPECT_NE(nullptr, interface2InScope1);
    EXPECT_NE(nullptr, objectInScope1);
    EXPECT_NE(interface1InScope1.get(), interface2InScope1.get());
    EXPECT_NE(interface1InScope1.get(), objectInScope1.get());

    const auto serviceProviderScope2 = m_serviceCollection.BuildServiceProvider();

    const std::shared_ptr<Interface1> interface1InScope2 = ext::GetInterface<Interface1>(serviceProviderScope2);
    const std::shared_ptr<Interface1> interface2InScope2 = ext::GetInterface<Interface1>(serviceProviderScope2);
    const std::shared_ptr<Interface1Impl> objectInScope2 = ext::CreateObject<Interface1Impl>(serviceProviderScope2);
    EXPECT_NE(nullptr, interface1InScope2);
    EXPECT_NE(nullptr, interface2InScope2);
    EXPECT_NE(nullptr, objectInScope2);
    EXPECT_NE(interface1InScope2.get(), interface2InScope2.get());
    EXPECT_NE(interface1InScope2.get(), objectInScope2.get());
    EXPECT_NE(interface1InScope1.get(), interface1InScope2.get());
    EXPECT_NE(objectInScope1.get(), objectInScope2.get());

#pragma warning (push)
#pragma warning (disable: 4834)
    ASSERT_THROW(ext::GetInterface<Interface2>(serviceProviderScope1), ext::di::not_registered);
    ASSERT_THROW(ext::GetInterface<Interface2>(serviceProviderScope2), ext::di::not_registered);
#pragma warning (pop)
}

TEST_F(DependencyInjectionFixture, Check_MultiRegistration)
{
    m_serviceCollection.RegisterTransient<Interface1, Interface1Impl>();
    m_serviceCollection.RegisterTransient<Interface1, Interface1Impl2>();
    auto serviceProvider = m_serviceCollection.BuildServiceProvider();

    EXPECT_NE(nullptr, std::dynamic_pointer_cast<Interface1Impl2>(ext::GetInterface<Interface1>(serviceProvider)));

    m_serviceCollection.RegisterSingleton<Interface1, Interface1Impl>();
    EXPECT_NE(nullptr, std::dynamic_pointer_cast<Interface1Impl2>(ext::GetInterface<Interface1>(serviceProvider)));

    serviceProvider = m_serviceCollection.BuildServiceProvider();
    const auto interfaceObj1 = ext::GetInterface<Interface1>(serviceProvider);
    EXPECT_NE(nullptr, std::dynamic_pointer_cast<Interface1Impl>(interfaceObj1));
    const auto interfaceObj2 = ext::GetInterface<Interface1>(serviceProvider);
    EXPECT_EQ(interfaceObj1.get(), interfaceObj2.get());
}

int LazyObjectCounter = 0;
TEST_F(DependencyInjectionFixture, LazyGetterTest)
{
    LazyObjectCounter = 0;
    struct LazyObjectTester
    {
        LazyObjectTester(ext::lazy_interface<Interface1> lazyInterface)
            : m_lazy(std::move(lazyInterface))
        {
            ++LazyObjectCounter;
        }

        void UseInterface()
        {
            EXT_UNUSED(m_lazy.get());
        }
        ext::lazy_interface<Interface1> m_lazy;
    };

    struct LazyInterface1 : Interface1
    {
        LazyInterface1()
        {
            ++LazyObjectCounter;
        }
    };

    m_serviceCollection.RegisterTransient<Interface1, LazyInterface1>();
    ext::lazy_object<LazyObjectTester> objectCounter(m_serviceCollection.BuildServiceProvider());
    EXPECT_EQ(0, LazyObjectCounter);
    EXT_UNUSED(objectCounter.get());
    EXPECT_EQ(1, LazyObjectCounter);
    objectCounter->UseInterface();
    EXPECT_EQ(2, LazyObjectCounter);
}