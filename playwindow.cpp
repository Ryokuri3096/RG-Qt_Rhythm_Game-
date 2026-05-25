#include "playwindow.h"
#include "ui_playwindow.h"

PlayWindow::PlayWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PlayWindow)
{
    ui->setupUi(this);
}

void PlayWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F11) {
        if (isFullScreen())
            showNormal();
        else
            showFullScreen();
    }
    QMainWindow::keyPressEvent(event);
}

PlayWindow::~PlayWindow()
{
    delete ui;
}
