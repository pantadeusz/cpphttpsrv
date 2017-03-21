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

#include <http.hpp>
#include <hhelpers.hpp>


#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>

#include <stdexcept>
#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <map>
#include <utility>
#include <vector>
#include <sstream>

#include <regex>
#include <fstream>

#include "mime.hpp"

using namespace std;
namespace puzniakowski {


t_sessionMemStorage &Request::getSession( bool createIfNotFound ) {
	if ( session != NULL ) return *session;
	return http->getSession(this->sessionId);
}

const std::string &Request::getSessionId() {
	http->getSession(this->sessionId);
	return this->sessionId;
};
void Request::setSessionId( const std::string &sId ) {
	sessionId = sId;
	session = &(http->getSession(this->sessionId));
};

Request::Request() {
	session = NULL;
	http = NULL;
	contentLength = 0;
	mime = "application/x-www-form-urlencoded";
}
Request::Request( Http *h ) {
	session = NULL;
	http = h;
}

Response::Response() {
	code = 200;
	codeComment = "ok";
	mime = "text/html";
	charset = "utf8";
}

Response::~Response() {
}

std::string Http::uridecode( const std::string &uristr ) {
	std::string retstr;
	retstr.reserve(uristr.length()*2);
	for ( int i = 0; i < uristr.length(); i++ ) {
		if ( uristr[i] == '%' && i < ( uristr.length()-2 ) ) {
			char num[3];
			num[0] = ( char )uristr[i+1];
			num[1] = ( char )uristr[i+2];
			num[2] = 0;
			char c = strtoul( num, NULL, 16 );
			retstr = retstr + c;
			i+= 2;
		} else if ( uristr[i] == '+' ) {
			retstr = retstr + ' ';
		} else {
			retstr = retstr + uristr[i];
		}
	}
	return retstr;
}

std::map<std::string, std::string> Http::decodeContentType( const std::string & contentType, const std::string &uristr ) {
	std::vector<char> us;
	us.reserve(uristr.length());
	for (auto c : uristr) us.push_back(c);
	return decodeContentType(contentType, us);
}
std::map<std::string, std::string> Http::decodeContentType( const std::string & contentType, const std::vector<char> &uristr ) {
	std::map<std::string, std::string> kvr;
	if (contentType == "application/x-www-form-urlencoded") {
		std::string s;
		std::vector<std::string> kv;
		kv.reserve(256);
		for (int i = 0; i < uristr.size(); i++) {
			if ((uristr[i] == '=') && (kv.size() % 2 == 0)) {
				kv.push_back(uridecode(s));
				s = "";
			} else if ((uristr[i] == '&') && (kv.size() % 2 == 1)) {
				kv.push_back(uridecode(s));
				s = "";
			} else s = s + uristr[i];
		}
		if(kv.size()%2) kv.push_back(uridecode(s));
		for (int i = 0; i < kv.size(); i+=2) {
			kvr[kv[i]] = kv[i+1];
		}
	} else {
//	if (contentType == "") { // assume JSON
		kvr["jsonstring"] = kvr[""] = std::string(uristr.begin(), uristr.end());
	}
	return kvr;
}


void Http::sigchld_handler( int s ) {
	while( waitpid( -1, NULL, WNOHANG ) > 0 );
}

void Http::get( const std::string &mapping, t_requHandler f ) {
	mappingS["GET"].push_back( mapping );
	mappingF["GET"].push_back( f );
}
void Http::post( const std::string &mapping, t_requHandler f ) {
	mappingS["POST"].push_back( mapping );
	mappingF["POST"].push_back( f );
}

int Http::acceptConnection() {
	auto now = std::chrono::system_clock::now(); // set current time
	Request req; // zapytanie
	req.http = this;
	req.remoteAddrSize = sizeof( struct sockaddr_in );
	if ( ( req.s = accept( sockfd, ( struct sockaddr * )&req.remoteAddr, &req.remoteAddrSize ) ) == -1 ) {
		perror( "accept: " );
		return -1;
	}

	auto doResponse = [&,req, now]() {
		Request requ = req;
		Response resp; // odpowiedz
		
		t_requHandler handler = notFoundHandler;

		requ.rawRequest.reserve( 2048 );
		char byteRead[1024];
		int r, posZero = 4;
		bool dobreak = false;
		while ( (!dobreak) ) {
			if (( r=read( requ.s, byteRead, 512 ) ) > 0) {
				byteRead[r] = 0;
				requ.rawRequest.insert (requ.rawRequest.end(), byteRead, byteRead+r);
				for (int i = posZero; (i < requ.rawRequest.size()); i++) {
					if ( ( requ.rawRequest[i-4] == 13 ) && ( requ.rawRequest[i-3] == 10 )
						&& ( requ.rawRequest[i-2] == 13 ) && ( requ.rawRequest[i-1] == 10 ) ){
						dobreak = true;
						requ.content.insert(requ.content.end(),&(requ.rawRequest[i-0]), &(requ.rawRequest.back())+1);
						break;
					}
				}
				posZero = requ.rawRequest.size();
			}
			if (r < 512) dobreak = true;
		}
		if ( requ.rawRequest.size() <= 0 ) {
			errlog( "got empty string '" + std::to_string(requ.rawRequest.size()) + "'; r = " + std::to_string( r ));
			perror( "read failed" );
			close( requ.s );
			return -1;
		}
		std::vector<std::string> lines;
		{
			std::string l = "";
			for (int i = 0; i < requ.rawRequest.size(); i++) {
				auto c = requ.rawRequest[i];
				if (c == '\n') { //l = l + c;
				}else if (c == '\r'){
					if (l.length() > 0) lines.push_back(l);
					l = "";
				} else l = l + c;
			}
			if (l.length() > 0) lines.push_back(l);
		}
		if ( lines.size() <= 0 ) {
			errlog("ERROR: got empty data");
			return -1;
		}
		std::stringstream ss( trim( lines[0] ) );
		ss >> requ.method >> requ.queryString >> requ.proto;

		stdlog("REQUEST: " + requ.method  + " " + requ.queryString + " " + requ.proto);
		for ( int i = 1; i < lines.size(); i++ ) {
			std::stringstream ss( trim( lines[i] ) );
			std::string header;
			ss >> header;
			if ( header == std::string( "Host:" ) ) {
				ss >> requ.hostname;
			} else if ( header == std::string( "Cookie:" ) ) {
				std::string sessionString;
				{
				auto s = lines[i];
				bool found = false;
				size_t p;
				while ((p = s.find("tphttp=")) != string::npos) {
					string ssid = s.substr(p+7,s.substr(p+7).find(";"));
					sessionString = ssid;
					if (sessions.count(sessionString) > 0) {
						found = true;
						break;
					}
					s = s.substr(p+7);
				}
				if (found && sessionString.size() == 0) {
					getSession(sessionString);
				}
				}
				if (sessionString.size() > 2) {
					requ.setSessionId( sessionString );
				} 
			} else if (header == "Content-Length:") {
				ss >> requ.contentLength;
				if (req.contentLength > 1024*1024*256) {
					errlog( "req.contentLength > 1024*1024*256");
					return -1;
				}
			} else if (header == "Content-Type:") {
				ss >> requ.mime;
				for (int i = 0; i < requ.mime.length(); i++) {
					if (requ.mime[i] == ';') {requ.mime = requ.mime.substr(0,i); break;}
				}
			}
		}
		// read the contents of the request
		for (int i = requ.content.size(); i < requ.contentLength && (( r=read( requ.s, byteRead, min((size_t)1024,requ.contentLength-i) ) ) > 0 ); i+=r) {
			requ.content.insert(requ.content.end(),byteRead, byteRead+r);
		}
		auto idx = requ.queryString.find('?');
		if (idx == std::string::npos) requ.path = requ.queryString;
		else requ.path = requ.queryString.substr(0, idx);
		if (requ.path.length() <= 0) requ.path = "/";
		// find matching path
		for ( int i = 0; i < mappingS[requ.method].size(); i++ ) {
			if ( std::regex_match ( requ.path, std::regex( mappingS[requ.method][i] ) ) ) {
				handler = mappingF[requ.method][i]; // assign handler
				break;
			}
		}
		if (idx != std::string::npos) {
			requ.params = decodeContentType("application/x-www-form-urlencoded", requ.queryString.substr(idx+1,requ.queryString.rfind('#')));
		}
		if (requ.mime == "application/x-www-form-urlencoded") {
			auto nrparams = decodeContentType(requ.mime, requ.content );
			for (auto it=nrparams.begin(); it!=nrparams.end(); ++it)
				requ.params[it->first] = it->second;
		} else { // TODO: unsupported mime - assume JSON string
			auto nrparams = decodeContentType(requ.mime, requ.content );
			for (auto it=nrparams.begin(); it!=nrparams.end(); ++it)
				requ.params[it->first] = it->second;
                }
		std::stringstream responseStream;
		resp.getWriter = [&]()->std::ostream&{
				return responseStream;
		};
		handler( requ, resp ); ///< actual response prepare
		
		std::string sent;
		sent.reserve(1024*2048);
		sent = "HTTP/1.1 " + std::to_string( resp.code ) + " - " + resp.codeComment
		                   + "\r\nServer: PuzHttp " + VER //std::to_string(VER)
		                   + "\r\nContent-Length: " + std::to_string( responseStream.tellp() )
		                   + "\r\nConnection: close\nContent-Type: "	+ resp.mime
		                   + "; charset=" + resp.charset;
		if (requ.getSessionId().length() > 0) sent = sent + "\r\nSet-Cookie: sessiontphttp=" + requ.getSessionId();
		sent = sent + "\r\n\r\n";
		send( requ.s, sent.c_str(), sent.length(), MSG_NOSIGNAL );
		//send( requ.s, &resp.response[0], resp.response.size(), MSG_NOSIGNAL );
		send( requ.s, responseStream.str().c_str(), responseStream.tellp(), MSG_NOSIGNAL );
		close( requ.s );
		return 0;
	};



	if ( async == 0 ) return doResponse();
	else {
		try {
			workers.push_back( std::async( std::launch::async, doResponse ) );
		} catch (...) {
			errlog("could not create worker");
			return doResponse();
		}
			for ( auto it = workers.begin(); it != workers.end(); ) {
				auto status = ( *it ).wait_for( std::chrono::milliseconds( 0 ) );
				if ( status == std::future_status::ready ) {
					auto result = ( *it ).get(); /// TODO: getting result - this is not really useful
					it = workers.erase( it );
				} else {
					++it;
				}
			}
	}

	return 0;
}


t_sessionMemStorage &Http::getSession( std::string &sessionId ) {
	std::lock_guard<std::mutex> guard( get_sessions_mutex );
	if(( sessions.count(sessionId) == 0 ) || (sessionId.size()==0)) {
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		if (sessionId.size() <= 1) {
			sessionId = std::to_string( rand() );
			while ( sessions.count( sessionId = std::to_string( rand() ) )  ); // TODO: better session number generator
		}
		sessions[sessionId] = t_sessionMemStorage ();
		sessionsTimes[sessionId] = now;
		// jesli nowa sesja, to sprawdzmy, czy aby nie mamy wiszacych sesji:
		std::vector < std::string > sessionsToDel;
		for ( auto & e: sessionsTimes ) {
			auto ft = e.second;
			auto dt = std::chrono::duration_cast<std::chrono::seconds>( now - ft ).count();
			if ( dt > 60*60*24 ) { // sesja jest aktywna 24h, chyba ze nastapi zapytanie z ciastkiem
				sessionsToDel.push_back( e.first );
			}
		}
		std::for_each( sessionsToDel.begin(), sessionsToDel.end(), [&]( std::string k ) {
			sessionsTimes.erase( k );
			sessions.erase( k );
		} );
		stdlog("new session: " + sessionId + "; session count: " + std::to_string(sessions.size()) + ";");
		return sessions[sessionId];
	} else {
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		sessionsTimes[sessionId] = now; // aktualizacja czasu sesji
		return sessions[sessionId];
	}
}

Http::Http( std::string hostname, int port, int async ) {
	int yes=1;
	struct sockaddr_in my_addr;
	this->async = async;
	this->port = port;
	mappingS["GET"] = std::vector<std::string>();
	mappingF["GET"] = std::vector < t_requHandler >();
	mappingS["POST"] = std::vector<std::string>();
	mappingF["POST"] = std::vector < t_requHandler >();
	if ( ( sockfd = socket( PF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		perror( "socket" );
		exit( 1 );
	}

	if ( setsockopt( sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof( int ) ) == -1 ) {
		perror( "setsockopt" );
		exit( 1 );
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons( port );
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset( &( my_addr.sin_zero ), '\0', 8 );

	if ( bind( sockfd, ( struct sockaddr * )&my_addr, sizeof( struct sockaddr ) ) == -1 ) {
		perror( "bind" );
		exit( 1 );
	}

	if ( listen( sockfd, 32 ) == -1 ) {
		perror( "listen" );
		exit( 1 );
	}

	sa.sa_handler = sigchld_handler;
	sigemptyset( &sa.sa_mask );
	sa.sa_flags = SA_RESTART;
	if ( sigaction( SIGCHLD, &sa, NULL ) == -1 ) {
		perror( "sigaction" );
		exit( 1 );
	}
	signal( SIGPIPE, SIG_IGN );
	workers.reserve(128);
	stdlog("Web server at port: " + std::to_string(port));
}
Http::~Http () {
	close( sockfd );
	stdlog("finished webapp");
}

t_requHandler getStaticFileHandler( const std::string sprefix, int subStrIgnore ) {
	return [=]( Request &req, Response &res )->void {
		std::string path = req.path.substr( subStrIgnore );
		if (path.length() <= 0) path = "./";
		if (path[path.length()-1] == '/') path = path + "index.html";
		std::string fname = sprefix + path; // obcinamy katalog glowny
		std::ifstream f( fname, std::ios::binary );
		try {
			if ( f.is_open() ) {
				res.getWriter() << f.rdbuf();
				res.mime = getMimeType(path);
			} else {
				Http::notFoundHandler(req,res);
			}
		} catch (const std::ios_base::failure& e) {
			Http::notFoundHandler(req,res);
		}
	};
}



t_requHandler Http::notFoundHandler = []( Request &req, Response &res )->void {
	res.getWriter() << std::string("Page not found: ") << req.path ;
	res.code = 404;
	res.codeComment = "not found";
};


}
