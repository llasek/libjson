/**
 * Simple JSON library
 * (c) 2023 Łukasz Łasek
 */
#include "myjson.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>

#ifndef NO_SSTREAM
#define WITH_SSTREAM
#endif

#ifdef WITH_SSTREAM
#include <sstream>
#include <iostream>
#endif
#include <stack>

#include <assert.h>


CMyJsonNode::CMyJsonNode( std::string_view a_strKey )
    : m_strKey( a_strKey )
{
}

CMyJsonNode::operator bool() const
{
#if __cpp_exceptions
    throw std::bad_cast();
#else
    return false;
#endif
}

CMyJsonNode::operator int() const
{
#if __cpp_exceptions
    throw std::bad_cast();
#else
    return std::numeric_limits<int>::min();;
#endif
}

CMyJsonNode::operator double() const
{
#if __cpp_exceptions
    throw std::bad_cast();
#else
    return std::numeric_limits<double>::quiet_NaN();
#endif
}

CMyJsonNode::operator std::string_view() const
{
#if __cpp_exceptions
    throw std::bad_cast();
#else
    return "";
#endif
}

std::string_view CMyJsonNode::GetKey() const
{
    return m_strKey;
}

CMyJsonNode::ValueType CMyJsonNode::GetType() const
{
    return CMyJsonNode::ValueType::Invalid;
}



class CMyJsonNodeNull : public CMyJsonNode
{
public:
    CMyJsonNodeNull( std::string_view a_strKey )
        : CMyJsonNode( a_strKey )
    {
    }

    explicit operator int() const override
    {
        return 0;
    }

    explicit operator std::string_view() const override
    {
        return "null";
    }

    ValueType GetType() const override
    {
        return ValueType::Null;
    }

protected:
};



CMyJsonNode& CMyJsonNode::operator[]( int a_nIdx )
{
#if __cpp_exceptions
    throw std::out_of_range( "Node doesn't exist" );
#else
    static CMyJsonNodeNull jn( "" );
    return jn;
#endif
}

CMyJsonNode& CMyJsonNode::operator[]( std::string_view a_strKey )
{
#if __cpp_exceptions
    throw std::out_of_range( "Node doesn't exist" );
#else
    static CMyJsonNodeNull jn( "" );
    return jn;
#endif
}



class CMyJsonNodeBool : public CMyJsonNode
{
public:
    CMyJsonNodeBool( std::string_view a_strKey, bool a_bValue )
        : CMyJsonNode( a_strKey ), m_bValue( a_bValue )
    {
    }

    explicit operator bool() const override
    {
        return m_bValue;
    }

    explicit operator std::string_view() const override
    {
        return ( m_bValue ) ? "true" : "false";
    }

    ValueType GetType() const override
    {
        return ValueType::Bool;
    }

protected:
    bool m_bValue;
};



class CMyJsonNodeInt : public CMyJsonNode
{
public:
    CMyJsonNodeInt( std::string_view a_strKey, long long a_nValue )
        : CMyJsonNode( a_strKey ), m_nValue( a_nValue )
    {
    }

    explicit operator int() const override
    {
        return m_nValue;
    }

    ValueType GetType() const override
    {
        return ValueType::Int;
    }

protected:
    long long m_nValue;
};



class CMyJsonNodeDouble : public CMyJsonNode
{
public:
    CMyJsonNodeDouble( std::string_view a_strKey, double a_dValue )
        : CMyJsonNode( a_strKey ), m_dValue( a_dValue )
    {
    }

    explicit operator double() const override
    {
        return m_dValue;
    }

    ValueType GetType() const override
    {
        return ValueType::Double;
    }

protected:
    double m_dValue;
};



class CMyJsonNodeString : public CMyJsonNode
{
public:
    CMyJsonNodeString( std::string_view a_strKey, std::string_view a_strValue )
        : CMyJsonNode( a_strKey ), m_strValue( a_strValue )
    {
    }

    explicit operator std::string_view() const override
    {
        return m_strValue;
    }

