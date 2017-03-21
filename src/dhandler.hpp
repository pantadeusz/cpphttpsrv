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
#ifndef __DHANDLER_HPP___
#define __DHANDLER_HPP___

#include "http.hpp"
#include <functional>
#include <string>
#include <map>

namespace puzniakowski {

// default converter for source files, it should generate
// C++11 source code that will contain
//
// t_requHandler getHandler() {
//  return []( Request &req, Response &res )->void {
//	  res.setResponse( "" );
//  };
//}


namespace converters {
	// input raw data
	extern std::function < std::pair<std::string, std::string> (const std::string &) > dynamicPartConverterCpp;
	// input CSP file (see documentation about csp)
	extern std::function < std::pair<std::string, std::string> (const std::string &) > dynamicPartConverterCsp;
}

typedef class DynamicCppHandler {
  private:
  	static std::map < std::string, void * > loadedLibs;
  	static std::map < std::string, t_requHandler > loadedHandlers;
  	std::string filename;
  	static std::mutex dyn_mutex;
  public:
	//static std::function < std::pair<std::string, std::string> (const std::string &) > defaultSourceConverter; 
  	t_requHandler &getHandler();
  	DynamicCppHandler(const std::string &fname, 
		std::function < std::pair<std::string, std::string> (const std::string &) > converter = converters::dynamicPartConverterCpp,
		std::string headersPath = "./src", 
		std::string buildPaht = "./build");
  	virtual ~DynamicCppHandler();
  } DynamicCppHandler;



/**
 * Generate dynamic file handler - it first checks if it is possible to compile given file, then
 * gets handler from it. In case of error, it redirects to static page.
 */
t_requHandler getDynamicFileHandler(const std::string sprefix = "", int subStrIgnore = 1,
	std::function < std::pair<std::string, std::string> (const std::string &) > handler = converters::dynamicPartConverterCpp,
		std::string headersPath = "./src",
		std::string buildPath = "./build");

}

#endif
