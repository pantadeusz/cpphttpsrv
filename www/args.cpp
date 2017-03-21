@args -lm


t_requHandler getHandler() {
  return []( Request &req, Response &res )->void {
    std::string b = "<html><body><h2>args.cpp</h2><p>";
    for(auto i = req.params.begin(); i != req.params.end(); i++) {
        b = b + i->first + "=" + i->second + "<br/>";
    }
    b = b + "</p> <body></html>";
    res.getWriter() << b;
  };
}
