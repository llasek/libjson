/**
 * Simple JSON library
 * (c) 2023-2024 Łukasz Łasek
 */
#pragma once

#ifndef JSON_WITHOUT_BOOL
    #define JSON_WITH_BOOL
#endif // JSON_WITHOUT_BOOL
#ifndef JSON_WITHOUT_INT
    #define JSON_WITH_INT
#endif // JSON_WITHOUT_INT
#ifndef JSON_WITHOUT_DOUBLE
    #define JSON_WITH_DOUBLE
#endif // JSON_WITHOUT_DOUBLE
#ifndef JSON_WITHOUT_STRING
    #define JSON_WITH_STRING
#endif // JSON_WITHOUT_STRING

#ifndef JSON_WITHOUT_OPTIONAL
    #define JSON_WITH_OPTIONAL
#endif // JSON_WITHOUT_OPTIONAL
#ifndef JSON_WITHOUT_DEFAULT
    #define JSON_WITH_DEFAULT
#endif // JSON_WITHOUT_DEFAULT

#include <functional>
#include <memory>
#ifdef JSON_WITH_OPTIONAL
    #include <optional>
#endif // JSON_WITH_OPTIONAL
#include <string>
#include <string_view>

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

    enum class ValueType : unsigned int {
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

    Node(std::string_view key, ValueType type);
    virtual ~Node() {};

    ValueType getType() const;
    std::string_view getKey() const;

#ifdef JSON_WITH_OPTIONAL
#ifdef JSON_WITH_BOOL
    std::optional<bool> getBool() const;
#endif // JSON_WITH_BOOL
#ifdef JSON_WITH_INT
    std::optional<int> getInt() const;
#endif // JSON_WITH_INT
#ifdef JSON_WITH_DOUBLE
    std::optional<double> getDouble() const;
#endif // JSON_WITH_DOUBLE
#ifdef JSON_WITH_STRING
    std::optional<std::string_view> getString() const;
#endif // JSON_WITH_STRING
#endif // JSON_WITH_OPTIONAL

#ifdef JSON_WITH_DEFAULT
#ifdef JSON_WITH_BOOL
    bool getBool(bool defaultValue) const;
#endif // JSON_WITH_BOOL
#ifdef JSON_WITH_INT
    int getInt(int defaultValue) const;
#endif // JSON_WITH_INT
#ifdef JSON_WITH_DOUBLE
    double getDouble(double defaultValue) const;
#endif // JSON_WITH_DOUBLE
#ifdef JSON_WITH_STRING
    std::string_view getString(std::string_view defaultValue) const;
#endif // JSON_WITH_STRING
#endif // JSON_WITH_DEFAULT

    const ptr operator[](int idx) const;
    const ptr operator[](std::string_view key) const;

    static const ptr parse(std::string_view json);
    static const ptr parse(std::function<std::string()> fnReadLine);

protected:
    ValueType type;
    std::string key;
};

}   // namespace myjson
