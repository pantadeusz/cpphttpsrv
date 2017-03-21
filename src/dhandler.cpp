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
#include "dhandler.hpp"
#include "http.hpp"

#include <sys/types.h>

#include <sys/stat.h>

#include <stdexcept>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <regex>
#include <dlfcn.h>
#include <fstream>
#include <tuple>
#include <utility>

using namespace std;

namespace puzniakowski {

t_requHandler &DynamicCppHandler::getHandler() {
	lock_guard<mutex> guard( dyn_mutex );
	return loadedHandlers[filename];
}

namespace converters {

	// args + libs, rawcontent
	// TODO: make it work fast
	std::function < std::pair<std::string, std::string> (const std::string &) > dynamicPartConverterCpp = [](const std::string &rawFile) -> std::pair<std::string, std::string> {
		std::string ret = "";
		std::string args = "";
		for (int i = 0; i < rawFile.length(); i++) {
			std::string line = "";
			for (int j = i; j < rawFile.length() && rawFile[j] != '\n'; ++j,i=j) {
				line = line + rawFile[j];
			}
			if (line.length() > 5 && line[0] == '@'
					&& line[1] == 'a'
					&& line[2] == 'r'
					&& line[3] == 'g'
					&& line[4] == 's') {
					args = line.substr(5,line.length());
			} else {
				ret = ret + line + "\n";
			}
		}
		return std::make_pair (args,ret);
	};
	std::function < std::pair<std::string, std::string> (const std::string &) > dynamicPartConverterCsp = [](const std::string &cphfile) -> std::pair<std::string, std::string> {
		std::string ret = "";
		std::string args = "";
		// file loaded
		// <%  %> -- raw commands
		// <%@args %> -- compilation arguments  
		// <%=  %> -- value to put  
		
		// <%!  %> -- commands before start (like #include )
		// <\% -- copy this as <%
		// inside <% %>   \%> -- this is not finish, translates to %>
		
		string currentDirective = "";
		string cppargs = "";
		string beforeall = "#include <sstream>\n";
		string rawString = "t_requHandler getHandler() { return []( Request &req, Response &res )->void { std::ostream &__ret = res.getWriter();\n__ret << \""; // to jest wnetrze funkcji
		for (int i = 0; i < cphfile.size(); i++) {
			int left=i, right=i;
			string toAdd = string() + cphfile[i];
			if (cphfile[i] == '\"') {
				toAdd = "\\\"";
			} else if (cphfile[i] == '\n') {
				toAdd = "\\n\"\n        <<\"";
			} else if (i < cphfile.length()-4) {
				if ((cphfile[i] == '<') && (cphfile[i+1] == '%')) {
					string dynpart = "";
					for (int j = i+2; j < cphfile.size()-2; ++j, i=j) {
						if ((cphfile[j] == '%') && (cphfile[j+1] == '>')) {
							if (cphfile[j-1] == '\\') {
								j+=2;
							} else {
								left+=2;
								right = j;
								i = j+1;
								
								dynpart = cphfile.substr(left,right-left);
								if (dynpart[0] == '=') { // value here
									toAdd = "\" << (" + dynpart.substr(1) +") << \"";
								} else if (dynpart[0] == '!') { // before everything
									beforeall = beforeall + dynpart.substr(1,dynpart.length()) + "\n";
									toAdd = "";
								} else if (dynpart[0] == '@') { // @args command line arguments for compiler
									cppargs = cppargs + dynpart.substr(5,dynpart.length()) + " ";
									toAdd = "";
								} else {
									toAdd = "\";\n"+dynpart+"\n __ret << \"";
								}
								break; //j = cphfile.size();
							}
						}
					}
				} else if ((cphfile[i] == '<') && (cphfile[i+2] == '%') && (cphfile[i+1] == '\\')) {
					toAdd = "<%"; i+=2;
				}
			}
			rawString += toAdd; // dodajemy zwykly tekst
		}
		rawString += "\";\n";
		rawString += "    \n};\n}";
		rawString = beforeall + "\n// cpp args: " + cppargs + "\n" + rawString;
		
		
		//std::cout << rawString;

		return std::make_pair (cppargs,rawString);
	};
}

//function < pair<string, string> (const std::string &) > DynamicCppHandler::defaultSourceConverter = converters::dynamicPartConverterCpp; 

DynamicCppHandler::DynamicCppHandler(const string &fname, function < pair<string, string> (const std::string &) > converter, std::string headersPath, std::string buildPath) {
	lock_guard<mutex> guard( dyn_mutex );
	filename = fname;
	void *fLib;
	// prepare file names
	int lastDirChar = fname.rfind('/');
	lastDirChar = (lastDirChar == string::npos)?0:(lastDirChar+1);
	string fnamebare = fname.substr(lastDirChar);
	string dirname = buildPath + "/" + fname.substr(0,(lastDirChar>0)?(lastDirChar-1):0);
	string srcdirname = fname.substr(0,(lastDirChar>0)?(lastDirChar-1):0);
	string sourcefilename = dirname + "/" + fnamebare + ".cpp";
	string libfilename = dirname + "/lib" + fnamebare + "." + VER + ".so";
	// check cache
	if (loadedLibs.count(fname) > 0) {
		auto srcTime = getFileModTime(fname);
		auto libTime = getFileModTime(libfilename);
		if (srcTime >= libTime) {
			stdlog("Cache invalidated for file: " + fname + "; recompiling");
			loadedHandlers.erase(fname);
			auto l = loadedLibs[fname];
			loadedLibs.erase(fname);
			dlclose(l);
		}
	}

	// check if not loaded
	if (loadedLibs.count(fname) > 0) {
		fLib = loadedLibs[fname];
	} else {
		ifstream t(fname);
		string functions((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
		if (t.is_open() && functions.length() > 5) {
			bool recompile = true;
			// jesli taki plik istnieje
			if (ifstream(libfilename).is_open()) {
				recompile = false; // jesli cache jest z biblioteka
				auto srcTime = getFileModTime(fname);
				auto libTime = getFileModTime(libfilename);
				if (srcTime >= libTime) {
					recompile = true; // jesli cache jest nieaktualny
				}
			}
			if (recompile) {
				// prepare directories

				for (int i = 0; i < dirname.length(); i++) if (dirname[i] == '/') {
						mkdir( dirname.substr(0,i).c_str(), 0700 );
				}
				mkdir( dirname.c_str(), 0700 );
				ofstream f ( sourcefilename );
				std::string args, srcs;
				std::tie<string,string>(args,srcs) = converter(functions);
				f << "#include <http.hpp>\n";
				f << "#include <iostream>\n";
				f << "#define VERSION \"" << VER << "\"\n";
				f << "using namespace puzniakowski;\n";
				f << srcs; // function from file
				f << "extern \"C\"\n";
				f << "t_requHandler F() {\n";
				f << "return getHandler();\n";
				f << "}\n";
				f.close();

				std::string compilercommand = string("") + "g++ -I"
					+ headersPath
					+ " -I"
					+ srcdirname
					+ " -std=c++11 -fPIC -shared " 
					+ sourcefilename
					+ " " + args + " " 
					+ " -o " 
					+ libfilename;
				std::cout << compilercommand << "\n";
				int r = system ( (compilercommand).c_str());
				if (r != 0) throw runtime_error( "Compilation errors" );
			}
			fLib = dlopen ( ( libfilename ).c_str(), RTLD_LAZY );
			if ( !fLib ) {
				errlog("Error loading lib: " + std::string(dlerror()));
			} else {
				loadedLibs[fname] = fLib;
				void *gptr = dlsym ( fLib, "F" );
				t_requHandler (*fn)(void) = reinterpret_cast<t_requHandler (*)(void)> ( gptr );
				if ( fn ) {
					loadedHandlers[filename] = fn();
				} else {
					throw runtime_error(  "FATAL ERROR! Could not load F function!" );
				}
			}
		} else {
			errlog("File not found: " + fname);
			throw runtime_error( "ERROR");
		}
	}
	if ( fLib == NULL ) {
		throw runtime_error( "ERROR" );
	}
}

DynamicCppHandler::~DynamicCppHandler() {
}

map < string, void * > DynamicCppHandler::loadedLibs;
map < string, t_requHandler > DynamicCppHandler::loadedHandlers;
mutex DynamicCppHandler::dyn_mutex;


t_requHandler getDynamicFileHandler(const string sprefix, 
		int subStrIgnore, 
		std::function < std::pair<std::string, std::string> (const std::string &) > converter,
		std::string headersPath,
		std::string buildPath) {
	return [=]( Request &req, Response &res )->void {
	try {
		string path = req.path.substr( subStrIgnore );
		if (path.length() <= 0) path = "./";
		if (path[path.length()-1] == '/') path = path + "index.cpp";
		string fname = sprefix + path; // obcinamy katalog glowny
		DynamicCppHandler dfh(fname, converter, headersPath,buildPath);
		dfh.getHandler()(req,res);
	} catch (exception& e) {
//		auto sfh = getStaticFileHandler( sprefix, subStrIgnore);
//		sfh( req,res );
		res.getWriter() << "Error loading file: " << req.path.substr( subStrIgnore ) << "\nException: "
		<< e.what() ;
	}
 };
}



}
