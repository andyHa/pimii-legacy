/********************************************************************************
** Form generated from reading UI file 'webwindow.ui'
**
** Created: Sun 25. Dec 16:41:18 2011
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WEBWINDOW_H
#define UI_WEBWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include <QtWebKit/QWebView>

QT_BEGIN_NAMESPACE

class Ui_WebWindow
{
public:
    QWidget *centralwidget;
    QWebView *webView;

    void setupUi(QMainWindow *WebWindow)
    {
        if (WebWindow->objectName().isEmpty())
            WebWindow->setObjectName(QString::fromUtf8("WebWindow"));
        WebWindow->resize(800, 600);
        centralwidget = new QWidget(WebWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        webView = new QWebView(centralwidget);
        webView->setObjectName(QString::fromUtf8("webView"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(webView->sizePolicy().hasHeightForWidth());
        webView->setSizePolicy(sizePolicy);
        webView->setUrl(QUrl("about:blank"));
        WebWindow->setCentralWidget(centralwidget);

        retranslateUi(WebWindow);

        QMetaObject::connectSlotsByName(WebWindow);
    } // setupUi

    void retranslateUi(QMainWindow *WebWindow)
    {
        WebWindow->setWindowTitle(QApplication::translate("WebWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class WebWindow: public Ui_WebWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WEBWINDOW_H
