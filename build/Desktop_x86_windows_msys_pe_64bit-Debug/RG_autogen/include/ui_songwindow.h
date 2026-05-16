/********************************************************************************
** Form generated from reading UI file 'songwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SONGWINDOW_H
#define UI_SONGWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SongWindow
{
public:
    QPushButton *backButton;
    QScrollArea *chooseArea;
    QWidget *scrollAreaWidgetContents;

    void setupUi(QWidget *SongWindow)
    {
        if (SongWindow->objectName().isEmpty())
            SongWindow->setObjectName("SongWindow");
        SongWindow->resize(1280, 720);
        SongWindow->setStyleSheet(QString::fromUtf8(""));
        backButton = new QPushButton(SongWindow);
        backButton->setObjectName("backButton");
        backButton->setGeometry(QRect(10, 10, 150, 50));
        chooseArea = new QScrollArea(SongWindow);
        chooseArea->setObjectName("chooseArea");
        chooseArea->setGeometry(QRect(740, 30, 501, 651));
        chooseArea->setAutoFillBackground(true);
        chooseArea->setStyleSheet(QString::fromUtf8("border: 1px solid #888888;\n"
"border-radius: 6px;"));
        chooseArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 499, 649));
        chooseArea->setWidget(scrollAreaWidgetContents);

        retranslateUi(SongWindow);

        QMetaObject::connectSlotsByName(SongWindow);
    } // setupUi

    void retranslateUi(QWidget *SongWindow)
    {
        SongWindow->setWindowTitle(QCoreApplication::translate("SongWindow", "Form", nullptr));
        backButton->setText(QCoreApplication::translate("SongWindow", "Back", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SongWindow: public Ui_SongWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SONGWINDOW_H