    ValueType GetType() const override
    {
        return ValueType::String;
    }

protected:
    std::string m_strValue;
};



class CMyJsonNodeObject : public CMyJsonNode
{
public:
    CMyJsonNodeObject( std::string_view a_strKey )
        : CMyJsonNode( a_strKey )
    {
    }

    ValueType GetType() const override
    {
        return ValueType::Object;
    }

    CMyJsonNode& operator[]( int a_nIdx ) override
    {
        if( m_vecNodes.size() < (size_t)a_nIdx )
        {
#if __cpp_exceptions
            throw std::out_of_range( "Node doesn't exist" );
#else
            static CMyJsonNodeNull jn( "" );
            return jn;
#endif
        }
        return *m_vecNodes[ a_nIdx ].m_ptr.get();
    }

    CMyJsonNode& operator[]( std::string_view a_strKey ) override
    {
        auto it = std::find_if( m_vecNodes.begin(), m_vecNodes.end(),
            [ a_strKey ]( auto pChild ){
                return pChild.m_ptr->GetKey() == a_strKey;
            });
        if( it == m_vecNodes.end())
        {
#if __cpp_exceptions
            throw std::out_of_range( "Node doesn't exist" );
#else
            static CMyJsonNodeNull jn( "" );
            return jn;
#endif
        }
        return *it->m_ptr.get();
    }

    void AddNode( my_shared_ptr< CMyJsonNode > a_jn )
    {
        m_vecNodes.push_back( a_jn );
    }

protected:
    std::vector< my_shared_ptr< CMyJsonNode >> m_vecNodes;
};



class CMyJsonNodeArray : public CMyJsonNode
{
public:
    CMyJsonNodeArray( std::string_view a_strKey )
        : CMyJsonNode( a_strKey )
    {
    }

    ValueType GetType() const override
    {
        return ValueType::Array;
    }

    CMyJsonNode& operator[]( int a_nIdx ) override
    {
        if( m_vecNodes.size() < (size_t)a_nIdx )
        {
#if __cpp_exceptions
            throw std::out_of_range( "Node doesn't exist" );
#else
            static CMyJsonNodeNull jn( "" );
            return jn;
#endif
        }
        return *m_vecNodes[ a_nIdx ].m_ptr;
    }

    CMyJsonNode& operator[]( std::string_view a_strKey ) override
    {
        auto it = std::find_if( m_vecNodes.begin(), m_vecNodes.end(), [ a_strKey ]( auto pChild ){ return pChild.m_ptr->GetKey() == a_strKey; });
        if( it == m_vecNodes.end())
        {
#if __cpp_exceptions
            throw std::out_of_range( "Node doesn't exist" );
#else
            static CMyJsonNodeNull jn( "" );
            return jn;
#endif
        }
        return *it->m_ptr;
    }

    void AddNode( my_shared_ptr< CMyJsonNode > a_jn )
    {
        m_vecNodes.push_back( a_jn );
    }

protected:
    std::vector< my_shared_ptr< CMyJsonNode >> m_vecNodes;
};



struct SMyJsonToken
{
    enum class TokenType : unsigned int
    {
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

    TokenType m_eType;
    std::string m_strValue;

    static std::string_view GetTokenType( TokenType a_tt )
    {
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

        return Sarr[(int)a_tt ];
    }
};



class CMyJsonParser
{
public:
    CMyJsonParser( std::string_view a_strJson )
        : m_strJson( a_strJson ), m_nJsonIdx( 0 )
    {
    }

    bool IsWhiteCase( const char a_char )
    {
        switch( a_char )
        {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                return true;
        }
        return false;
    }

    SMyJsonToken::TokenType GetQuotedStringTokenType()
    {
        while( m_nJsonIdx < m_strJson.length())
        {
            const char c = m_strJson[ m_nJsonIdx++ ];
            if( IsWhiteCase( c ))
            {
                continue;
            }

            switch( c )
            {
                case ':':
                    // @todo_llasek: double :
                    return SMyJsonToken::TokenType::ObjectName;

                default:
                    m_nJsonIdx--;
                    return SMyJsonToken::TokenType::StringValue;
            }
        }
        return SMyJsonToken::TokenType::Invalid;
    }

