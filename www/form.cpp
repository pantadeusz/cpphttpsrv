#include <iostream>

t_requHandler getHandler() {
  return []( Request &req, Response &res )->void {
	  std::string b = "<!DOCTYPE html>\
		<html>\
		<body>\
		<form action='form.cpp' method='post'>\
		First name:<br>\
		<input type='text' name='firstname' value='"+req.params["firstname"]+"'>\
		<br>\
		Last name:<br>\
		<input type='text' name='lastname' value='"+req.params["lastname"]+"'>\
		<br><br>\
		<input type='submit' value='Submit'>\
		</form> \
    <h3>Values form POST:</h3> \
		<ul>";
		for(auto i = req.params.begin(); i != req.params.end(); i++) {
        b = b + "<li>" + i->first + "=" + i->second + "</li>";
    }
    b = b + "</ul></body></html>";
    res.getWriter() <<  b ;
  };
}
