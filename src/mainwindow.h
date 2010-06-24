#include <QtGui>

class QWebView;
QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    void log(const QString& msg);

protected slots:

    void adjustTitle();
    void setProgress(int p);
    void finishLoading(bool);

private:
    QWebView *view;
    QListWidget *logList;
    int progress;
};
