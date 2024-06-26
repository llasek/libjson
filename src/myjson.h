/**
 * Simple JSON library
 * (c) 2023-2024 Łukasz Łasek
 */
#pragma once

#include "myjsondef.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#ifdef JSON_WITH_OPTIONAL
    #include <optional>
#endif // JSON_WITH_OPTIONAL

namespace myjson {

template<typename TDerived, typename TBase = void>
struct my_shared_ptr : my_shared_ptr<TBase, void> {
    std::shared_ptr<TDerived> ptr;

    TDerived& operator*() const {
        return *ptr;
    }

    TDerived* operator->() const {
        return ptr.get();
    }

    explicit operator bool() const {
        return ptr.get() ? true : false;
    }

    const my_shared_ptr<TDerived> operator[](int idx) const {
        return (*ptr)[idx];
    }

    const my_shared_ptr<TDerived> operator[](std::string_view key) const {
        return (*ptr)[key];
    }
};

template<typename TBase>
struct my_shared_ptr<TBase, void> {
    std::shared_ptr<TBase> ptr;

    TBase& operator*() const {
        return *ptr;
    }

    TBase* operator->() const {
        return ptr.get();
    }

    explicit operator bool() const {
        return ptr.get() ? true : false;
    }

    const my_shared_ptr<TBase> operator[](int idx) const {
        return (*ptr)[idx];
    }

    const my_shared_ptr<TBase> operator[](std::string_view key) const {
        return (*ptr)[key];
    }
};

class Node {
public:
    using ptr = my_shared_ptr<Node>;

    enum class Type : unsigned int {
        Invalid = 0,
        Null,
        Object,
        Array,
#ifdef JSON_WITH_BOOL
        Bool,
#endif // JSON_WITH_BOOL
#ifdef JSON_WITH_INT
        Int,
#endif // JSON_WITH_INT
#ifdef JSON_WITH_DOUBLE
        Double,
#endif // JSON_WITH_DOUBLE
#ifdef JSON_WITH_STRING
        String,
#endif // JSON_WITH_STRING
    };

    Node(std::string_view key, Type type);
    virtual ~Node() {};

    /**
     * Get node type
     */
    Type getType() const;

    /**
     * Return a node key
     */
    std::string_view getKey() const;

#ifdef JSON_WITH_BOOL
#ifdef JSON_WITH_OPTIONAL
    std::optional<bool> getBool() const;
#endif // JSON_WITH_OPTIONAL
#ifdef JSON_WITH_DEFAULT
    bool getBool(bool defaultValue) const;
#endif // JSON_WITH_DEFAULT
    ptr addNode(std::string_view key, bool value);
#endif // JSON_WITH_BOOL

#ifdef JSON_WITH_INT
#ifdef JSON_WITH_OPTIONAL
    std::optional<int> getInt() const;
#endif // JSON_WITH_OPTIONAL
#ifdef JSON_WITH_DEFAULT
    int getInt(int defaultValue) const;
#endif // JSON_WITH_DEFAULT
    ptr addNode(std::string_view key, int value);
#endif // JSON_WITH_INT

#ifdef JSON_WITH_DOUBLE
#ifdef JSON_WITH_OPTIONAL
    std::optional<double> getDouble() const;
#endif // JSON_WITH_OPTIONAL
#ifdef JSON_WITH_DEFAULT
    double getDouble(double defaultValue) const;
#endif // JSON_WITH_DEFAULT
    ptr addNode(std::string_view key, double value);
#endif // JSON_WITH_DOUBLE

#ifdef JSON_WITH_STRING
#ifdef JSON_WITH_OPTIONAL
    std::optional<std::string_view> getString() const;
#endif // JSON_WITH_OPTIONAL
#ifdef JSON_WITH_DEFAULT
    std::string_view getString(std::string_view defaultValue) const;
#endif // JSON_WITH_DEFAULT
    ptr addNode(std::string_view key, std::string_view value);
#endif // JSON_WITH_STRING

    /**
     * Object and array accessor
     */
    const ptr operator[](int idx) const;

    /**
     * Object and array accessor
     */
    const ptr operator[](std::string_view key) const;

    /**
     * Parse a string
     */
    static const ptr parse(std::string_view json);

    /**
     * Parse a string
     */
    static const ptr parse(std::function<std::string()> fnReadLine);

    /**
     * Create a root object node
     */
    static ptr createRootNode();

    /**
     * Add a child object, array or null-typed node
     */
    ptr addNode(Type type = Type::Object, std::string_view key = {});

    /**
     * Convert to a string
     */
    std::string toString() const;

protected:
    ptr addNode(std::function<ptr()> createNewNode);

    Type type;
    std::string key;
};

}   // namespace myjson
