#include "application.hpp"
#include "http_server.hpp"

Application::Application(int argc, char** argv)
    : QApplication(argc, argv)
{
  httpServer = new HttpServer(8080, this);
  connect(httpServer, SIGNAL(commandReceived(const QString&)), this, SLOT(commandReceived(const QString&)));
}

void Application::commandReceived(const QString& cmd)
{
  printf("Command received\n");
}
