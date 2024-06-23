/**
 * Simple JSON library
 * (c) 2023-2024 Łukasz Łasek
 */
#include "myjson.h"

#include <algorithm>
#include <assert.h>
#include <stack>
#include <string>
#include <string_view>

#ifndef NO_SSTREAM
    #define WITH_SSTREAM
#endif

#ifdef WITH_SSTREAM
    #include <iostream>
    #include <sstream>
#endif

namespace myjson {

#ifdef JSON_WITH_BOOL
class BoolNode : public Node {
public:
    BoolNode(std::string_view key, bool value) : Node(key, ValueType::Bool), value(value) {}
    bool value;
};
#endif // JSON_WITH_BOOL

#ifdef JSON_WITH_INT
class IntNode : public Node {
public:
    IntNode(std::string_view key, long long value) : Node(key, ValueType::Int), value(value) {}
    long long value;
};
#endif // JSON_WITH_INT

#ifdef JSON_WITH_DOUBLE
class DoubleNode : public Node {
public:
    DoubleNode(std::string_view key, double value) : Node(key, ValueType::Double), value(value) {}
    double value;
};
#endif // JSON_WITH_DOUBLE

#ifdef JSON_WITH_STRING
class StringNode : public Node {
public:
    StringNode(std::string_view key, std::string value) : Node(key, ValueType::String), value(value) {}
    std::string value;
};
#endif // JSON_WITH_STRING

class VectorNode : public Node {
public:
    VectorNode(std::string_view key, ValueType type) : Node(key, type) {}

    const Node::ptr operator[](int idx) const {
        if (nodes.size() < (size_t)idx) {
            return {};
        }

        return nodes[idx];
    }

    const Node::ptr operator[](std::string_view key) const {
        auto it = std::find_if(nodes.cbegin(), nodes.cend(), [key](auto child) {
            return child->getKey() == key;
        });

        if (it == nodes.cend()) {
            return {};
        }

        return *it;
    }

    void addNode(Node::ptr node) {
        nodes.push_back(node);
    }

protected:
    std::vector<Node::ptr> nodes;
};

class ObjectNode : public VectorNode {
public:
    ObjectNode(std::string_view key) : VectorNode(key, ValueType::Object) {}
};

class ArrayNode : public VectorNode {
public:
    ArrayNode(std::string_view key) : VectorNode(key, ValueType::Array) {}
};



Node::Node(std::string_view key, ValueType type)
    : type{type}, key{key}
{
}

Node::ValueType Node::getType() const
{
    return type;
}

std::string_view Node::getKey() const
{
    return key;
}

#ifdef JSON_WITH_OPTIONAL
#ifdef JSON_WITH_BOOL
std::optional<bool> Node::getBool() const {
    if (type == ValueType::Bool) {
        return {static_cast<const BoolNode*>(this)->value};
    }
    return {};
}
#endif // JSON_WITH_BOOL

#ifdef JSON_WITH_INT
std::optional<int> Node::getInt() const {
    if (type == ValueType::Int) {
        return {static_cast<const IntNode*>(this)->value};
    }
    return {};
}
#endif // JSON_WITH_INT

#ifdef JSON_WITH_DOUBLE
std::optional<double> Node::getDouble() const {
    if (type == ValueType::Double) {
        return {static_cast<const DoubleNode*>(this)->value};
    }
    return {};
}
#endif // JSON_WITH_DOUBLE

#ifdef JSON_WITH_STRING
std::optional<std::string_view> Node::getString() const {
    if (type == ValueType::String) {
        return {static_cast<const StringNode*>(this)->value};
    }
    return {};
}
#endif // JSON_WITH_STRING
#endif // JSON_WITH_OPTIONAL

#ifdef JSON_WITH_DEFAULT
#ifdef JSON_WITH_BOOL
bool Node::getBool(bool defaultValue) const {
    if (type == ValueType::Bool) {
        return static_cast<const BoolNode*>(this)->value;
    }
    return defaultValue;
}
#endif // JSON_WITH_BOOL

#ifdef JSON_WITH_INT
int Node::getInt(int defaultValue) const {
    if (type == ValueType::Int) {
        return static_cast<const IntNode*>(this)->value;
    }
    return defaultValue;
}
#endif // JSON_WITH_INT

#ifdef JSON_WITH_DOUBLE
double Node::getDouble(double defaultValue) const {
    if (type == ValueType::Double) {
        return static_cast<const DoubleNode*>(this)->value;
    }
    return defaultValue;
}
#endif // JSON_WITH_DOUBLE

#ifdef JSON_WITH_STRING
std::string_view Node::getString(std::string_view defaultValue) const {
    if (type == ValueType::String) {
        return static_cast<const StringNode*>(this)->value;
    }
    return defaultValue;
}
#endif // JSON_WITH_STRING
#endif // JSON_WITH_DEFAULT

const Node::ptr Node::operator[](int idx) const
{
    if (type != ValueType::Array && type != ValueType::Object) {
        return {};
    }

    const auto& vectorNode = *static_cast<const VectorNode*>(this);
    return vectorNode[idx];
}

const Node::ptr Node::operator[](std::string_view key) const
{
    if (type != ValueType::Array && type != ValueType::Object) {
        return {};
    }

    const auto& vectorNode = *static_cast<const VectorNode*>(this);
    return vectorNode[key];
}



class Parser {
public:
    struct Token {
        enum class Type : unsigned int {
            Invalid = 0,

