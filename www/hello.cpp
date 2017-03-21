t_requHandler getHandler() {
  return []( Request &req, Response &res )->void {
    res.getWriter() << "Hello World!" ;
  };
}
