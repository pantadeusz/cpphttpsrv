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
#include "hhelpers.hpp"


#include <sys/stat.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iostream>

namespace puzniakowski {

std::vector<char> base64tobin(const std::string &s) {
  std::vector<char> ret;
  ret.reserve(s.length());
  for (int i = s.find(',') + 1; i < s.size() - 3; i += 4) {
    unsigned int val = 0;
    int pads = 0;
    int v = 0;
    for (int j = 0; j < 4; j++) {
      int c = s[i + j];
      v = 0;
      if ((c >= 'A') && (c <= 'Z'))
        v = c - 'A';
      else if ((c >= 'a') && (c <= 'z'))
        v = c - 'a' + 26;
      else if ((c >= '0') && (c <= '9'))
        v = c - '0' + 52;
      else if (c == '+')
        v = 62;
      else if (c == '/')
        v = 63;
      else if (c == '=')
        pads++;
      val = (val << 6) + v;
    }
    ret.push_back((val >> 16) & 0x0ff);
    if (pads < 2) ret.push_back((val >> 8) & 0x0ff);
    if (pads <= 3) ret.push_back((val >> 0) & 0x0ff);
  }
  return ret;
}

std::string filenameProtect(const std::string &s) {
  std::string projname = s;
  if (projname[0] == '/') projname = projname.substr(1);
  for (std::size_t found = projname.find("../"); found != std::string::npos;
        found = projname.find("../", found)) {
    projname[found] = '_';
    projname[found + 1] = '_';
    projname[found + 2] = '_';
  }
  return projname;
}


unsigned long int getFileModTime(const std::string &fname) {
	struct stat sb;
	stat(fname.c_str(), &sb);
	return sb.st_mtime;
}



// after http://stackoverflow.com/questions/236129/split-a-string-in-c
std::vector<std::string> &split( const std::string &s, const char delim, std::vector<std::string> &elems ) {
	std::stringstream ss( s );
	std::string item;
	while ( std::getline( ss, item, delim ) ) {
		elems.push_back( item );
	}
	return elems;
}
// after http://stackoverflow.com/questions/236129/split-a-string-in-c
std::vector<std::string> split( const std::string &s, char delim ) {
	std::vector<std::string> elems;
	split( s, delim, elems );
	return elems;
}

// after http://stackoverflow.com/
std::string trim( const std::string& str,
                  const std::string& whitespace ) {
	const auto strBegin = str.find_first_not_of( whitespace );
	if ( strBegin == std::string::npos )
		return ""; // no content
	const auto strEnd = str.find_last_not_of( whitespace );
	const auto strRange = strEnd - strBegin + 1;
	return str.substr( strBegin, strRange );
}


std::function<void( const std::string & )> errlog = [](const std::string &s) {
	std::cout << "ERROR: " << s << std::endl;
};

std::function<void( const std::string & )> stdlog = [](const std::string &s) {
	std::cout << "LOG: " << s << std::endl;
};


}
