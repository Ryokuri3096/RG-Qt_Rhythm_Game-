#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "songwindow.h"
#include <QLayout>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    displayShadowForButtons(); //给按钮加上阴影
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_playButton_clicked()
{
    qDebug() << "playButton is clicked";

    this->hide(); //隐藏当前主窗口
    SongWindow* songWin=new SongWindow();

    songWin->setMainWindow(this); // 把主窗口指针传给选歌窗口
    songWin->setFixedSize(this->size()); //设置窗口大小与主窗口相同
    songWin->show();
}


void MainWindow::on_profileButton_clicked()
{
    qDebug() << "profileButton is clicked";
}


void MainWindow::on_settingsButton_clicked()
{
    qDebug() << "settingsButton is clicked";
}


void MainWindow::on_exitButton_clicked()
{
    qApp->exit();
}

void MainWindow::displayShadowForButtons(){ //给按钮加上阴影的函数
    QGraphicsDropShadowEffect *shadow[4];
    for(int i=0;i<4;i++){
        shadow[i] = new QGraphicsDropShadowEffect(this);
        shadow[i]->setBlurRadius(15);
        shadow[i]->setOffset(3, 3);
        shadow[i]->setColor(QColor(0, 0, 0, 180));
    }
    ui->playButton->setGraphicsEffect(shadow[0]);
    ui->profileButton->setGraphicsEffect(shadow[1]);
    ui->settingsButton->setGraphicsEffect(shadow[2]);
    ui->exitButton->setGraphicsEffect(shadow[3]);
}