            ObjectName,     // object name is followed by a ':'

            NullValue,
            TrueValue,
            FalseValue,
            IntValue,
            DoubleValue,
            StringValue,

            NewObject,
            EndObject,

            NewArray,
            EndArray,

            Comma,
            Eof,
        };

        Type type;
        std::string value;

        static std::string_view getType(Type type) {
            static std::string_view Sarr[] = {
                "Invalid",
                "ObjectName",
                "NullValue",
                "TrueValue",
                "FalseValue",
                "IntValue",
                "DoubleValue",
                "StringValue",
                "NewObject",
                "EndObject",
                "NewArray",
                "EndArray",
                "Comma",
                "Eof",
            };
            return Sarr[(int)type];
        }
    };

    Parser(std::function<std::string()> fnReadLine)
        : fnReadLine(fnReadLine), jsonIdx(0) {
        json = fnReadLine();
    }

    bool isWhiteCase(const char a_char) {
        switch (a_char) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return true;

        default:
            return false;
        }
    }

    Token::Type getQuotedStringTokenType() {
        while (jsonIdx < json.length()) {
            const char c = json[jsonIdx++];

            if (isWhiteCase(c)) {
                continue;
            }

            switch (c) {
            case ':':
                // @todo_llasek: double :
                return Token::Type::ObjectName;

            default:
                jsonIdx--;
                return Token::Type::StringValue;
            }
        }

        return Token::Type::StringValue;
    }

    Token getQuotedStringToken() {
#ifdef WITH_SSTREAM
        std::stringstream ss;
#else
        std::string str;
#endif
        bool isEscaped = false;

        while (jsonIdx < json.length()) {
            const char c = json[jsonIdx++];

            switch (c) {
            case '\\':
                if (isEscaped) {
                    isEscaped = false;
#ifdef WITH_SSTREAM
                    ss << c;
#else
                    str += c;
#endif
                } else {
                    isEscaped = true;
                }

                break;

            case '"':
                if (isEscaped) {
                    isEscaped = false;
#ifdef WITH_SSTREAM
                    ss << c;
#else
                    str += c;
#endif
                } else {
#ifdef WITH_SSTREAM
                    return Token { getQuotedStringTokenType(), ss.str() };
#else
                    return Token { getQuotedStringTokenType(), str };
#endif
                }

                break;

            default:
                if (isEscaped) {
#ifdef WITH_SSTREAM
                    ss << '\\';
#else
                    str += c;
#endif
                    isEscaped = false;
                }

#ifdef WITH_SSTREAM
                ss << c;
#else
                str += c;
#endif
                break;
            }
        }

#ifdef WITH_SSTREAM
        return Token { getQuotedStringTokenType(), ss.str() };
#else
        return Token { getQuotedStringTokenType(), str };
#endif
    }

