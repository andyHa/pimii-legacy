#include "webwindow.h"
#include "ui_webwindow.h"

WebWindow::WebWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WebWindow)
{
    ui->setupUi(this);
}

QWebView* WebWindow::getWebView() {
    return ui->webView;
}

WebWindow::~WebWindow()
{
    delete ui;
}
