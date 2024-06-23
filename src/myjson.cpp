/**
 * Simple JSON library
 * (c) 2023-2024 Łukasz Łasek
 */
#include "myjson.h"

#include <assert.h>
#include <algorithm>
#include <stack>
#include <string>
#include <string_view>

#ifdef JSON_WITH_SSTREAM
    #include <iostream>
    #include <sstream>

    using TStringBuf = std::stringstream;
#else
    using TStringBuf = std::string;
#endif

namespace myjson {

template<class TBuf>
std::string helper_printBuf(TBuf& buf)
{
    return buf.str();
}

template<>
std::string helper_printBuf(std::string& buf)
{
    return buf;
}

template<class TValue, class TBuf>
void helper_appendBuf(TValue value, TBuf& buf)
{
    buf << value;
}

template<class TValue>
void helper_appendBuf(TValue value, std::string& buf)
{
    buf += value;
}



#ifdef JSON_WITH_BOOL
class BoolNode : public Node {
public:
    BoolNode(std::string_view key, bool value) : Node(key, Type::Bool), value(value) {}
    bool value;
};

template<class TBuf>
void helper_boolNodeToString(const Node* node, TBuf& buf)
{
    auto value = static_cast<const BoolNode*>(node)->value;
    helper_appendBuf(value ? "true" : "false", buf);
};

#ifdef JSON_WITH_OPTIONAL
std::optional<bool> Node::getBool() const {
    if (type == Type::Bool) {
        return {static_cast<const BoolNode*>(this)->value};
    }
    return {};
}
#endif // JSON_WITH_OPTIONAL

#ifdef JSON_WITH_DEFAULT
bool Node::getBool(bool defaultValue) const {
    if (type == Type::Bool) {
        return static_cast<const BoolNode*>(this)->value;
    }
    return defaultValue;
}
#endif // JSON_WITH_DEFAULT

Node::ptr Node::addNode(std::string_view key, bool value)
{
    return addNode([key, value]() -> Node::ptr {
        return {std::make_shared<BoolNode>(key, value)};
    });
}
#endif // JSON_WITH_BOOL



#ifdef JSON_WITH_INT
class IntNode : public Node {
public:
    IntNode(std::string_view key, long long value) : Node(key, Type::Int), value(value) {}
    long long value;
};

template<class TBuf>
void helper_intNodeToString(const Node* node, TBuf& buf)
{
    auto value = static_cast<const IntNode*>(node)->value;
    helper_appendBuf(std::move(std::to_string(value)), buf);
};

#ifdef JSON_WITH_OPTIONAL
std::optional<int> Node::getInt() const {
    if (type == Type::Int) {
        return {static_cast<const IntNode*>(this)->value};
    }
    return {};
}
#endif // JSON_WITH_OPTIONAL

#ifdef JSON_WITH_DEFAULT
int Node::getInt(int defaultValue) const {
    if (type == Type::Int) {
        return static_cast<const IntNode*>(this)->value;
    }
    return defaultValue;
}
#endif // JSON_WITH_DEFAULT

Node::ptr Node::addNode(std::string_view key, int value)
{
    return addNode([key, value]() -> Node::ptr {
        return {std::make_shared<IntNode>(key, value)};
    });
}
#endif // JSON_WITH_INT



#ifdef JSON_WITH_DOUBLE
class DoubleNode : public Node {
public:
    DoubleNode(std::string_view key, double value) : Node(key, Type::Double), value(value) {}
    double value;
};

template<class TBuf>
void helper_doubleNodeToString(const Node* node, TBuf& buf)
{
    auto value = static_cast<const DoubleNode*>(node)->value;
    helper_appendBuf(std::move(std::to_string(value)), buf);
};

#ifdef JSON_WITH_OPTIONAL
std::optional<double> Node::getDouble() const {
    if (type == Type::Double) {
        return {static_cast<const DoubleNode*>(this)->value};
    }
    return {};
}
#endif // JSON_WITH_OPTIONAL

#ifdef JSON_WITH_DEFAULT
double Node::getDouble(double defaultValue) const {
    if (type == Type::Double) {
        return static_cast<const DoubleNode*>(this)->value;
    }
    return defaultValue;
}
#endif // JSON_WITH_DEFAULT

Node::ptr Node::addNode(std::string_view key, double value)
{
    return addNode([key, value]() -> Node::ptr {
        return {std::make_shared<DoubleNode>(key, value)};
    });
}
#endif // JSON_WITH_DOUBLE



#ifdef JSON_WITH_STRING
class StringNode : public Node {
public:
    StringNode(std::string_view key, std::string_view value) : Node(key, Type::String), value(value) {}
    std::string value;
};

template<class TBuf>
void helper_stringNodeToString(const Node* node, TBuf& buf)
{
    const auto& value = static_cast<const StringNode*>(node)->value;
    std::string escapedValue;
    escapedValue.reserve(value.size());
    for (auto c : value) {
        switch (c) {
        case '\"':
        case '\\':
            escapedValue += '\\';
            break;

        default:
            break;
        }
        escapedValue += c;
    }
    helper_appendBuf("\"", buf);
    helper_appendBuf(escapedValue, buf);
    helper_appendBuf("\"", buf);
};

#ifdef JSON_WITH_OPTIONAL
std::optional<std::string_view> Node::getString() const {
    if (type == Type::String) {
        return {static_cast<const StringNode*>(this)->value};
    }
    return {};
}
#endif // JSON_WITH_OPTIONAL

#ifdef JSON_WITH_DEFAULT
std::string_view Node::getString(std::string_view defaultValue) const {
    if (type == Type::String) {
        return static_cast<const StringNode*>(this)->value;
    }
    return defaultValue;
}
#endif // JSON_WITH_DEFAULT

Node::ptr Node::addNode(std::string_view key, std::string_view value)
{
    return addNode([key, value]() -> Node::ptr {
        return {std::make_shared<StringNode>(key, value)};
    });
}
#endif // JSON_WITH_STRING



class VectorNode : public Node {
public:
    VectorNode(std::string_view key, Type type) : Node(key, type) {}