    Token parseValueToken(std::string_view value) {
        // special values:
        static struct {
            std::string_view value;
            Token::Type type;
        } SarrSpecialTokens[] = {
            { "null", Token::Type::NullValue },
            { "true", Token::Type::TrueValue },
            { "false", Token::Type::FalseValue },
        };

        for (auto st : SarrSpecialTokens) {
            if (st.value.length() == value.length()
            && std::equal(value.begin(), value.end(), st.value.begin(), st.value.end(), [](char c1, char c2) { return (tolower(c1) == tolower(c2)); })) {
                return Token{st.type};
            }
        }

        // int/double/string value:
        Token::Type tokenType{Token::Type::IntValue};

        for (size_t nIdx = 0; nIdx < value.length(); ++nIdx) {
            auto c = value[nIdx];

            switch (c) {
            case '0'...'9':
                break;

            case '.':
                if ((tokenType == Token::Type::IntValue) && (nIdx + 1 < value.length())) {
                    tokenType = Token::Type::DoubleValue;
                } else {
                    tokenType = Token::Type::StringValue;
                }

                break;

            default:
                tokenType = Token::Type::StringValue;
                break;
            }
        }

        return Token{tokenType, std::string(value)};
    }

    Token getValueToken() {
#ifdef WITH_SSTREAM
        std::stringstream ss;
#else
        std::string str;
#endif

        while (jsonIdx < json.length()) {
            const char c = json[jsonIdx++];

            if (isWhiteCase(c) || c == ',' || c == '}' || c == ']') {
                if (c == '}' || c == ']') {
                    jsonIdx--;
                }

#ifdef WITH_SSTREAM
                return parseValueToken(ss.str());
#else
                return parseValueToken(str);
#endif
            }

#ifdef WITH_SSTREAM
            ss << c;
#else
            str += c;
#endif
        }

#ifdef WITH_SSTREAM
        return parseValueToken(ss.str());
#else
        return parseValueToken(str);
#endif
    }

    Token getNextToken() {
        auto nJsonLen = json.length();

        do {
            while (jsonIdx < nJsonLen) {
                const char c = json[jsonIdx++];

                if (isWhiteCase(c)) {
                    continue;
                }

                switch (c) {
                case ',':
                    return Token{Token::Type::Comma};

                case '{':
                    return Token{Token::Type::NewObject};

                case '}':
                    return Token{Token::Type::EndObject};

                case '[':
                    return Token{Token::Type::NewArray};

                case ']':
                    return Token{Token::Type::EndArray};

                case '"':
                    return getQuotedStringToken();

                default:
                    jsonIdx--;
                    return getValueToken();
                }
            }

            json = fnReadLine();
            nJsonLen = json.length();
            jsonIdx = 0;
        } while (nJsonLen);

        return Token{Token::Type::Eof};
    }

    bool jsonAddNode(std::stack<Node::ptr>& stack, Node::ptr node, Token& nodeName) {
        if (stack.empty()) {
            stack.push(node);
        } else {
            auto parentNode = stack.top();

            switch (parentNode->getType()) {
            case Node::ValueType::Object:
            case Node::ValueType::Array: {
                VectorNode* vectorNode = static_cast<VectorNode*>(parentNode.ptr.get());
                vectorNode->addNode(node);
            }
            break;

            default:
                return false;
            }

            switch (node->getType()) {
            case Node::ValueType::Object:
            case Node::ValueType::Array:
                stack.push(node);
                break;

            default:
                break;
            }
        }

        nodeName.type = Token::Type::Invalid;
        nodeName.value.clear();
        return true;
    }

    Node::ptr jsonRmNode(std::stack<Node::ptr>& stack, Token nodeName) {
        if ((stack.empty()) || (nodeName.type != Token::Type::Invalid)) {
            return {};
        }

        auto jn = stack.top();
        stack.pop();
        return jn;
    }

