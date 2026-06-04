#ifndef PROFILEWINDOW_H
#define PROFILEWINDOW_H

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSettings>

class MainWindow;

namespace Ui {
class ProfileWindow;
}

class ProfileWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProfileWindow(QWidget *parent = nullptr);
    ~ProfileWindow();

    void setMainWindow(MainWindow *win); // 设置主窗口指针用于返回

protected:
    void paintEvent(QPaintEvent *event) override;  // 绘制全屏背景图
    void resizeEvent(QResizeEvent *event) override; // 窗口大小变化时更新面板位置
    void showEvent(QShowEvent *event) override;     // 显示时加载数据并定位面板
    bool eventFilter(QObject *obj, QEvent *event) override; // 拦截头像和昵称标签的点击

private slots:
    void on_backButton_clicked();        // 返回主菜单
    void onNicknameEditFinished();       // 昵称编辑完成（回车）→ 保存并切换回显示模式

private:
    void applyStyles();       // 给 .ui 中控件应用代码样式
    void connectSignals();    // 连接信号槽
    void updateOverlayPos();  // 更新左右面板位置（resizeEvent时调用）

    // 数据持久化
    void loadProfileData(); 
    void saveNickname(const QString &name);
    void saveAvatarPath(const QString &path);
    void loadPlayHistory();
    void refreshCardDisplay();

    void switchToEditMode();  // 昵称标签 → 编辑框
    void switchToDisplayMode(); // 编辑框 → 昵称标签（保存并刷新）
    void onAvatarClicked();   // 点击头像 → 打开文件对话框更换头像

    // 游玩记录结构体
    struct PlayRecord {
        QString songId;
        QString songName;
        int score;
        QString coverPath;
        QString playTime;
    };

    Ui::ProfileWindow *ui;
    MainWindow *m_mainWin;
    QSettings *m_settings;

    // 背景图
    QPixmap m_background;

    // 游玩记录列表（最多6条）
    QList<PlayRecord> m_playHistory;

    // 6张游玩记录卡片的子控件
    QFrame *m_cardFrames[6];     // 卡片外框 QFrame
    QLabel *m_cardMasks[6];      // 灰色半透明蒙版
    QLabel *m_cardSongNames[6];  // 歌曲名称
    QLabel *m_cardScores[6];     // 分数

    // 统计数据的数值标签
    QLabel *m_countLabels[3];    // [0]=clear, [1]=fc, [2]=ap
};

#endif // PROFILEWINDOW_H
