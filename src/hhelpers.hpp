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
#ifndef ___HHELPERS_HPP___
#define ___HHELPERS_HPP___

#include <vector>
#include <string>
#include <functional>

namespace puzniakowski {


std::vector<char> base64tobin(const std::string &s);
std::string filenameProtect(const std::string &s);


  // after http://stackoverflow.com/questions/236129/split-a-string-in-c
  std::vector<std::string> &split( const std::string &s, const char delim, std::vector<std::string> &elems );

  // after http://stackoverflow.com/questions/236129/split-a-string-in-c
  std::vector<std::string> split( const std::string &s, char delim );

  // after http://stackoverflow.com/
  std::string trim( const std::string& str, const std::string& whitespace = "\n \t\r" );

  // returns file modification time
  unsigned long int getFileModTime(const std::string &fname);

  extern std::function<void( const std::string & )> errlog;
  extern std::function<void( const std::string & )> stdlog;

}

#endif
