#include "mainwindow.h"
#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include "./ui_mainwindow.h"
#include "profilewindow.h"  // 个人资料界面
#include "settingswindow.h"
#include "songwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    displayShadowForButtons(); //给按钮加上阴影

    // 让标签不拦截点击（点击穿透到下方按钮）
    ui->playLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->profileLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->settingsLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 初始化按钮音效（不受sfxVolume控制）
    m_click1Sfx = new QSoundEffect(this);
    m_click1Sfx->setSource(QUrl("qrc:/sfx/click1.wav"));
    m_click3Sfx = new QSoundEffect(this);
    m_click3Sfx->setSource(QUrl("qrc:/sfx/click3.wav"));

    // 4个按钮安装悬停事件过滤器
    ui->playButton->installEventFilter(this);
    ui->profileButton->installEventFilter(this);
    ui->settingsButton->installEventFilter(this);
    ui->exitButton->installEventFilter(this);

    // 加载背景图
    m_background.load(":/img/ttt.png");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    if (!m_background.isNull()) {
        QPixmap scaled = m_background.scaled(size(),
                                              Qt::KeepAspectRatioByExpanding,
                                              Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    }
}


void MainWindow::on_playButton_clicked()
{
    m_click1Sfx->play();
    qDebug() << "playButton is clicked";

    this->hide(); //隐藏当前主窗口
    SongWindow *songWin = new SongWindow();

    songWin->setMainWindow(this);        // 把主窗口指针传给选歌窗口
    songWin->setFixedSize(this->size()); //设置窗口大小与主窗口相同
    songWin->show();
    //songWin->showMaximized();
}

void MainWindow::on_profileButton_clicked()
{
    m_click1Sfx->play();
    qDebug() << "profileButton is clicked";

    this->hide(); //隐藏当前主窗口
    ProfileWindow *profileWin = new ProfileWindow();

    profileWin->setMainWindow(this);        // 把主窗口指针传给个人资料窗口
    profileWin->setFixedSize(this->size()); //设置窗口大小与主窗口相同
    profileWin->show();
}

void MainWindow::on_settingsButton_clicked()
{
    m_click1Sfx->play();
    qDebug() << "settingsButton is clicked";

    this->hide();
    SettingsWindow *settingsWin = new SettingsWindow();

    settingsWin->setMainWindow(this);
    settingsWin->setFixedSize(this->size());
    settingsWin->show();
}

void MainWindow::on_exitButton_clicked()
{
    m_click1Sfx->play();
    qApp->exit();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        m_click3Sfx->play();
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::displayShadowForButtons()
{ //给按钮加上阴影的函数
    QGraphicsDropShadowEffect *shadow[4];
    for (int i = 0; i < 4; i++) {
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
