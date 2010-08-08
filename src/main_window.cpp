#include "main_window.hpp"
#include <QVBoxLayout>
#include <QTabWidget>
#include "session_tab.hpp"
#include <QWebView>

MainWindow::MainWindow(Application* a)
    : QWidget(), app(a)
{
  resize(800, 600);
  setWindowTitle("Revisor Window");

  QVBoxLayout* layout = new QVBoxLayout;
  tabWidget = new QTabWidget(this);
  setLayout(layout);
  layout->addWidget(tabWidget);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  event->ignore();
}

void MainWindow::updateVisibility()
{
  setVisible(tabs.size() != 0);
}

void MainWindow::registerSessionTab(SessionTab* t)
{
  if (!tabs.contains(t->getFullName())) {
    tabs.insert(t->getFullName(), t);
    tabWidget->addTab(t->getWebView(), t->getFullName());

    connect(t,    SIGNAL(titleChanged(const QString&)),
            this, SLOT(updateTabTitle(const QString&)));

    updateVisibility();
  }
}

void MainWindow::unregisterSessionTab(SessionTab* tab)
{
  if (tabs.contains(tab->getFullName())) {
    tabWidget->removeTab(tabWidget->indexOf(tab->getWebView()));
    tabs.remove(tab->getFullName());
    updateVisibility();
  }
}

void MainWindow::raiseSessionTab(SessionTab* tab)
{
  if (tabs.contains(tab->getFullName())) {
    raise();
    tabWidget->setCurrentWidget(tab->getWebView());
  }
}

void MainWindow::updateTabTitle(const QString& newTitle)
{
  SessionTab* s = qobject_cast<SessionTab*>(sender());
  int idx = tabWidget->indexOf(s->getWebView());
  tabWidget->setTabText(idx, newTitle);
}