    SMyJsonToken GetQuotedStringToken()
    {
#ifdef WITH_SSTREAM
        std::stringstream ss;
#else
        std::string str;
#endif
        bool bEscape = false;
        while( m_nJsonIdx < m_strJson.length())
        {
            const char c = m_strJson[ m_nJsonIdx++ ];
            switch( c )
            {
                case '\\':
                    if( bEscape )
                    {
                        bEscape = false;
#ifdef WITH_SSTREAM
                        ss << c;
#else
                        str += c;
#endif
                    }
                    else
                    {
                        bEscape = true;
                    }
                    break;

                case '"':
                    if( bEscape )
                    {
                        bEscape = false;
#ifdef WITH_SSTREAM
                        ss << c;
#else
                        str += c;
#endif
                    }
                    else
                    {
#ifdef WITH_SSTREAM
                        return SMyJsonToken{ GetQuotedStringTokenType(), ss.str() };
#else
                        return SMyJsonToken{ GetQuotedStringTokenType(), str };
#endif
                    }
                    break;

                default:
                    if( bEscape ) {
#ifdef WITH_SSTREAM
                        ss << '\\';
#else
                        str += c;
#endif
                        bEscape = false;
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
        return SMyJsonToken{ GetQuotedStringTokenType(), ss.str() };
#else
        return SMyJsonToken{ GetQuotedStringTokenType(), str };
#endif
    }

    SMyJsonToken ParseValueToken( std::string_view a_strValue )
    {
        // special values:
        static struct
        {
            std::string_view m_str;
            SMyJsonToken::TokenType m_tt;
        } SarrSpecialTokens[] = {
            { "null", SMyJsonToken::TokenType::NullValue },
            { "true", SMyJsonToken::TokenType::TrueValue },
            { "false", SMyJsonToken::TokenType::FalseValue },
        };
        for( auto st : SarrSpecialTokens )
        {
            if(( st.m_str.length() == a_strValue.length()) && ( std::equal( a_strValue.begin(), a_strValue.end(), st.m_str.begin(), st.m_str.end(),
                []( char c1, char c2 )
                {
                    return( tolower( c1 ) == tolower( c2 ));
                })))
            {
                return SMyJsonToken{ st.m_tt };
            }
        }

        // int/double/string value:
        SMyJsonToken::TokenType tt{ SMyJsonToken::TokenType::IntValue };
        for( size_t nIdx = 0; nIdx < a_strValue.length(); ++nIdx )
        {
            auto c = a_strValue[ nIdx ];
            switch( c )
            {
                case '0'...'9':
                    break;

                case '.':
                    if(( tt == SMyJsonToken::TokenType::IntValue ) && ( nIdx + 1 < a_strValue.length()))
                    {
                        tt = SMyJsonToken::TokenType::DoubleValue;
                    }
                    else
                    {
                        tt = SMyJsonToken::TokenType::StringValue;
                    }
                    break;

                default:
                    tt = SMyJsonToken::TokenType::StringValue;
                    break;
            }
        }
        return SMyJsonToken{ tt, std::string( a_strValue )};
    }

    SMyJsonToken GetValueToken()
    {
#ifdef WITH_SSTREAM
        std::stringstream ss;
#else
        std::string str;
#endif
        while( m_nJsonIdx < m_strJson.length())
        {
            const char c = m_strJson[ m_nJsonIdx++ ];
            if( IsWhiteCase( c ) || c == ',' || c == '}' || c == ']' )
            {
                if( c == '}' || c == ']' ) {
                    m_nJsonIdx--;
                }
#ifdef WITH_SSTREAM
                return ParseValueToken( ss.str());
#else
                return ParseValueToken( str );
#endif
            }

#ifdef WITH_SSTREAM
            ss << c;
#else
            str += c;
#endif
        }
#ifdef WITH_SSTREAM
        return ParseValueToken( ss.str());
#else
        return ParseValueToken( str );
#endif
    }

    SMyJsonToken GetNextToken()
    {
        while( m_nJsonIdx < m_strJson.length())
        {
            const char c = m_strJson[ m_nJsonIdx++ ];
            if( IsWhiteCase( c ))
            {
                continue;
            }

            switch( c )
            {
                case ',':
                    return SMyJsonToken{ SMyJsonToken::TokenType::Comma };

                case '{':
                    return SMyJsonToken{ SMyJsonToken::TokenType::NewObject };

                case '}':
                    return SMyJsonToken{ SMyJsonToken::TokenType::EndObject };

                case '[':
                    return SMyJsonToken{ SMyJsonToken::TokenType::NewArray };

                case ']':
                    return SMyJsonToken{ SMyJsonToken::TokenType::EndArray };

                case '"':
                    return GetQuotedStringToken();

                default:
                    m_nJsonIdx--;
                    return GetValueToken();
            }
        }
        return SMyJsonToken{ SMyJsonToken::TokenType::Eof };
    }

    bool JsonAddNode( std::stack< my_shared_ptr< CMyJsonNode >>& a_stack, my_shared_ptr< CMyJsonNode > a_jn, SMyJsonToken& a_jtObjName )
    {
        if( a_stack.empty())
        {
            a_stack.push( a_jn );
        }
        else
        {
            auto jnParent = a_stack.top();
            switch( jnParent.m_ptr->GetType())
            {
                case CMyJsonNode::ValueType::Object:
                    {
                        CMyJsonNodeObject* jnObject = (CMyJsonNodeObject*)jnParent.m_ptr.get();
                        jnObject->AddNode( a_jn );
                    }
                    break;

                case CMyJsonNode::ValueType::Array:
                    {
                        CMyJsonNodeArray* jnArray = (CMyJsonNodeArray*)jnParent.m_ptr.get();
                        jnArray->AddNode( a_jn );
                    }
                    break;

                default:
                    return false;
            }

            switch( a_jn.m_ptr->GetType())
            {
                case CMyJsonNode::ValueType::Object:
                case CMyJsonNode::ValueType::Array:
                    a_stack.push( a_jn );
                    break;

                default:
                    break;
            }
        }

        a_jtObjName.m_eType = SMyJsonToken::TokenType::Invalid;
        a_jtObjName.m_strValue.clear();
        return true;
    }

    my_shared_ptr< CMyJsonNode > JsonRmNode(std::stack< my_shared_ptr< CMyJsonNode >>& a_stack, SMyJsonToken a_jtObjName )
    {
        if(( a_stack.empty()) || ( a_jtObjName.m_eType != SMyJsonToken::TokenType::Invalid ))
        {
            return my_shared_ptr< CMyJsonNode > {};
        }
        auto jn = a_stack.top();
        a_stack.pop();
        return jn;
    }

    my_shared_ptr< CMyJsonNode > Parse()
    {
        const static my_shared_ptr< CMyJsonNode > SpJsonInvalid{};

        std::stack< my_shared_ptr< CMyJsonNode >> stack;
        my_shared_ptr< CMyJsonNode > curNode {};
        SMyJsonToken jtObjName{};
        bool bInvalid = false;

        for( ; !bInvalid; )
        {
            auto jt = GetNextToken();

            // @todo_llasek: DBG
            // std::cout << SMyJsonToken::GetTokenType( jt.m_eType ) << '\n';

            if(( curNode.m_ptr ) && ( stack.empty()))
            {
                if( jt.m_eType == SMyJsonToken::TokenType::Eof )
                {
                    break;
                }
                bInvalid = true;
                break;
            }

            switch( jt.m_eType )
            {
                case SMyJsonToken::TokenType::ObjectName:
                    if( jtObjName.m_eType == jt.m_eType )
                    {
                        bInvalid = true;
                    }
                    jtObjName = jt;
                    break;

                case SMyJsonToken::TokenType::NullValue:
                    curNode.m_ptr = std::make_shared< CMyJsonNodeNull >( jtObjName.m_strValue );
                    bInvalid = !JsonAddNode( stack, curNode, jtObjName );
                    break;

                case SMyJsonToken::TokenType::TrueValue:
                    curNode.m_ptr = std::make_shared< CMyJsonNodeBool >( jtObjName.m_strValue, true );
                    bInvalid = !JsonAddNode( stack, curNode, jtObjName );
                    break;

                case SMyJsonToken::TokenType::FalseValue:
                    curNode.m_ptr = std::make_shared< CMyJsonNodeBool >( jtObjName.m_strValue, false );
                    bInvalid = !JsonAddNode( stack, curNode, jtObjName );
                    break;

                case SMyJsonToken::TokenType::IntValue:
                    curNode.m_ptr = std::make_shared< CMyJsonNodeInt >( jtObjName.m_strValue, strtoll( jt.m_strValue.c_str(), nullptr, 10 ));
                    bInvalid = !JsonAddNode( stack, curNode, jtObjName );
                    break;

                case SMyJsonToken::TokenType::DoubleValue:
                    curNode.m_ptr = std::make_shared< CMyJsonNodeDouble >( jtObjName.m_strValue, strtod( jt.m_strValue.c_str(), nullptr ));
                    bInvalid = !JsonAddNode( stack, curNode, jtObjName );
                    break;

                case SMyJsonToken::TokenType::StringValue:
                    curNode.m_ptr = std::make_shared< CMyJsonNodeString >( jtObjName.m_strValue, jt.m_strValue );
                    bInvalid = !JsonAddNode( stack, curNode, jtObjName );
                    break;

                case SMyJsonToken::TokenType::NewObject:
                    curNode.m_ptr = std::make_shared< CMyJsonNodeObject >( jtObjName.m_strValue );
                    bInvalid = !JsonAddNode( stack, curNode, jtObjName );
                    break;

                case SMyJsonToken::TokenType::EndObject:
                    curNode = JsonRmNode( stack, jtObjName );
                    bInvalid = (( !curNode.m_ptr ) || ( curNode.m_ptr->GetType() != CMyJsonNode::ValueType::Object ));
                    assert( bInvalid == false );    // @todo_llasek: DBG
                    break;

                case SMyJsonToken::TokenType::NewArray:
                    curNode.m_ptr = std::make_shared< CMyJsonNodeArray >( jtObjName.m_strValue );
                    bInvalid = !JsonAddNode( stack, curNode, jtObjName );
                    break;

                case SMyJsonToken::TokenType::EndArray:
                    curNode = JsonRmNode( stack, jtObjName );
                    bInvalid = (( !curNode.m_ptr ) || ( curNode.m_ptr->GetType() != CMyJsonNode::ValueType::Array ));
                    assert( bInvalid == false );    // @todo_llasek: DBG
                    break;

                case SMyJsonToken::TokenType::Comma:
                    // @todo_llasek: implement corner cases like: subsequent commas, start with comma
                    bInvalid = (( stack.empty()) ||
                        (( stack.top().m_ptr->GetType() != CMyJsonNode::ValueType::Object ) && ( stack.top().m_ptr->GetType() != CMyJsonNode::ValueType::Array )));
                    break;

                case SMyJsonToken::TokenType::Eof:
                    bInvalid = true;
                    break;

                case SMyJsonToken::TokenType::Invalid:
                default:
                    return SpJsonInvalid;
            }
        }

        if( bInvalid )
        {
            return my_shared_ptr< CMyJsonNode > {};
        }
        return curNode;
    }

protected:
    std::string_view m_strJson;
    uint32_t m_nJsonIdx;
};

my_shared_ptr< CMyJsonNode > CMyJsonNode::Parse( std::string_view a_strJson )
{
    CMyJsonParser jp( a_strJson );
    return jp.Parse();
}
