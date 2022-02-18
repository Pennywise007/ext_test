#include "pch.h"

#include <set>
#include <map>

#include "Samples/Samples_helper.h"

#define USE_PUGI_XML
#include <ext/serialization/iserializable.h>

#include <ext/utils/filesystem.h>

using namespace ext::serializable;
using namespace ext::serializable::serializer;

struct BaseTypes : SerializableObject<BaseTypes>
{
    DECLARE_SERIALIZABLE((long) value, 0);
    DECLARE_SERIALIZABLE((std::string) text);

    DECLARE_SERIALIZABLE((std::map<int, long>) valueMap);
    DECLARE_SERIALIZABLE((std::multimap<unsigned, long>) valueMultimap);
    DECLARE_SERIALIZABLE((std::list<int>) valueList);
    DECLARE_SERIALIZABLE((std::vector<float>) valueVector);
    DECLARE_SERIALIZABLE((std::set<double>) valueSet);
    DECLARE_SERIALIZABLE((std::multiset<int>) valueMultiSet);

    DECLARE_SERIALIZABLE((std::filesystem::path) path);

    DECLARE_SERIALIZABLE((std::optional<bool>) optional);
    DECLARE_SERIALIZABLE((std::optional<std::pair<int, bool>>) optionalPair);
    DECLARE_SERIALIZABLE((std::vector<std::optional<bool>>) vectorOptional);
    DECLARE_SERIALIZABLE((std::pair<int, bool>) pair);
    DECLARE_SERIALIZABLE((std::list<std::pair<int, bool>>) listPair);

    bool testField;
    BaseTypes()
        : testField(true)
    {
        REGISTER_SERIALIZABLE_OBJECT(testField);
    }
    BaseTypes(long val, const std::list<int>& _list) : value(val), valueList(_list)
    {
        REGISTER_SERIALIZABLE_OBJECT(value);
        REGISTER_SERIALIZABLE_OBJECT(valueList);
        REGISTER_SERIALIZABLE_OBJECT(testField);
    }
    BaseTypes(bool /*SetValue*/)
        : BaseTypes()
    {
        SetFieldValues();
    }

    virtual void SetFieldValues()
    {
        value = 213;
        text = "text,\\s2\\\"\\d";

        valueMap = { {0, 2} };
        valueMultimap = { {0, 2} };
        valueList = { {2}, {5} };
        valueVector = { {2.4f}, {5.7f} };
        valueSet = { {2.4}, {5.7} };
        valueMultiSet = { {2}, {5} };

        path = "C:\\Test";

        optional = false;
        optionalPair = { 453, true };
        vectorOptional = { false, true };
        pair = { 7, false };
        listPair = { { 5, true }, { 88, false} };
    }
};

struct SerializableField : ISerializableField
{
    EXT_NODISCARD const wchar_t* GetName() const EXT_NOEXCEPT override { return L"Name"; }
    EXT_NODISCARD SerializableValue SerializeValue() const override { return L"test"; }
    void DeserializeValue(const SerializableValue& value) override { EXT_EXPECT(value == L"test"); }
};

struct SerializableTypes : SerializableObject<SerializableTypes>, BaseTypes, SerializableField
{
    REGISTER_SERIALIZABLE_BASE(BaseTypes, SerializableField);

    DECLARE_SERIALIZABLE((SerializableField) serializableObjectField);

    DECLARE_SERIALIZABLE((BaseTypes) baseTypesField);
    DECLARE_SERIALIZABLE((std::list<BaseTypes>) serializableList);

    DECLARE_SERIALIZABLE((std::shared_ptr<BaseTypes>) serializableSharedPtr);
    DECLARE_SERIALIZABLE((std::unique_ptr<BaseTypes>) serializableUniquePtr);
    DECLARE_SERIALIZABLE((std::vector<std::shared_ptr<BaseTypes>>) serializableUniquePtrList);

    DECLARE_SERIALIZABLE((std::optional<BaseTypes>) serializableOptional);

    DECLARE_SERIALIZABLE((std::map<int, std::shared_ptr<BaseTypes>>) serializableStructsMap);
    DECLARE_SERIALIZABLE((std::map<std::shared_ptr<BaseTypes>, int>) serializableStructsMapShared);
    DECLARE_SERIALIZABLE((std::multimap<int, std::shared_ptr<BaseTypes>>) serializableStructsMultiMap);
    DECLARE_SERIALIZABLE((std::multimap<std::shared_ptr<BaseTypes>, unsigned>) serializableStructsMultiMapShared);
    DECLARE_SERIALIZABLE((std::set<std::shared_ptr<BaseTypes>>) serializableStructsSet);
    DECLARE_SERIALIZABLE((std::multiset<std::shared_ptr<BaseTypes>>) serializableStructsMultiSet);

    DECLARE_SERIALIZABLE((std::vector<std::string>) strings);
    DECLARE_SERIALIZABLE((std::list<std::wstring>) wstrings);
    DECLARE_SERIALIZABLE_N(L"My flag name", (bool) oneFlag, true);

