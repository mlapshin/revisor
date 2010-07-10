#include "web_page.hpp"
#include <QDebug>

WebPage::WebPage(QObject* p)
    : QWebPage(p)
{

}

void WebPage::javaScriptAlert(QWebFrame* frame, const QString& msg)
{
  qDebug() << "JS Alert: " << msg;
}

bool WebPage::javaScriptConfirm(QWebFrame* frame, const QString& msg)
{
  return confirmAnswer;
}

void WebPage::javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID)
{
  qDebug() << "JS Console: " << message;
}

bool WebPage::javaScriptPrompt(QWebFrame* frame, const QString& msg, const QString& defaultValue, QString* result)
{
  *result = promptAnswer;
  return promptCancelled;
}
