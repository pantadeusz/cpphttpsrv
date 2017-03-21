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
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <streambuf>

using namespace std;

std::string loadFile(const std::string &ifname) {
		std::ifstream t(ifname);
		std::string str;

		t.seekg(0, std::ios::end);   
		str.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
		return str;
}


int main(int argc, char **argv) {
	if (argc > 2) {
		string ifname(argv[1]),ofname(argv[2]);
		

		string cphfile = loadFile(ifname);
		// file loaded
		// <%  %> -- raw commands
		// <%@args %> -- compilation arguments  
		// <%=  %> -- value to put  
		
		// <%!  %> -- commands before start (like #include )
		// <\% -- copy this as <%
		// inside <% %>   \%> -- this is not finish, translates to %>
		
//		vector<string> headers;
		string currentDirective = "";
		string cppargs = "";
		string beforeall = "#include <sstream>\n";
		string rawString = "t_requHandler getHandler() { return []( Request &req, Response &res )->void { std::stringstream __ret;\n__ret << \""; // to jest wnetrze funkcji
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
		rawString += "    res.setResponse( __ret.str() );\n};\n}";
		rawString = beforeall + "\n// cpp args: " + cppargs + "\n" + rawString;
		
		
		std::cout << rawString;
	} else {
		cout << "Podaj nazwy plikow" << endl;
	}
	
	return 0;
}
