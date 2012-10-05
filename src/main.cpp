#include "application.hpp"
#include <libjson.h>
#include <iostream>

int main(int argc, char * argv[])
{
  Application app(argc, argv);
  return app.start();
}
