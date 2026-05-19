#ifndef SONGWINDOW_H
#define SONGWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QVector>
#include <QDirIterator>
#include <QFile>           // 打开文件
#include <QJsonDocument>   // 解析 JSON 文档
#include <QJsonObject>     // 操作 JSON 对象
#include <QJsonParseError> // 解析错误信息
#include <QJsonArray>      // 操作 JSON 数组
#include <QLabel>
#include "qmainwindow.h"

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

private slots:
    void on_backButton_clicked();

    void on_startButton_clicked();

private:
    Ui::SongWindow *ui;
    MainWindow* m_mainWin;
    QMap<QString, QJsonObject> m_chartData;  // 存储每个文件的 JSON 数据

    void loadCharts();
    QJsonObject loadJsonFile(const QString &path);  // 读取.json文件信息的函数
    QString findCoverImage(const QString &dirPath);  // 查找目录下的封面图片
    void updateLeftPanel(const QString &jsonPath, const QJsonObject &jsonObj); // 更新左侧显示
    QScrollArea *scrollArea;
    QVector<QPushButton*> buttons;
};

#endif // SONGWINDOW_H
