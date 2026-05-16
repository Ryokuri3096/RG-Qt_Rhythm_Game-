/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *playButton;
    QPushButton *profileButton;
    QPushButton *settingsButton;
    QPushButton *exitButton;
    QStatusBar *statusbar;
    QMenuBar *menubar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1280, 720);
        MainWindow->setMinimumSize(QSize(1280, 720));
        MainWindow->setMaximumSize(QSize(1280, 720));
        MainWindow->setStyleSheet(QString::fromUtf8(""));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setMinimumSize(QSize(1280, 720));
        centralwidget->setMaximumSize(QSize(1280, 720));
        centralwidget->setStyleSheet(QString::fromUtf8("background-image:url(:/img/mainmenu.png);\n"
"background-repeat:no-repeat;\n"
"background-position:center;\n"
"border:none;"));
        playButton = new QPushButton(centralwidget);
        playButton->setObjectName("playButton");
        playButton->setGeometry(QRect(600, 40, 641, 311));
        playButton->setStyleSheet(QString::fromUtf8("border: none;\n"
"background-image: url(:/img/Play.png);\n"
"background-repeat: no-repeat;\n"
"background-position: center;"));
        playButton->setIconSize(QSize(641, 311));
        profileButton = new QPushButton(centralwidget);
        profileButton->setObjectName("profileButton");
        profileButton->setGeometry(QRect(600, 390, 312, 214));
        profileButton->setStyleSheet(QString::fromUtf8("background-image:url(:/img/Profile.jpg);\n"
"background-repeat:no-repeat;\n"
"background-position:center;\n"
"border:none;"));
        settingsButton = new QPushButton(centralwidget);
        settingsButton->setObjectName("settingsButton");
        settingsButton->setGeometry(QRect(930, 390, 312, 214));
        settingsButton->setStyleSheet(QString::fromUtf8("background-image:url(:/img/Settings.jpg);\n"
"background-repeat:no-repeat;\n"
"background-position:center;\n"
"border:none;"));
        exitButton = new QPushButton(centralwidget);
        exitButton->setObjectName("exitButton");
        exitButton->setGeometry(QRect(1090, 620, 151, 61));
        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1280, 17));
        MainWindow->setMenuBar(menubar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        playButton->setText(QString());
        profileButton->setText(QString());
        settingsButton->setText(QString());
        exitButton->setText(QCoreApplication::translate("MainWindow", "ExitGame", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
