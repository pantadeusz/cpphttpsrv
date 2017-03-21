/*

    Copyright (C) 2016,2017 Tadeusz Puźniakowski

    This file is part of cpphttpsrv.

    cpphttpsrv is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    cpphttpsrv is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpphttpsrv; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Ten plik jest częścią cpphttpsrv.

    cpphttpsrv jest wolnym oprogramowaniem; możesz go rozprowadzać dalej
    i/lub modyfikować na warunkach Powszechnej Licencji Publicznej GNU,
    wydanej przez Fundację Wolnego Oprogramowania - według wersji 2 tej
    Licencji lub (według twojego wyboru) którejś z późniejszych wersji.

    Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
    użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
    gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
    ZASTOSOWAŃ. W celu uzyskania bliższych informacji sięgnij do
    Powszechnej Licencji Publicznej GNU.

    Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
    Powszechnej Licencji Publicznej GNU (GNU General Public License);
    jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
    Place, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef __PUZNIAKOWSKI_HTTP__
#define __PUZNIAKOWSKI_HTTP__

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <future>
#include <utility>

#include <netinet/in.h>
#include <signal.h>


namespace puzniakowski {

typedef std::map < std::string, std::string > t_sessionMemStorage;

class Http;

typedef class Request {
private:
	/**
	 *  Session variables
	 * */
	t_sessionMemStorage *session;
	std::string sessionId;

public:
	/**
	 * pointer to owning http object
	 */
	Http *http;

	std::string url;
	std::string path;
	std::string method;
	std::string proto;
	std::string queryString;
	std::string hostname;
	std::string mime;
	size_t contentLength;

	std::map < std::string, std::string > params;

	std::vector <char> rawRequest;
	std::vector <char> content;
	int s;
	struct sockaddr_in remoteAddr;
	socklen_t remoteAddrSize;

	/**
	 * Gets the session object
	 *
	 * @param createIfNotFound if session was not found, create new session. The session ID will be generated automatically.
	 * @return returns session object (map).
	 * */
	t_sessionMemStorage &getSession( bool createIfNotFound = true );
	/**
	 * returns raw session ID
	 * */
	const std::string &getSessionId();
	/**
	 * Sets new session ID.
	 * */
	void setSessionId( const std::string &sId );
	Request();
	Request( Http *h );

} Request;

typedef class Response {
	
public:
	
	int code;
	std::string codeComment;
	std::string charset;
	std::string mime;

	std::function<std::ostream&()> getWriter;
	Response();
	virtual ~Response();

} Response;

typedef std::function<void( Request &, Response & )> t_requHandler;




typedef class Http {
private:

	std::map < std::string, std::vector < t_requHandler > > mappingF;
	std::map < std::string, std::vector < std::string > > mappingS;

	/// tasks
	std::vector< std::future < int > > workers;

	///  session handling
	std::mutex sessions_mutex;
	std::mutex get_sessions_mutex;
	std::map < std::string, t_sessionMemStorage > sessions;
	std::map < std::string, std::chrono::system_clock::time_point > sessionsTimes;

	// accepting socket
	int sockfd;
	// current port
	int port;
	// is async
	int async;

	struct sigaction sa;

	static void sigchld_handler( int s );

public:

	static std::string uridecode( const std::string &uristr );
	static std::map<std::string, std::string> decodeContentType( const std::string & contentType, const std::string &uristr );
	static std::map<std::string, std::string> decodeContentType( const std::string & contentType, const std::vector<char> &uristr );

  /**
	 * adds new handler for url.
	 */
	void get( const std::string &mapping, t_requHandler f );
	void post( const std::string &mapping, t_requHandler f );
  /**
	 * function that accepts new connection. It also parses the request and dispatches new task to handle it
	 */
	int acceptConnection();

	/**
	 * returns the session for given id. If the session does not exist, then the new session can be created
	 */
	t_sessionMemStorage &getSession( std::string &sessionId );

 /**
  * constructor creates working server
	*/
	Http( std::string hostname = "localhost", int port = 8080, int async = false );
	virtual ~Http ();

	/**
	 * The default handler for 404 error
	 * */
	static t_requHandler notFoundHandler;



} Http;


/**
 * Get static file handler for serving static sites.
 *
 * @param prefix path to data
 * @param subStrIgnore how many character from Request::path to ignore (it helps in situation, when the
 *                     static page is served under some alias)
 * @return returns newly created t_requestHandler
 * */
t_requHandler getStaticFileHandler( const std::string sprefix = "", int subStrIgnore = 1 );



}


#endif
