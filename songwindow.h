#ifndef SONGWINDOW_H
#define SONGWINDOW_H

#include <QDir>
#include <QDirIterator>
#include <QFile>           // 打开文件
#include <QJsonArray>      // 操作 JSON 数组
#include <QJsonDocument>   // 解析 JSON 文档
#include <QJsonObject>     // 操作 JSON 对象
#include <QJsonParseError> // 解析错误信息
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSoundEffect>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include "qmainwindow.h"
#include "playwindow.h"

class MainWindow;

namespace Ui {
class SongWindow;
}

class SongWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SongWindow(QMainWindow *parent = nullptr);
    ~SongWindow();

    void setMainWindow(MainWindow *win); //传递主窗口指针的函数

protected:
    void paintEvent(QPaintEvent *event) override; // 绘制背景图

private slots:
    void on_backButton_clicked();

    void on_startButton_clicked();

private:
    Ui::SongWindow *ui;
    MainWindow* m_mainWin;
    QSoundEffect *m_click1Sfx = nullptr; // 通用按钮音效
    QSoundEffect *m_click4Sfx = nullptr; // 返回/选曲按钮音效
    QPixmap m_background; // 背景图
    QMap<QString, QJsonObject> m_chartData;  // 存储每个文件的 JSON 数据
    QString songPath;

    void loadCharts();
    QJsonObject loadJsonFile(const QString &path);  // 读取.json文件信息的函数
    QString findCoverImage(const QString &dirPath); // 查找目录下的封面图片
    void updateLeftPanel(const QString &jsonPath, const QJsonObject &jsonObj); // 更新左侧显示
    void judgementManage(PlayWindow *play, const QString &text, GameManager *gm);
    QScrollArea *scrollArea;
    QVector<QPushButton *> buttons;
};

#endif // SONGWINDOW_H
