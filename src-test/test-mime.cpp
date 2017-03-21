#include "lest.hpp"
#include <mime.hpp>

using namespace std;

const lest::test specification[] =
{
    CASE( "Testing of getMimeType" )
    {
        EXPECT( "image/png" == getMimeType("test1.png") );
        EXPECT( "image/png" == getMimeType("test1.PNG") );
        EXPECT( "application/octet-stream" == getMimeType("test1") );
    },
};

int main( int argc, char * argv[] )
{
    return lest::run( specification, argc, argv );
}
