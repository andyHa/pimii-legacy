#ifndef WEBWINDOW_H
#define WEBWINDOW_H

#include <QMainWindow>
#include <QtWebKit>

namespace Ui {
    class WebWindow;
}

class WebWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit WebWindow(QWidget *parent = 0);
    ~WebWindow();

    QWebView* getWebView();

private:
    Ui::WebWindow *ui;
};

#endif // WEBWINDOW_H
