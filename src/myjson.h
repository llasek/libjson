/**
 * Simple JSON library
 * (c) 2023 Łukasz Łasek
 */
#pragma once

#include <string_view>
#include <string>
#include <memory>
#include <functional>

template<typename TDerived, typename TBase = void>
struct my_shared_ptr : my_shared_ptr<TBase, void> {
    std::shared_ptr<TDerived> m_ptr;

    TDerived& operator*() const {
        return *m_ptr;
    }

    TDerived* operator->() const {
        return m_ptr.get();
    }

    explicit operator bool() const {
        return m_ptr.get() ? true : false;
    }
};

template<typename TBase>
struct my_shared_ptr<TBase, void> {
    std::shared_ptr<TBase> m_ptr;

    TBase& operator*() const {
        return *m_ptr;
    }

    TBase* operator->() const {
        return m_ptr.get();
    }

    explicit operator bool() const {
        return m_ptr.get() ? true : false;
    }
};

class CMyJsonNode {
public:
    enum class ValueType : unsigned int {
        Invalid = 0,
        Null,
        Bool,
        Int,
        Double,
        String,
        Object,
        Array,
    };

    CMyJsonNode(std::string_view a_strKey);

    virtual explicit operator bool() const;
    virtual explicit operator int() const;
    virtual explicit operator double() const;
    virtual explicit operator std::string_view() const;

    virtual std::string_view GetKey() const;
    virtual ValueType GetType() const;

    virtual CMyJsonNode& operator[](int a_nIdx);
    virtual CMyJsonNode& operator[](std::string_view a_strKey);

    static my_shared_ptr<CMyJsonNode> Parse(std::string_view a_strJson);
    static my_shared_ptr<CMyJsonNode> Parse(std::function< std::string() > a_fnReadLine);

protected:
    std::string m_strKey;
};
