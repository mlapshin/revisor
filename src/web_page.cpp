#include "web_page.hpp"
#include <QWebFrame>
#include <QPainter>
#include <QDebug>
#include <QWebElement>

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
  return !promptCancelled;
}

void WebPage::saveScreenshot(const QString& fileName, const QSize& vpSize)
{
  QSize originalVpSize = viewportSize();
  QWebFrame* frame = mainFrame();

  if (!vpSize.isNull()) {
    setViewportSize(vpSize);
  }

  QImage image(frame->contentsSize(), QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);

  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);
  painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
  frame->documentElement().render(&painter);
  painter.end();

  image.save(fileName);

  setViewportSize(originalVpSize);
}
