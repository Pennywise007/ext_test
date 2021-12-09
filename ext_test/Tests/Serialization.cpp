#include "pch.h"

#include <set>
#include <map>

#include "Samples/Samples_helper.h"

#define USE_PUGI_XML
#include <ext/serialization/iserializable.h>

#include <ext/utils/filesystem.h>

using namespace ext::serializable;
using namespace ext::serializable::serializer;

struct InternalStruct : SerializableObject<InternalStruct>
{
    DECLARE_SERIALIZABLE((long) value);
    DECLARE_SERIALIZABLE((std::list<int>) valueList);

    bool testField;
    InternalStruct()
        : testField(true)
    {
        REGISTER_SERIALIZABLE_OBJECT(testField);
    }
    InternalStruct(long val, const std::list<int>& _list) : value(val), valueList(_list)
    {
        REGISTER_SERIALIZABLE_OBJECT(value);
        REGISTER_SERIALIZABLE_OBJECT(valueList);
        REGISTER_SERIALIZABLE_OBJECT(testField);
    }
};

struct SerializableField : ISerializableField
{
    SSH_NODISCARD const wchar_t* GetName() const SSH_NOEXCEPT override { return L"Name"; }
    SSH_NODISCARD SerializableValue SerializeValue() const override { return L"test"; }
    void DeserializeValue(const SerializableValue& value) override { SSH_EXPECT(value == L"test"); }
};

struct MyTestStruct : SerializableObject<MyTestStruct>, InternalStruct
{
    REGISTER_SERIALIZABLE_BASE(InternalStruct);

    DECLARE_SERIALIZABLE((long) value);

    DECLARE_SERIALIZABLE((SerializableField) field);
    DECLARE_SERIALIZABLE((InternalStruct) internalStruct);

    DECLARE_SERIALIZABLE((std::map<int, long>) valueMap, {{0, 1},  {1, 15245}});
    DECLARE_SERIALIZABLE((std::multimap<unsigned, long>) valueMultimap);
    DECLARE_SERIALIZABLE((std::list<int>) valueList, { 23, 1123 });
    DECLARE_SERIALIZABLE((std::vector<float>) valueVector, {0.4f, 20.5555f});
    DECLARE_SERIALIZABLE((std::set<double>) valueSet, {0.1, 5.55});
    DECLARE_SERIALIZABLE((std::multiset<int>) valueMultiSet, { 10, 5 });

    DECLARE_SERIALIZABLE((std::list<SerializableField>) serializableList, { SerializableField(), SerializableField()});
    DECLARE_SERIALIZABLE((std::vector<SerializableField>) serializableVector, { SerializableField() });

    DECLARE_SERIALIZABLE((std::map<int, std::shared_ptr<InternalStruct>>) internalStructsMap);
    DECLARE_SERIALIZABLE((std::map<std::shared_ptr<InternalStruct>, int>) internalStructsMapShared);
    DECLARE_SERIALIZABLE((std::multimap<int, std::shared_ptr<InternalStruct>>) internalStructsMultiMap);
    DECLARE_SERIALIZABLE((std::multimap<std::shared_ptr<InternalStruct>, unsigned>) internalStructsMultiMapShared);
    DECLARE_SERIALIZABLE((std::list<std::shared_ptr<InternalStruct>>) internalStructsList);
    DECLARE_SERIALIZABLE((std::vector<std::shared_ptr<InternalStruct>>) internalStructsVector);
    DECLARE_SERIALIZABLE((std::set<std::shared_ptr<InternalStruct>>) internalStructsSet);
    DECLARE_SERIALIZABLE((std::multiset<std::shared_ptr<InternalStruct>>) internalStructsMultiSet);

    DECLARE_SERIALIZABLE((std::vector<std::wstring>) strings, { L"TEST" });
    DECLARE_SERIALIZABLE_N((bool) oneFlag, L"My flag name", true);

    std::map<int, long> flags2 = [this]() -> decltype(std::remove_pointer_t<decltype(this)>::flags2)
    {
        SerializableObject<MyTestStruct>::RegisterField(this, &MyTestStruct::flags2, L"flags name");
        return { { 0, 1 },  { 1, 15245 } };
    }();
};

namespace {

void modify_struct_settings(MyTestStruct& test)
{
    test.internalStruct.value = 123123123;
    test.internalStructsMap.emplace(std::make_pair(20, new InternalStruct{ 10, {1, 2} }));
    test.internalStructsMultiMap.emplace(std::make_pair(30, new InternalStruct(30, { 10, 20 })));
    test.valueSet.emplace(2.2);
    test.valueSet.emplace(5.6);
    test.strings = { L"123", L"213" };
    test.oneFlag = false;
}

} // namespace

TEST(TestSerialization, SerializationText)
{
    MyTestStruct testStruct;

    std::wstring defaultText;
    Executor::SerializeObject(Fabric::TextSerializer(defaultText), testStruct);
    test::samples::compare_with_resource_file(defaultText, IDR_SERIALIZE_TEXT_DEFAULT, L"SERIALIZE_TEXT_DEFAULT");

    modify_struct_settings(testStruct);

    std::wstring textAfterModification;
    Executor::SerializeObject(Fabric::TextSerializer(textAfterModification), testStruct);
    test::samples::compare_with_resource_file(textAfterModification, IDR_SERIALIZE_TEXT_MODIFY, L"SERIALIZE_TEXT_MODIFY");

    MyTestStruct otherStruct;
    Executor::DeserializeObject(Fabric::TextDeserializer(textAfterModification), otherStruct);

    std::wstring textAfterDeserializationModifiedStruct;
    Executor::SerializeObject(Fabric::TextSerializer(textAfterDeserializationModifiedStruct), otherStruct);
    test::samples::compare_with_resource_file(textAfterDeserializationModifiedStruct, IDR_SERIALIZE_TEXT_MODIFY, L"SERIALIZE_TEXT_MODIFY");

    EXPECT_STREQ(textAfterDeserializationModifiedStruct.c_str(), textAfterModification.c_str());
}

TEST(TestSerialization, SerializationXML)
{
    const auto testXmlFilePath = std::filesystem::get_exe_directory() / L"test.xml";

    MyTestStruct testStruct;
    Executor::SerializeObject(Fabric::XMLSerializer(testXmlFilePath), testStruct);
    test::samples::compare_with_resource_file(testXmlFilePath, IDR_SERIALIZE_XML_DEFAULT, L"SERIALIZE_XML_DEFAULT");
    std::filesystem::remove(testXmlFilePath);

    modify_struct_settings(testStruct);

    Executor::SerializeObject(Fabric::XMLSerializer(testXmlFilePath), testStruct);
    test::samples::compare_with_resource_file(testXmlFilePath, IDR_SERIALIZE_XML_MODIFY, L"SERIALIZE_XML_MODIFY");

    MyTestStruct otherStruct;
    Executor::DeserializeObject(Fabric::XMLDeserializer(testXmlFilePath), otherStruct);
    std::filesystem::remove(testXmlFilePath);

    Executor::SerializeObject(Fabric::XMLSerializer(testXmlFilePath), otherStruct);
    test::samples::compare_with_resource_file(testXmlFilePath, IDR_SERIALIZE_XML_MODIFY, L"SERIALIZE_XML_MODIFY");
    std::filesystem::remove(testXmlFilePath);
}