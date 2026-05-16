#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private slots:
    void on_playButton_clicked();

    void on_profileButton_clicked();

    void on_settingsButton_clicked();

    void on_exitButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