    const Node::ptr operator[](int idx) const {
        if (nodes.size() <= (size_t)idx) {
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

template<class TBuf>
void helper_vectorNodeToString(const Node* node, TBuf& buf)
{
    int idx = 0;
    for (auto childNode = (*node)[idx]; childNode; childNode = (*node)[++idx]) {
        if (idx) {
            helper_appendBuf(",", buf);
        }
        helper_toString(childNode.ptr.get(), buf);
    }
};



class ObjectNode : public VectorNode {
public:
    ObjectNode(std::string_view key) : VectorNode(key, Type::Object) {}
};



class ArrayNode : public VectorNode {
public:
    ArrayNode(std::string_view key) : VectorNode(key, Type::Array) {}
};



Node::Node(std::string_view key, Type type)
    : type{type}, key{key}
{
}

Node::Type Node::getType() const
{
    return type;
}

std::string_view Node::getKey() const
{
    return key;
}

const Node::ptr Node::operator[](int idx) const
{
    if (type != Type::Array && type != Type::Object) {
        return {};
    }

    const auto& vectorNode = *static_cast<const VectorNode*>(this);
    return vectorNode[idx];
}

const Node::ptr Node::operator[](std::string_view key) const
{
    if (type != Type::Array && type != Type::Object) {
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

        // static std::string_view getType(Type type) {
        //     static std::string_view Sarr[] = {
        //         "Invalid",
        //         "ObjectName",
        //         "NullValue",
        //         "TrueValue",
        //         "FalseValue",
        //         "IntValue",
        //         "DoubleValue",
        //         "StringValue",
        //         "NewObject",
        //         "EndObject",
        //         "NewArray",
        //         "EndArray",
        //         "Comma",
        //         "Eof",
        //     };
        //     return Sarr[(int)type];
        // }
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
        TStringBuf strBuf;
        bool isEscaped = false;

        while (jsonIdx < json.length()) {
            const char c = json[jsonIdx++];

            switch (c) {
            case '\\':
                if (isEscaped) {
                    isEscaped = false;
                    helper_appendBuf(c, strBuf);
                } else {
                    isEscaped = true;
                }

                break;

            case '"':
                if (isEscaped) {
                    isEscaped = false;
                    helper_appendBuf(c, strBuf);
                } else {
                    return Token{getQuotedStringTokenType(), helper_printBuf(strBuf)};
                }

                break;

            default:
                if (isEscaped) {
                    helper_appendBuf(c, strBuf);
                    isEscaped = false;
                }

                helper_appendBuf(c, strBuf);
                break;
            }
        }

        return Token{getQuotedStringTokenType(), helper_printBuf(strBuf)};
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
        TStringBuf strBuf;

        while (jsonIdx < json.length()) {
            const char c = json[jsonIdx++];

            if (isWhiteCase(c) || c == ',' || c == '}' || c == ']') {
                if (c == '}' || c == ']') {
                    jsonIdx--;
                }

                return parseValueToken(helper_printBuf(strBuf));
            }

            helper_appendBuf(c, strBuf);
        }

        return parseValueToken(helper_printBuf(strBuf));
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
            case Node::Type::Object:
            case Node::Type::Array: {
                VectorNode* vectorNode = static_cast<VectorNode*>(parentNode.ptr.get());
                vectorNode->addNode(node);
            }
            break;

            default:
                return false;
            }

            switch (node->getType()) {
            case Node::Type::Object:
            case Node::Type::Array:
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
                curNode.ptr = std::make_shared<Node>(nodeName.value, Node::Type::Null);
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
                isInvalid = ((!curNode.ptr) || (curNode.ptr->getType() != Node::Type::Object));
                assert(isInvalid == false);      // @todo_llasek: DBG
                break;

            case Token::Type::NewArray:
                curNode.ptr = std::make_shared<ArrayNode>(nodeName.value);
                isInvalid = !jsonAddNode(stack, curNode, nodeName);
                break;

            case Token::Type::EndArray:
                curNode = jsonRmNode(stack, nodeName);
                isInvalid = ((!curNode.ptr) || (curNode.ptr->getType() != Node::Type::Array));
                assert(isInvalid == false);      // @todo_llasek: DBG
                break;

            case Token::Type::Comma:
                // @todo_llasek: implement corner cases like: subsequent commas, start with comma
                isInvalid = (stack.empty() ||
                             (stack.top().ptr->getType() != Node::Type::Object && stack.top().ptr->getType() != Node::Type::Array));
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



Node::ptr Node::createRootNode()
{
    return {std::make_shared<ObjectNode>(std::string_view{})};
}

Node::ptr Node::addNode(std::function<ptr()> createNewNode)
{
    auto parentType = getType();
    if (parentType != Type::Object && parentType != Type::Array) {
        return {};
    }

    auto vectorNode = static_cast<VectorNode*>(this);
    auto childNode = createNewNode();
    vectorNode->addNode(childNode);
    return childNode;
}

Node::ptr Node::addNode(Type type, std::string_view key)
{
    if (type != Type::Null && type != Type::Object && type != Type::Array) {
        return {};
    }

    switch (type) {
    case Type::Null:
        return addNode([type, key]() -> Node::ptr {
            return {std::make_shared<Node>(key, type)};
        });

    case Type::Object:
        return addNode([key]() -> Node::ptr {
            return {std::make_shared<ObjectNode>(key)};
        });

    case Type::Array:
        return addNode([key]() -> Node::ptr {
            return {std::make_shared<ArrayNode>(key)};
        });
        break;

    default:
        return {};
    }
}



template<class TBuf>
void helper_toString(const Node* node, TBuf& buf)
{
    auto nodeType = node->getType();
    auto nodeKey = node->getKey();

    if (nodeKey.length() > 0) {
        helper_appendBuf("\"", buf);
        helper_appendBuf(nodeKey, buf);
        helper_appendBuf("\":", buf);
    }

    switch (nodeType) {
    case Node::Type::Null:
        helper_appendBuf("null", buf);
        break;

    case Node::Type::Object:
        helper_appendBuf("{", buf);
        helper_vectorNodeToString(node, buf);
        helper_appendBuf("}", buf);
        break;

    case Node::Type::Array:
        helper_appendBuf("[", buf);
        helper_vectorNodeToString(node, buf);
        helper_appendBuf("]", buf);
        break;

#ifdef JSON_WITH_BOOL
    case Node::Type::Bool:
        helper_boolNodeToString(node, buf);
        break;
#endif // JSON_WITH_BOOL

#ifdef JSON_WITH_INT
    case Node::Type::Int:
        helper_intNodeToString(node, buf);
        break;
#endif // JSON_WITH_INT

#ifdef JSON_WITH_DOUBLE
    case Node::Type::Double:
        helper_doubleNodeToString(node, buf);
        break;
#endif // JSON_WITH_DOUBLE

#ifdef JSON_WITH_STRING
    case Node::Type::String:
        helper_stringNodeToString(node, buf);
        break;
#endif // JSON_WITH_STRING

    default:
        helper_appendBuf("<invalid>", buf);
        break;
    }
}

std::string Node::toString() const
{
    TStringBuf strBuf;
    helper_toString(this, strBuf);
    return helper_printBuf(strBuf);
}

}   // namespace myjson
