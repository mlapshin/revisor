#include <QtGui>
#include <QtWebKit>
#include <QListWidget>
#include "mainwindow.h"

MainWindow::MainWindow()
{
  progress = 0;

  QNetworkProxyFactory::setUseSystemConfiguration(true);

  view = new QWebView(this);
  view->load(QUrl("http://www.google.com/ncr"));
  connect(view, SIGNAL(titleChanged(QString)), SLOT(adjustTitle()));
  connect(view, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
  connect(view, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));

  QDockWidget *logListDockWidget = new QDockWidget("Log", this);
  logListDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
  logList = new QListWidget(logListDockWidget);
  logListDockWidget->setWidget(logList);

  addDockWidget(Qt::BottomDockWidgetArea, logListDockWidget);
  setCentralWidget(view);
}

void MainWindow::adjustTitle()
{
  if (progress <= 0 || progress >= 100) {
    setWindowTitle(view->title());
  } else {
    setWindowTitle(QString("%1 (%2%)").arg(view->title()).arg(progress));
  }
}

void MainWindow::setProgress(int p)
{
  progress = p;
  adjustTitle();
}

void MainWindow::finishLoading(bool)
{
  progress = 100;
  adjustTitle();
}

void MainWindow::log(const QString& msg)
{
  new QListWidgetItem(msg, logList);
}
