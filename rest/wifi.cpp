/*

    Copyright (C) 2016 Tadeusz Puźniakowski

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

/* This is example CRUD for WiFi list in Json file */



#include <iostream>
#include <sstream>
#include <map>
#include <fstream>
#include <regex>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>

#include <json11/json11.hpp>

using namespace json11;

// exec command, and return string from the standard output
// after: http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix#478960
std::string exec(const char* cmd) {
    char buffer[128];
    std::stringstream result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("Cannot run command");
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result << buffer;
    }
    return result.str();
}

Json getAll() {
	// todo: get detected wifis:
	
	
    std::string jsonData = "{wifis:[]}";
	std::string err;
	std::ifstream f( "setup/wpasupplicant.json", std::ios::binary );
	if ( f.is_open() ) {
		std::vector<char> data;
		data.assign( std::istreambuf_iterator<char>( f ), std::istreambuf_iterator<char>() );
		jsonData = std::string ( data.begin(), data.end() );
	} else {
		jsonData = "{\"wifi\":[]}";
	}
	auto j = Json::parse(jsonData, err);
	if (err.length() > 0) throw std::runtime_error(err);
	return j;
}

void setAll(const Json &json) {
	std::ofstream output("setup/wpasupplicant.json", std::ios::binary );
	if (output) {
		output << json.dump();
	}
	else {
		throw std::runtime_error("Could not save file");
	}
}

Json getSsid(const std::string &ssid) {
        int found = 0;
        std::string err;
        auto json = getAll();
        for (auto &k : json["wifis"].array_items()) {
            if (ssid == k["wifi"]["ssid"].string_value() ) {
                return k;
            }
        }
        json = Json::parse("{\"wifi\":{}}", err);
		if (err.length() > 0) throw std::runtime_error(err);
		return json;
}

Json setSsid(const Json &w) {
		auto json = getAll();
		auto o = Json::object { };
        std::vector < Json > wifis;
		int found = 0;
		wifis.insert(wifis.end(), json["wifis"].array_items().begin(), json["wifis"].array_items().end());
        for (auto &k : json["wifis"].array_items()) {
            if (w["wifi"]["ssid"].string_value() == k["wifi"]["ssid"].string_value() ) {
                wifis[found] = w;
                o["wifis"] = Json(wifis);
                json = o;
                return o;
            }
            found++;
        }
		wifis.push_back(w);
        o["wifis"] = Json(wifis);
        setAll(o);
		return o;
}

Json delSsid(const Json &w) {
		auto json = getAll();
		auto o = Json::object { };
        std::vector < Json > wifis;
		int found = 0;
		// create array of wifis
		wifis.insert(wifis.end(), json["wifis"].array_items().begin(), json["wifis"].array_items().end());
        for (auto &k : json["wifis"].array_items()) {
			// if the ssid is correct, then delete
            if (w["wifi"]["ssid"].string_value() == k["wifi"]["ssid"].string_value() ) {
				wifis[found] = wifis.back();
				wifis.pop_back();
                o["wifis"] = Json(wifis);
                json = o;
                setAll(o);
                return o;
            }
            found++;
        }
        o["wifis"] = Json(wifis);
		return o;
}

// supports: get, post
// get /wifi/wifis -- get all wifis
// get /wifi/wifis/[ssid] -- get selected wifi by ssid
// post  /wifi/wifis/[ssid] -- set selected wifi, for example: {"wifi": {"psk": "somepasswd2", "ssid": "sn"}}
// post  /wifi/wifis/[ssid] -- delete selected wifi if action==delete, for example: {"wifi": {"psk": "somepasswd2", "ssid": "sn","action":"delete"}}

t_requHandler getHandler() {
  return []( Request &req, Response &res )->void {
    std::stringstream out( "" );
    std::string err;
    std::string jsonData = "{wifis:[]}";
    res.mime = "application/json";
	if (req.path == "/wifi/wifis") {
		auto json = getAll();
        res.getWriter() << json.dump(); //out.str() );
    } else if (std::regex_match ( req.path, std::regex( "/wifi/wifis/.*" ) ) && req.method == "GET") {
        res.getWriter() <<  getSsid(req.path.substr(req.path.find_last_of("/")+1)).dump() ;
    } else if (std::regex_match ( req.path, std::regex( "/wifi/wifis/.*" ) ) && req.method == "POST") {
		auto e = Json::parse(req.params["jsonstring"], err);
		if (e["wifi"]["ssid"].string_value() !=  req.path.substr(req.path.find_last_of("/")+1)) {
			std::cout << "ERROR: json wrong: " << e.dump() << "\n";
			throw std::runtime_error(e.dump() + "||||" + req.path);
		}
		if (Json::parse(req.params["jsonstring"], err)["wifi"]["action"].dump() == "\"delete\"") {
			//,"action":"delete"
			auto o = delSsid(e);				
			res.getWriter() <<  o.dump();
		} else {
			auto e = Json::parse(req.params["jsonstring"], err);
			auto o = setSsid(e);
			res.getWriter() << o.dump() ; 
		}
    } else {
        res.mime = "text/html";
        res.getWriter() << "<a href='/wifi/wifis'>jsonapi get all</a> <a href='/wifi/wifis/ssid'>jsonapi get selected by ssid</a>";
    }
    if (err.length() > 0) std::cout << err;
  };
}
