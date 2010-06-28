#include "application.hpp"
#include "http_server.hpp"
#include "dispatcher.hpp"
#include "session.hpp"

Application::Application(int argc, char** argv)
    : QApplication(argc, argv)
{
  setQuitOnLastWindowClosed(false);
  dispatcher = new Dispatcher(this);
  httpServer = new HttpServer(8080, this, dispatcher);
}

Application::~Application()
{
  for(QList<Session*>::iterator i = sessions.begin(); i != sessions.end(); i++) {
    delete *i;
  }
}

int Application::startSession()
{
  Session* s = new Session(this);
  sessions.append(s);
  return sessions.length() - 1;
}

void Application::stopSession(int i)
{
  if (i >= 0 && i < sessions.length()) {
    Session* s = sessions[i];
    sessions.removeAt(i);
    delete s;
  }
}

Session* Application::getSession(int i)
{
  return sessions[i];
}
