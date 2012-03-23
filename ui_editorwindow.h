/********************************************************************************
** Form generated from reading UI file 'editorwindow.ui'
**
** Created: Sun 18. Mar 21:36:55 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDITORWINDOW_H
#define UI_EDITORWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include <gui/codeedit.h>

QT_BEGIN_NAMESPACE

class Ui_EditorWindow
{
public:
    QAction *action_New;
    QAction *action_Save;
    QAction *actionSave_as;
    QAction *action_Open;
    QAction *action_Quit;
    QAction *action_Run_File;
    QAction *action_Inspect_Selection;
    QAction *action_Terminate_Execution;
    QAction *actionClear;
    QAction *actionLaTex;
    QWidget *centralwidget;
    QGridLayout *gridLayout_2;
    CodeEdit *editor;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuInterpreter;
    QMenu *menuConsole;
    QMenu *menuExport;
    QStatusBar *statusbar;
    QToolBar *toolBar;
    QDockWidget *consoleDock;
    QWidget *dockWidgetContents;
    QGridLayout *gridLayout;
    QTextEdit *console;

    void setupUi(QMainWindow *EditorWindow)
    {
        if (EditorWindow->objectName().isEmpty())
            EditorWindow->setObjectName(QString::fromUtf8("EditorWindow"));
        EditorWindow->resize(800, 600);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/pimii/pimii.png"), QSize(), QIcon::Normal, QIcon::Off);
        EditorWindow->setWindowIcon(icon);
        action_New = new QAction(EditorWindow);
        action_New->setObjectName(QString::fromUtf8("action_New"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/pimii/document_new.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_New->setIcon(icon1);
        action_Save = new QAction(EditorWindow);
        action_Save->setObjectName(QString::fromUtf8("action_Save"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/pimii/floppy_disk.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_Save->setIcon(icon2);
        actionSave_as = new QAction(EditorWindow);
        actionSave_as->setObjectName(QString::fromUtf8("actionSave_as"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/pimii/save_as.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_as->setIcon(icon3);
        action_Open = new QAction(EditorWindow);
        action_Open->setObjectName(QString::fromUtf8("action_Open"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/pimii/folder_document.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_Open->setIcon(icon4);
        action_Quit = new QAction(EditorWindow);
        action_Quit->setObjectName(QString::fromUtf8("action_Quit"));
        action_Run_File = new QAction(EditorWindow);
        action_Run_File->setObjectName(QString::fromUtf8("action_Run_File"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/pimii/gears_run.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_Run_File->setIcon(icon5);
        action_Inspect_Selection = new QAction(EditorWindow);
        action_Inspect_Selection->setObjectName(QString::fromUtf8("action_Inspect_Selection"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/pimii/gears_view.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_Inspect_Selection->setIcon(icon6);
        action_Terminate_Execution = new QAction(EditorWindow);
        action_Terminate_Execution->setObjectName(QString::fromUtf8("action_Terminate_Execution"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/pimii/gears_stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        action_Terminate_Execution->setIcon(icon7);
        actionClear = new QAction(EditorWindow);
        actionClear->setObjectName(QString::fromUtf8("actionClear"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/pimii/window_black.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionClear->setIcon(icon8);
        actionLaTex = new QAction(EditorWindow);
        actionLaTex->setObjectName(QString::fromUtf8("actionLaTex"));
        centralwidget = new QWidget(EditorWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout_2 = new QGridLayout(centralwidget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        editor = new CodeEdit(centralwidget);
        editor->setObjectName(QString::fromUtf8("editor"));
        QFont font;
        font.setFamily(QString::fromUtf8("Consolas"));
        font.setPointSize(10);
        editor->setFont(font);

        gridLayout_2->addWidget(editor, 0, 0, 1, 1);

        EditorWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(EditorWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 21));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuInterpreter = new QMenu(menubar);
        menuInterpreter->setObjectName(QString::fromUtf8("menuInterpreter"));
        menuConsole = new QMenu(menubar);
        menuConsole->setObjectName(QString::fromUtf8("menuConsole"));
        menuExport = new QMenu(menubar);
        menuExport->setObjectName(QString::fromUtf8("menuExport"));
        EditorWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(EditorWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        EditorWindow->setStatusBar(statusbar);
        toolBar = new QToolBar(EditorWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        EditorWindow->addToolBar(Qt::TopToolBarArea, toolBar);
        consoleDock = new QDockWidget(EditorWindow);
        consoleDock->setObjectName(QString::fromUtf8("consoleDock"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(consoleDock->sizePolicy().hasHeightForWidth());
        consoleDock->setSizePolicy(sizePolicy);
        consoleDock->setMinimumSize(QSize(274, 150));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(dockWidgetContents->sizePolicy().hasHeightForWidth());
        dockWidgetContents->setSizePolicy(sizePolicy1);
        gridLayout = new QGridLayout(dockWidgetContents);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        console = new QTextEdit(dockWidgetContents);
        console->setObjectName(QString::fromUtf8("console"));
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush1(QColor(255, 255, 255, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush1);
        palette.setBrush(QPalette::Active, QPalette::Light, brush1);
        palette.setBrush(QPalette::Active, QPalette::Midlight, brush1);
        QBrush brush2(QColor(127, 127, 127, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Dark, brush2);
        QBrush brush3(QColor(170, 170, 170, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Mid, brush3);
        QBrush brush4(QColor(239, 239, 239, 255));
        brush4.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Text, brush4);
        palette.setBrush(QPalette::Active, QPalette::BrightText, brush1);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        QBrush brush5(QColor(65, 65, 65, 255));
        brush5.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush5);
        QBrush brush6(QColor(98, 98, 98, 255));
        brush6.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Window, brush6);
        palette.setBrush(QPalette::Active, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Active, QPalette::AlternateBase, brush1);
        QBrush brush7(QColor(207, 207, 207, 255));
        brush7.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::NoRole, brush7);
        QBrush brush8(QColor(255, 255, 220, 255));
        brush8.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::ToolTipBase, brush8);
        palette.setBrush(QPalette::Active, QPalette::ToolTipText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Light, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Midlight, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Dark, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::Mid, brush3);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette.setBrush(QPalette::Inactive, QPalette::BrightText, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        QBrush brush9(QColor(126, 126, 126, 255));
        brush9.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush9);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush6);
        palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::NoRole, brush7);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush8);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Light, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Midlight, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Dark, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::Mid, brush3);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::BrightText, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush6);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush6);
        palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::NoRole, brush7);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush8);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush);
        console->setPalette(palette);
        console->setFont(font);

        gridLayout->addWidget(console, 0, 0, 1, 1);

        consoleDock->setWidget(dockWidgetContents);
        EditorWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(8), consoleDock);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuInterpreter->menuAction());
        menubar->addAction(menuConsole->menuAction());
        menubar->addAction(menuExport->menuAction());
        menuFile->addAction(action_New);
        menuFile->addSeparator();
        menuFile->addAction(action_Open);
        menuFile->addSeparator();
        menuFile->addAction(action_Save);
        menuFile->addAction(actionSave_as);
        menuFile->addAction(action_Quit);
        menuInterpreter->addAction(action_Run_File);
        menuInterpreter->addAction(action_Inspect_Selection);
        menuInterpreter->addAction(action_Terminate_Execution);
        menuConsole->addAction(actionClear);
        menuExport->addAction(actionLaTex);
        toolBar->addAction(action_New);
        toolBar->addAction(action_Save);
        toolBar->addAction(actionSave_as);
        toolBar->addAction(action_Open);
        toolBar->addSeparator();
        toolBar->addAction(action_Run_File);
        toolBar->addAction(action_Inspect_Selection);
        toolBar->addAction(action_Terminate_Execution);
        toolBar->addSeparator();
        toolBar->addAction(actionClear);

        retranslateUi(EditorWindow);

        QMetaObject::connectSlotsByName(EditorWindow);
    } // setupUi

    void retranslateUi(QMainWindow *EditorWindow)
    {
        EditorWindow->setWindowTitle(QApplication::translate("EditorWindow", "pimii", 0, QApplication::UnicodeUTF8));
        action_New->setText(QApplication::translate("EditorWindow", "&New", 0, QApplication::UnicodeUTF8));
        action_Save->setText(QApplication::translate("EditorWindow", "&Save", 0, QApplication::UnicodeUTF8));
        action_Save->setShortcut(QApplication::translate("EditorWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        actionSave_as->setText(QApplication::translate("EditorWindow", "Save &As", 0, QApplication::UnicodeUTF8));
        action_Open->setText(QApplication::translate("EditorWindow", "&Open", 0, QApplication::UnicodeUTF8));
        action_Open->setShortcut(QApplication::translate("EditorWindow", "F2", 0, QApplication::UnicodeUTF8));
        action_Quit->setText(QApplication::translate("EditorWindow", "&Quit", 0, QApplication::UnicodeUTF8));
        action_Run_File->setText(QApplication::translate("EditorWindow", "&Run File", 0, QApplication::UnicodeUTF8));
        action_Run_File->setShortcut(QApplication::translate("EditorWindow", "F5", 0, QApplication::UnicodeUTF8));
        action_Inspect_Selection->setText(QApplication::translate("EditorWindow", "&Inspect Selection", 0, QApplication::UnicodeUTF8));
        action_Inspect_Selection->setShortcut(QApplication::translate("EditorWindow", "F6", 0, QApplication::UnicodeUTF8));
        action_Terminate_Execution->setText(QApplication::translate("EditorWindow", "&Terminate Execution", 0, QApplication::UnicodeUTF8));
        action_Terminate_Execution->setShortcut(QApplication::translate("EditorWindow", "F7", 0, QApplication::UnicodeUTF8));
        actionClear->setText(QApplication::translate("EditorWindow", "Clear", 0, QApplication::UnicodeUTF8));
        actionClear->setShortcut(QApplication::translate("EditorWindow", "F8", 0, QApplication::UnicodeUTF8));
        actionLaTex->setText(QApplication::translate("EditorWindow", "LaTex", 0, QApplication::UnicodeUTF8));
        actionLaTex->setShortcut(QApplication::translate("EditorWindow", "Ctrl+L", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("EditorWindow", "File", 0, QApplication::UnicodeUTF8));
        menuInterpreter->setTitle(QApplication::translate("EditorWindow", "Interpreter", 0, QApplication::UnicodeUTF8));
        menuConsole->setTitle(QApplication::translate("EditorWindow", "Console", 0, QApplication::UnicodeUTF8));
        menuExport->setTitle(QApplication::translate("EditorWindow", "Export", 0, QApplication::UnicodeUTF8));
        toolBar->setWindowTitle(QApplication::translate("EditorWindow", "toolBar", 0, QApplication::UnicodeUTF8));
        consoleDock->setWindowTitle(QApplication::translate("EditorWindow", "Console", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class EditorWindow: public Ui_EditorWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDITORWINDOW_H
