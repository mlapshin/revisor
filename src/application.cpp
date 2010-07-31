#include "application.hpp"
#include "http_server.hpp"
#include "dispatcher.hpp"
#include "session.hpp"
#include <QDebug>
#include "exception.hpp"
#include <QNetworkDiskCache>

Application::Application(int _argc, char** _argv)
    : QApplication(_argc, _argv)
{
  listeningInterface = "127.0.0.1";
  portNumber = 8080;
  argc = _argc;
  argv = _argv;
  setQuitOnLastWindowClosed(false);
}

int Application::start()
{
  if (initArgtable()) {
    dispatcher = new Dispatcher(this);
    httpServer = new HttpServer(listeningInterface, portNumber, this, dispatcher);

    httpCache = new QNetworkDiskCache(this);
    httpCache->setCacheDirectory("/tmp/revisor_cache");

    return exec();
  } else {
    return 0;
  }
}

bool Application::initArgtable()
{
  const char* progname = "revisor";
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

  if (l->count > 0) {
    listeningInterface = l->sval[0];
  }

  if (p->count > 0) {
    portNumber = *p->ival;
  }

  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
  return true;
}

Application::~Application()
{
  for(SessionsMap::iterator i = sessions.begin(); i != sessions.end(); i++) {
    delete *i;
  }

  delete httpCache;
  httpCache = 0;

  sessions.clear();
}

Session* Application::startSession(const QString& name)
{
  if (sessions.contains(name)) {
    throw Exception(QString("Session with name '%1' already exists").arg(name));
  } else {
    Session* s = new Session(this, name);
    sessions.insert(name, s);
    return s;
  }
}

void Application::stopSession(const QString& name)
{
  if (sessions.contains(name)) {
    Session* s = sessions[name];
    sessions.remove(name);
    delete s;
  } else {
    throw Exception(QString("Session with name '%1' does not exists").arg(name));
  }
}

Session* Application::getSession(const QString& name)
{
  return sessions.contains(name) ? sessions[name] : 0;
}
