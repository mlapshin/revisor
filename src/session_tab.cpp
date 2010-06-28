#include "session_tab.hpp"
#include "session.hpp"

SessionTab::SessionTab(Session* s)
    : QObject(s)
{
  networkManager = new QNetworkAccessManager(this);
  networkManager->setCookieJar(session->getNetworkCookieJar());
}
