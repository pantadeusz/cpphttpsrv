#include <iostream>
#include <sstream>

using namespace std;

int setup () {
    cout << "\nInitialization of dynamic page\n";
    return 1;
}

int ready = setup();
int n = 0;

t_requHandler getHandler() {
  return []( Request &req, Response &res )->void {
    auto &s = req.getSession( true );
    s["counter"] = to_string(((s["counter"].length()>0)?stoi( s["counter"].c_str() ):-1)+1 );
    res.getWriter() << "Session counter " << s["counter"] << "; Application counter: " << n << ";\n";
    n++;
  };
}
