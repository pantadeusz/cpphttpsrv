#include "lest.hpp"
#include <http.hpp>

using namespace std;
using namespace puzniakowski;

const lest::test specification[] =
{
    CASE( "Testing of Http::uridecode" )
    {
        EXPECT( "test1" == Http::uridecode("test1") );
        EXPECT( "tes t1" == Http::uridecode("tes+t1") );
        EXPECT( "~!@#$%^&*()" == Http::uridecode("~!%40%23%24%25%5E%26*()") );
        EXPECT( "" == Http::uridecode("") );
    },
    CASE( "Testing of std::map<std::string, std::string> Http::decodeContentType( const std::string & contentType, const std::vector<char> &uristr )" )
    {
		std::map<std::string, std::string> kvr = Http::decodeContentType("application/x-www-form-urlencoded","jan=JAN&nowak=NOWAK");
        EXPECT( "JAN" == kvr["jan"] );
        EXPECT( "NOWAK" == kvr["nowak"] );
        EXPECT( "" == kvr["other"] );
    },
};

int main( int argc, char * argv[] )
{
    return lest::run( specification, argc, argv );
}
