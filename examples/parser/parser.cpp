/**
 * JSON parser example
 * (c) 2023 Łukasz Łasek
 */
#include <iostream>
#include <string_view>

#include "myjson.h"

static std::string_view Sg_strJson1[] = {
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

void Json1Print( my_shared_ptr< CMyJsonNode > a_json )
{
    if( !a_json )
    {
        std::cout << "invalid json\n";
        return;
    }

    std::cout << (int)(*a_json)[ "key4" ].GetType() << '\n';
    std::cout << (int)(*a_json)[ 3 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key4" ][ 0 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key4" ][ 1 ].GetType() << '\n';

    std::cout << (std::string_view)(*a_json)[ "key4" ][ 5 ][ "key10" ] << '\n';
    std::cout << (int)(*a_json)[ "key4" ][ 5 ][ 1 ] << '\n';
    std::cout << (std::string_view)(*a_json)[ "key4" ][ 5 ][ 2 ] << '\n';

    std::cout << (int)(*a_json)[ "key4" ][ 2 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key4" ][ 3 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key4" ][ 4 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key4" ][ 5 ].GetType() << '\n';

    std::cout << (int)(*a_json)[ "key_json" ].GetType() << '\n';
    std::cout << (std::string_view)(*a_json)[ "key_json" ] << '\n';


    std::cout << "--- Parse the inner json ---\n";
    auto jn = CMyJsonNode::Parse( (std::string_view)(*a_json)[ "key_json" ]);
    std::cout << (std::string_view)(*jn)[ "keyA" ] << '\n';
    std::cout << (std::string_view)(*jn)[ "keyB" ][ 0 ] << '\n';
}

void Json2Print( my_shared_ptr< CMyJsonNode > a_json )
{
    if( !a_json )
    {
        std::cout << "invalid json\n";
        return;
    }

    std::cout << (int)(*a_json)[ "key40" ].GetType() << '\n';
    std::cout << (int)(*a_json)[ 3 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key40" ][ 0 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key40" ][ 1 ].GetType() << '\n';

    std::cout << (std::string_view)(*a_json)[ "key40" ][ 5 ][ "key20" ] << '\n';
    std::cout << (int)(*a_json)[ "key40" ][ 5 ][ 1 ] << '\n';
    std::cout << (std::string_view)(*a_json)[ "key40" ][ 5 ][ 2 ] << '\n';

    std::cout << (int)(*a_json)[ "key40" ][ 2 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key40" ][ 3 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key40" ][ 4 ].GetType() << '\n';
    std::cout << (int)(*a_json)[ "key40" ][ 5 ].GetType() << '\n';

    std::cout << (int)(*a_json)[ "key_json2" ].GetType() << '\n';
    std::cout << (std::string_view)(*a_json)[ "key_json2" ] << '\n';


    std::cout << "--- Parse the inner json ---\n";
    auto jn = CMyJsonNode::Parse( (std::string_view)(*a_json)[ "key_json2" ]);
    std::cout << (std::string_view)(*jn)[ "keyC" ] << '\n';
    std::cout << (std::string_view)(*jn)[ "keyD" ][ 0 ] << '\n';
}

void Parse1()
{
    std::cout << "--- Parse1() ---\n";
    std::cout << "--- First json ---\n";

    std::string strJson( Sg_strJson1[ 0 ]);
    size_t nJsonLen = sizeof( Sg_strJson1 ) / sizeof( Sg_strJson1[ 0 ]);
    for( size_t idx = 1; idx < nJsonLen; idx++ )
    {
        strJson += Sg_strJson1[ idx ];
    }

    auto json = CMyJsonNode::Parse( strJson );

    Json1Print( json );

}

void Parse2()
{
    std::cout << "--- Parse2() ---\n";
    std::cout << "--- First json ---\n";

    size_t nJsonLen = sizeof( Sg_strJson1 ) / sizeof( Sg_strJson1[ 0 ]);
    size_t nJsonLn = 0;
    auto json = CMyJsonNode::Parse(
        [ &nJsonLn, nJsonLen ]()
        {
            return nJsonLn < nJsonLen ? std::string( Sg_strJson1[ nJsonLn++ ]) : std::string();
        });

    std::cout << "--- parsed lines: " << nJsonLn << " ---\n";
    Json1Print( json );

    json = CMyJsonNode::Parse(
        [ &nJsonLn, nJsonLen ]()
        {
            return nJsonLn < nJsonLen ? std::string( Sg_strJson1[ nJsonLn++ ]) : std::string();
        });
    
    std::cout << "--- Next json ---\n";
    Json2Print( json );
}

int main( int argc, const char *argv[] )
{
    Parse1();
    Parse2();
    return 0;
}