    std::map<int, long> flags2 = [this]()
    {
        SerializableObject<SerializableTypes>::RegisterField(L"flags name", &SerializableTypes::flags2);
        return std::map<int, long>{};
    }();

    void SetFieldValues() override
    {
        BaseTypes::SetFieldValues();
        baseTypesField.SetFieldValues();

        serializableList = { BaseTypes(true), BaseTypes(true) };

        serializableSharedPtr = std::make_shared<BaseTypes>(true);
        serializableUniquePtr = std::make_unique<BaseTypes>(true);
        serializableUniquePtrList = { std::make_shared<BaseTypes>(true) };

        serializableOptional.emplace(BaseTypes(true));

        serializableStructsMap = { {1,std::make_shared<BaseTypes>(true) }};
        serializableStructsMapShared = { {std::make_shared<BaseTypes>(true), 534 }};
        serializableStructsMultiMap = { {1,std::make_shared<BaseTypes>(true) }};
        serializableStructsMultiMapShared = { {std::make_shared<BaseTypes>(true), 534 }};
        serializableStructsSet = { {std::make_shared<BaseTypes>(true) }};
        serializableStructsMultiSet = { {std::make_shared<BaseTypes>(true)}};

        strings = { "123", "Тест1" };
        wstrings = { L"123", L"Тест" };

        oneFlag = false;
        flags2 = { { 0, 1 },  { 1, 15245 } };
    }
};

TEST(TestSerialization, SerializationText)
{
    std::locale::global(std::locale(""));

    SerializableTypes testStruct;

    std::wstring defaultText;
    Executor::SerializeObject(Fabric::TextSerializer(defaultText), testStruct);
    test::samples::compare_with_resource_file(defaultText, IDR_SERIALIZE_TEXT_DEFAULT, L"SERIALIZE_TEXT_DEFAULT");

    testStruct.SetFieldValues();

    std::wstring textAfterModification;
    Executor::SerializeObject(Fabric::TextSerializer(textAfterModification), testStruct);
    test::samples::compare_with_resource_file(textAfterModification, IDR_SERIALIZE_TEXT_MODIFY, L"SERIALIZE_TEXT_MODIFY");

    SerializableTypes otherStruct;
    Executor::DeserializeObject(Fabric::TextDeserializer(textAfterModification), otherStruct);

    std::wstring textAfterDeserializationModifiedStruct;
    Executor::SerializeObject(Fabric::TextSerializer(textAfterDeserializationModifiedStruct), otherStruct);
    EXPECT_STREQ(textAfterDeserializationModifiedStruct.c_str(), textAfterModification.c_str());

    Executor::DeserializeObject(Fabric::TextDeserializer(defaultText), otherStruct);
    std::wstring textAfterRestoringToDefaults;
    Executor::SerializeObject(Fabric::TextSerializer(textAfterRestoringToDefaults), otherStruct);
    EXPECT_STREQ(textAfterRestoringToDefaults.c_str(), defaultText.c_str());
}

TEST(TestSerialization, SerializationXML)
{
    const auto testXmlFilePath = std::filesystem::get_exe_directory() / L"test.xml";

    SerializableTypes testStruct;
    Executor::SerializeObject(Fabric::XMLSerializer(testXmlFilePath), testStruct);
    test::samples::compare_with_resource_file(testXmlFilePath, IDR_SERIALIZE_XML_DEFAULT, L"SERIALIZE_XML_DEFAULT");
    std::filesystem::remove(testXmlFilePath);
    testStruct.SetFieldValues();

    Executor::SerializeObject(Fabric::XMLSerializer(testXmlFilePath), testStruct);
    test::samples::compare_with_resource_file(testXmlFilePath, IDR_SERIALIZE_XML_MODIFY, L"SERIALIZE_XML_MODIFY");

    SerializableTypes otherStruct;
    Executor::DeserializeObject(Fabric::XMLDeserializer(testXmlFilePath), otherStruct);
    std::filesystem::remove(testXmlFilePath);

    Executor::SerializeObject(Fabric::XMLSerializer(testXmlFilePath), otherStruct);
    test::samples::compare_with_resource_file(testXmlFilePath, IDR_SERIALIZE_XML_MODIFY, L"SERIALIZE_XML_MODIFY");

    SerializableTypes defaultStruct;
    Executor::SerializeObject(Fabric::XMLSerializer(testXmlFilePath), defaultStruct);
    Executor::DeserializeObject(Fabric::XMLDeserializer(testXmlFilePath), otherStruct);
    Executor::SerializeObject(Fabric::XMLSerializer(testXmlFilePath), otherStruct);
    test::samples::compare_with_resource_file(testXmlFilePath, IDR_SERIALIZE_XML_DEFAULT, L"SERIALIZE_XML_DEFAULT");
    std::filesystem::remove(testXmlFilePath);
}