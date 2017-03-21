/*

    Copyright (C) 2016,2017 Tadeusz Puźniakowski

    This file is part of cpphttpsrv-app.

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

/* This file is an example usage of Http class */

#include <iostream>
#include <sstream>
#include <string>
#include <http.hpp>
#include <dhandler.hpp>
#include <unistd.h>
#include <stdlib.h>


using namespace puzniakowski;
using namespace std;

int main ( int argc, char **argv ) {

	Http srv( "localhost", 8090, true );
	auto sfh = getStaticFileHandler( "www/" );

	srv.get( "/", [&sfh]( Request &req, Response &res )->void {
		req.path = "/index.html";
		sfh( req, res );
	} );

	srv.get( "/.*csp", getDynamicFileHandler("www/",1,converters::dynamicPartConverterCsp,"include/cpphttpsrv", "./build") );
	srv.post( "/.*csp", getDynamicFileHandler("www/",1,converters::dynamicPartConverterCsp,"include/cpphttpsrv", "./build") );

	srv.get( "/.*cpp", getDynamicFileHandler("rest/",1,converters::dynamicPartConverterCpp, "include/cpphttpsrv" , "./build"));
	srv.post( "/.*cpp", getDynamicFileHandler("rest/",1,converters::dynamicPartConverterCpp, "include/cpphttpsrv" , "./build") );
	srv.get( "/.*", sfh );

	while ( 1 ) {
		auto ret = srv.acceptConnection();
	}
	return 0;
}