    Node::ptr parse() {
        std::stack<Node::ptr> stack;
        Node::ptr curNode {};
        Token nodeName{};
        bool isInvalid = false;

        for (; !isInvalid;) {
            if (curNode && stack.empty()) {
                break;
            }

            auto token = getNextToken();

            // @todo_llasek: DBG
            // std::cout << Token::GetTokenType( jt.m_eType ) << '\n';

            if (curNode && stack.empty()) {
                if (token.type == Token::Type::Eof) {
                    break;
                }

                isInvalid = true;
                break;
            }

            switch (token.type) {
            case Token::Type::ObjectName:

                // object name can't follow itself
                if (nodeName.type == token.type) {
                    isInvalid = true;
                }

                nodeName = token;
                break;

            case Token::Type::NullValue:
                curNode.ptr = std::make_shared<Node>(nodeName.value, Node::ValueType::Null);
                isInvalid = !jsonAddNode(stack, curNode, nodeName);
                break;

#ifdef JSON_WITH_BOOL
            case Token::Type::TrueValue:
                curNode.ptr = std::make_shared<BoolNode>(nodeName.value, true);
                isInvalid = !jsonAddNode(stack, curNode, nodeName);
                break;

            case Token::Type::FalseValue:
                curNode.ptr = std::make_shared<BoolNode>(nodeName.value, false);
                isInvalid = !jsonAddNode(stack, curNode, nodeName);
                break;
#endif // JSON_WITH_BOOL

#ifdef JSON_WITH_INT
            case Token::Type::IntValue:
                curNode.ptr = std::make_shared<IntNode>(nodeName.value, strtoll(token.value.c_str(), nullptr, 10));
                isInvalid = !jsonAddNode(stack, curNode, nodeName);
                break;
#endif // JSON_WITH_INT

#ifdef JSON_WITH_DOUBLE
            case Token::Type::DoubleValue:
                curNode.ptr = std::make_shared<DoubleNode>(nodeName.value, strtod(token.value.c_str(), nullptr));
                isInvalid = !jsonAddNode(stack, curNode, nodeName);
                break;
#endif // JSON_WITH_DOUBLE

#ifdef JSON_WITH_STRING
            case Token::Type::StringValue:
                curNode.ptr = std::make_shared<StringNode>(nodeName.value, token.value);
                isInvalid = !jsonAddNode(stack, curNode, nodeName);
                break;
#endif // JSON_WITH_STRING

            case Token::Type::NewObject:
                curNode.ptr = std::make_shared<ObjectNode>(nodeName.value);
                isInvalid = !jsonAddNode(stack, curNode, nodeName);
                break;

            case Token::Type::EndObject:
                curNode = jsonRmNode(stack, nodeName);
                isInvalid = ((!curNode.ptr) || (curNode.ptr->getType() != Node::ValueType::Object));
                assert(isInvalid == false);      // @todo_llasek: DBG
                break;

            case Token::Type::NewArray:
                curNode.ptr = std::make_shared<ArrayNode>(nodeName.value);
                isInvalid = !jsonAddNode(stack, curNode, nodeName);
                break;

            case Token::Type::EndArray:
                curNode = jsonRmNode(stack, nodeName);
                isInvalid = ((!curNode.ptr) || (curNode.ptr->getType() != Node::ValueType::Array));
                assert(isInvalid == false);      // @todo_llasek: DBG
                break;

            case Token::Type::Comma:
                // @todo_llasek: implement corner cases like: subsequent commas, start with comma
                isInvalid = (stack.empty() ||
                             (stack.top().ptr->getType() != Node::ValueType::Object && stack.top().ptr->getType() != Node::ValueType::Array));
                break;

            case Token::Type::Eof:
                isInvalid = true;
                break;

            case Token::Type::Invalid:
            default:
                return {};
            }
        }

        if (isInvalid) {
            return {};
        }

        return curNode;
    }

    std::function<std::string()> fnReadLine;
    std::string json;
    uint32_t jsonIdx;
};

const Node::ptr Node::parse(std::string_view json)
{
    std::string strJson(json);
    Parser parser([&strJson]() {
        std::string str(strJson);
        strJson.clear();
        return str;
    });
    return parser.parse();
}

const Node::ptr Node::parse(std::function<std::string()> fnReadLine)
{
    Parser parser(fnReadLine);
    return parser.parse();
}

}   // namespace myjson
