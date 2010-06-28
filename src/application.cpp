#include "application.hpp"
#include "http_server.hpp"
#include "dispatcher.hpp"
#include "session.hpp"

Application::Application(int argc, char** argv)
    : QApplication(argc, argv)
{
  dispatcher = new Dispatcher(this);
  httpServer = new HttpServer(8080, this, dispatcher);
  Session* s = new Session(this);
}
