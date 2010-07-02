#include "application.hpp"
#include "http_server.hpp"
#include "dispatcher.hpp"
#include "session.hpp"
#include <QDebug>

Application::Application(int _argc, char** _argv)
    : QApplication(_argc, _argv)
{
  argc = _argc;
  argv = _argv;
  setQuitOnLastWindowClosed(false);
}

int Application::start() {
  if (initArgtable()) {
    dispatcher = new Dispatcher(this);
    httpServer = new HttpServer(8080, this, dispatcher);
    return exec();
  } else {
    return 0;
  }
}

bool Application::initArgtable()
{
  const char* progname = "revisor_server";
  struct arg_str* l   = arg_str0("l", NULL, "<interface>",   "Interface to listen, default '127.0.0.1'");
  struct arg_int* p   = arg_int0("p", NULL, "<port number>", "Port to listen, default 8080");
  struct arg_lit* h   = arg_lit0("h", "help",                "This help message");
  struct arg_end* end = arg_end(20);
  void *argtable[]  = {l, p, h, end};

  int nerrors = arg_parse(argc, argv, argtable);

  // special case: '--help' takes precedence over error reporting
  if (h->count > 0) {
    printf("Usage: %s", progname);
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    return false;
  }

  if (nerrors > 0) {
    arg_print_errors(stdout, end, progname);
    return false;
  }

  return true;
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
