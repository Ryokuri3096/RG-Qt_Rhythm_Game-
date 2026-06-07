#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QPixmap>
#include <QSoundEffect>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void displayShadowForButtons(); //给按钮加上阴影的函数

protected:
    void paintEvent(QPaintEvent *event) override; // 绘制背景图
    bool eventFilter(QObject *obj, QEvent *event) override; // 鼠标悬停音效

private slots:
    void on_playButton_clicked();

    void on_profileButton_clicked();

    void on_settingsButton_clicked();

    void on_exitButton_clicked();

private:
    Ui::MainWindow *ui;
    QPixmap m_background; // 背景图
    QSoundEffect *m_click1Sfx = nullptr; // 通用按钮音效
    QSoundEffect *m_click3Sfx = nullptr; // 悬停音效
};
#endif // MAINWINDOW_H
