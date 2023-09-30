/**
 * JSON parser example
 * (c) 2023 Łukasz Łasek
 */
#include <iostream>
#include <string_view>

#include "myjson.h"

static std::string_view SstrJson1(
""
""
"{"
"    \"key1\": \"value1\","
"    \"key2\": 123,"
"    \"key3\": true,"
"    \"key4\": [0, 1, \"aaa\", null, false, { \"key10\": \"value\\\\\\\"10\", 456, \"value11\" }],"
"    \"value5\","
"    false,"
"    \"key_json\": \"{\\\"keyA\\\": \\\"value\\A\\\", \\\"keyB\\\": [\\\"bbbb\\\"]}\""
"}"
" " );

int main( int argc, const char *argv[] )
{
    auto json = CMyJsonNode::Parse( SstrJson1 );

    if( !json.m_ptr )
    {
        std::cout << "invalid json\n";
        return -1;
    }

    std::cout << (int)(*(json.m_ptr))[ "key4" ].GetType() << '\n';
    std::cout << (int)(*(json.m_ptr))[ 3 ].GetType() << '\n';
    std::cout << (int)(*(json.m_ptr))[ "key4" ][ 0 ].GetType() << '\n';
    std::cout << (int)(*(json.m_ptr))[ "key4" ][ 1 ].GetType() << '\n';

    std::cout << (std::string_view)(*(json.m_ptr))[ "key4" ][ 5 ][ "key10" ] << '\n';
    std::cout << (int)(*(json.m_ptr))[ "key4" ][ 5 ][ 1 ] << '\n';
    std::cout << (std::string_view)(*(json.m_ptr))[ "key4" ][ 5 ][ 2 ] << '\n';

    std::cout << (int)(*(json.m_ptr))[ "key4" ][ 2 ].GetType() << '\n';
    std::cout << (int)(*(json.m_ptr))[ "key4" ][ 3 ].GetType() << '\n';
    std::cout << (int)(*(json.m_ptr))[ "key4" ][ 4 ].GetType() << '\n';
    std::cout << (int)(*(json.m_ptr))[ "key4" ][ 5 ].GetType() << '\n';

    std::cout << (int)(*(json.m_ptr))[ "key_json" ].GetType() << '\n';
    std::cout << ((std::string_view)(*json.m_ptr)[ "key_json" ]).data() << '\n';


    auto jn = CMyJsonNode::Parse( "{\"key1\": \"value1\", \"key2\": 123}" );
    std::cout << ((std::string_view)(*jn.m_ptr)[ "key1" ]).data();
    return 0;
}
