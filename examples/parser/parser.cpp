/**
 * JSON parser example
 * (c) 2023-2024 Łukasz Łasek
 */
#include <iostream>
#include <string_view>

#include "myjson.h"

static std::string_view json1[] = {
    "\n",
    "\n",
    "{\n",
    "    \"key1\": \"value1\",\n",
    "    \"key2\": 123,\n",
    "    \"key3\": true,\n",
    "    \"key4\": [0, 1, \"aaa\", null, false, { \"key10\": \"value\\\\\\\"10\", 456, \"value11\" }],\n",
    "    \"value5\",\n",
    "    false,\n",
    "    \"key_json\": \"{\\\"keyA\\\": \\\"value\\A\\\", \\\"keyB\\\": [\\\"bbbb\\\"]}\"\n",
    "}\n",
    "\n",
    "{\n",
    "    \"key10\": \"value2\",\n",
    "    \"key20\": 456,\n",
    "    \"key30\": false,\n",
    "    \"key40\": [0, 1, \"aaa\", null, false, { \"key20\": \"value\\\\\\\"20\", 789, \"value22\" }],\n",
    "    \"value6\",\n",
    "    true,\n",
    "    \"key_json2\": \"{\\\"keyC\\\": \\\"value\\C\\\", \\\"keyD\\\": [\\\"dd\\DD\\\"]}\"\n",
    "}\n",
};

std::ostream& operator<<(std::ostream& os, myjson::Node::Type jsonType)
{
    switch (jsonType) {
    case myjson::Node::Type::Invalid: return os << "Invalid";

    case myjson::Node::Type::Null: return os << "Null";

    case myjson::Node::Type::Object: return os << "Object";

    case myjson::Node::Type::Array: return os << "Array";

#ifdef JSON_WITH_BOOL
    case myjson::Node::Type::Bool: return os << "Bool";
#endif

#ifdef JSON_WITH_INT
    case myjson::Node::Type::Int: return os << "Int";
#endif

#ifdef JSON_WITH_DOUBLE
    case myjson::Node::Type::Double: return os << "Double";
#endif

#ifdef JSON_WITH_STRING
    case myjson::Node::Type::String: return os << "String";
#endif

    default: return os << "Unknown";
    }
}

void json1Print(myjson::Node::ptr json)
{
    if (!json) {
        std::cout << "invalid json\n";
        return;
    }

    std::cout << json["key4"]->getType() << '\n';
    std::cout << json[3]->getType() << '\n';
    std::cout << json["key4"][0]->getType() << '\n';
    std::cout << json["key4"][1]->getType() << '\n';
    std::cout << json["key4"][2]->getType() << '\n';
    std::cout << json["key4"][3]->getType() << '\n';
    std::cout << json["key4"][4]->getType() << '\n';
    std::cout << json["key4"][5]->getType() << '\n';

    std::cout << *json["key4"][5]["key10"]->getString() << '\n';
    std::cout << *json["key4"][5][1]->getInt() << '\n';
    std::cout << *json["key4"][5][2]->getString() << '\n';

    std::cout << json["key_json"]->getType() << '\n';
    std::cout << *json["key_json"]->getString() << '\n';


    std::cout << "--- Parse the inner json ---\n";
    auto innerJson = myjson::Node::parse(*json["key_json"]->getString());
    std::cout << *innerJson["keyA"]->getString() << '\n';
    std::cout << *innerJson["keyB"][0]->getString() << '\n';
}

void json2Print(myjson::Node::ptr json)
{
    if (!json) {
        std::cout << "invalid json\n";
        return;
    }

    std::cout << json["key40"]->getType() << '\n';
    std::cout << json[3]->getType() << '\n';
    std::cout << json["key40"][0]->getType() << '\n';
    std::cout << json["key40"][1]->getType() << '\n';
    std::cout << json["key40"][2]->getType() << '\n';
    std::cout << json["key40"][3]->getType() << '\n';
    std::cout << json["key40"][4]->getType() << '\n';
    std::cout << json["key40"][5]->getType() << '\n';

    std::cout << *json["key40"][5]["key20"]->getString() << '\n';
    std::cout << *json["key40"][5][1]->getInt() << '\n';
    std::cout << *json["key40"][5][2]->getString() << '\n';

    std::cout << json["key_json2"]->getType() << '\n';
    std::cout << *json["key_json2"]->getString() << '\n';

    std::cout << (json["missing_key"] ? (const char*)"has missing_key" : (const char*)"no missing_key") << '\n';
    std::cout << json["key40"][5][1]->getInt(-1) << '\n';
    std::cout << json["key40"][5][1]->getString("oops") << '\n';

    std::cout << "--- Parse the inner json ---\n";
    auto innerJson = myjson::Node::parse(*json["key_json2"]->getString());
    std::cout << *innerJson["keyC"]->getString() << '\n';
    std::cout << *innerJson["keyD"][0]->getString() << '\n';
}

void parse1()
{
    std::cout << "--- parse1() ---\n";
    std::cout << "--- First json ---\n";

    std::string strJson(json1[0]);
    size_t jsonLines = sizeof(json1) / sizeof(json1[0]);

    for (size_t line = 0; line < jsonLines; line++) {
        strJson += json1[line];
    }

    auto json = myjson::Node::parse(strJson);
    json1Print(json);

}

void parse2()
{
    std::cout << "--- parse2() ---\n";
    std::cout << "--- First json ---\n";

    size_t jsonLines = sizeof(json1) / sizeof(json1[0]);
    size_t currentLine = 0;
    auto json = myjson::Node::parse([&currentLine, jsonLines]() {
        return currentLine < jsonLines ? std::string(json1[currentLine++]) : std::string();
    });

    std::cout << "--- parsed lines: " << currentLine << " ---\n";
    json1Print(json);

    std::cout << "--- Next json ---\n";

    json = myjson::Node::parse([&currentLine, jsonLines]() {
        return currentLine < jsonLines ? std::string(json1[currentLine++]) : std::string();
    });

    std::cout << "--- parsed lines: " << currentLine << " ---\n";
    json2Print(json);
}

void create1()
{
    auto root = myjson::Node::createRootNode();
    root->addNode("int0", 100);
    root->addNode("int1", 101);
    root->addNode({}, 102);
    root->addNode("bool0", false);
    root->addNode("bool1", true);
    root->addNode(myjson::Node::Type::Null, "null0");
    root->addNode(myjson::Node::Type::Null);
    auto json{std::move(root->toString())};

    std::cout << "--- create1() ---\n";
    std::cout << json << '\n';
    auto arr0 = root->addNode(myjson::Node::Type::Array, "arr0");
    auto arr1 = root->addNode(myjson::Node::Type::Array);
    json = std::move(root->toString());
    std::cout << json << '\n';
}

int main(int argc, const char* argv[])
{
    parse1();
    parse2();

    create1();
    return 0;
}
