# cpphttpsrv

![travis ci](https://travis-ci.org/pantadeusz/cpphttpsrv.svg?branch=master "travis results")

### by Tadeusz Puźniakowski, 2016

Very simple implementation of HTTP server allowing you to write your code in C++11.
The intention of this project is to provide lightweight server for 
Internet of Things applications, where there is need to access hardware
directly via C or C++ code.

## sample usage (in C++11 code)

It uses lambdas for request handling. It is asynchronous, so it can handle
multiple request in  parallel. 

```c++
Http srv( "localhost", 8090, true );
auto sfh = getStaticFileHandler( "www/" );

srv.get( "/", [&sfh]( Request &req, Response &res )->void {
	req.path = "/index.html";
	sfh( req, res );
} );

srv.get( "/counter", [&]( Request &req, Response &res )->void {
	auto &session = req.getSession( true );
	if ( session.count( "counter" ) <= 0 ) session["counter"] = "-1"; // new counter
	session["counter"] = std::to_string( std::stoi( session["counter"].c_str() )+1 );
	res.getWriter() << "Counter " << session["counter"] << ";\n";
} );

// handling dynamic pages from www directory.- GET method
srv.get( "/.*csp", getDynamicFileHandler("www/",1,converters::dynamicPartConverterCsp) );
// handling dynamic pages from www directory. - POST method
srv.post( "/.*csp", getDynamicFileHandler("www/",1,converters::dynamicPartConverterCsp) );
// handling static pages from www directory.- GET method
srv.get( "/.*", sfh );

while ( 1 ) {
	auto ret = srv.acceptConnection();
}
```

## dynamic pages

The server can handle dynamic pages written in CppServerPages (CSP) that allow for writing dynamic pages in JSP fashion, see:

```html
<%@args -fopenmp %>
<%! int i = 0; %>
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>cpphttpsrv - the demo page</title>
<link rel="stylesheet" href="style.css">
</head>

<body>
<h2>The sample CSP page.</h2>

<% 
auto &session = req.getSession( true );
if ( session.count( "counter" ) <= 0 ) 
session["counter"] = "-1"; // new counter
session["counter"] = std::to_string( std::stoi( session["counter"].c_str() )+1 );
i++; 
%>
<p>
Hello World, the counter is:  <%= i %>
</p>
<p>
Session counter is: <%= session["counter"] %>
</p>

</body>

</html> 
```
you can use it in the following way:
```c++
srv.get( "/.*csp", getDynamicFileHandler("www/",1,converters::dynamicPartConverterCsp) );
```


## Application Server Seed

There is ready to use application seed in directory:  [server-seed](https://bitbucket.org/t4deusz/cpphttpsrv/src/tip/server-seed/)

you can compile it with:

```
make
```

and run by:

```
./start.sh
```



### donations

I accept donations in BTC at 13CfATaTXvh5aVn4cfcgGHEo67rtuwVqnF
