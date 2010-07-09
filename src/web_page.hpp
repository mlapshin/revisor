#ifndef _WEB_PAGE_H_
#define _WEB_PAGE_H_

#include <QWebPage>

class WebPage : public QWebPage
{
  Q_OBJECT;
 public:
  WebPage(QObject* parent = 0);

  inline void setConfirmAnswer(bool a) {
    confirmAnswer = a;
  }

  inline void setPromptCanceled(bool canceled) {
    promptCanceled = canceled;
  }

  inline void setPromptAnswer(const QString& a) {
    promptAnswer = a;
  }

 protected:
  // virtual bool acceptNavigationRequest  ( QWebFrame * frame, const QNetworkRequest & request, NavigationType type );
  // virtual QString chooseFile ( QWebFrame * parentFrame, const QString & suggestedFile );
  // virtual QObject* createPlugin ( const QString & classid, const QUrl & url, const QStringList & paramNames, const QStringList & paramValues );
  // virtual QWebPage* createWindow ( WebWindowType type );
  virtual void javaScriptAlert(QWebFrame* frame, const QString& msg);
  virtual bool javaScriptConfirm(QWebFrame* frame, const QString& msg);
  virtual void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);
  virtual bool javaScriptPrompt(QWebFrame* frame, const QString& msg, const QString& defaultValue, QString* result);
  // virtual QString userAgentForUrl ( const QUrl & url ) const;

 private:
  bool confirmAnswer;
  bool promptCanceled;
  QString promptAnswer;
};

#endif /* _WEB_PAGE_H_ */
