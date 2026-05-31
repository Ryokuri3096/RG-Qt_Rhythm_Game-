#ifndef PLAYWINDOW_H
#define PLAYWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QElapsedTimer>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QFile>
#include <QPoint>
#include <QSet>
#include <QMap>
#include <QtMath>
#include <QDir>
#include <QDebug>

#include "gamemanager.h"
#include "resultoverlay.h"
#include "pauseoverlay.h"

namespace Ui {
class PlayWindow;
}

class PlayWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlayWindow(QWidget *parent = nullptr);
    ~PlayWindow();

    void extracted(QJsonArray &timeArray);
    void loadChart(const QString &chartPath, GameManager &gameManager);
    void startGame();
    void setFps(int fps); // 60 或 120
    int fps() const;

    void togglePause(); // 切换暂停/恢复
    void restartGame(); // 重新开始

    Ui::PlayWindow *getUI();

signals:
    void judgement(int dissMs); // 处理判定 接收判定信号的函数
    void gameFinished(); // 游戏结束信号
    void returnToMenu();
    void restartRequested(); // 请求重启游戏 由GameManager接收 重置GameManager各成员变量

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void gameLoop();

public slots:
    void showResult(GameManager *gm); // 显示结算画面

private:
    Ui::PlayWindow *ui;

    struct NoteData {
        double timeMs; // 到达判定线的时间
        double endTimeMs; // Hold结束时间 非Hold则为0
        int lane; // 0~4轨道
        enum Type { TAP, HOLD, FLICK } type;
    };

    struct GameNote {
        NoteData data;
        bool missed = false;
        bool judged = false;
        bool holding = false;
    };

    // 谱面数据
    std::vector<GameNote> m_notes;
    double m_bpm = 130.0; // 乐曲bpm
    int m_divide = 4; // 每拍的细分 默认为4
    double m_speedFactor = 1.0; // 速度倍率
    QString m_songTitle; // 乐曲标题
    QString m_songArtist; // 乐曲作者
    QString m_coverPath; // 曲绘文件路径（可以是绝对路径或资源路径）
    QString m_difficulty; // 谱面版本名称

    // 游戏状态
    QElapsedTimer m_elapsed; // 游戏时长
    qint64 m_musicStartOffset = 0; // 偏移量
    int m_hitLineY = 0; // 判定线Y坐标
    float m_baseSpeed = 1.2f; // 基础下落速度 单位为px/ms
    int m_fps = 120; // 帧率 默认120
    bool m_gameEnded = false; // 记录游戏是否结束 防止重复发射
    bool m_showResult = false; // 是否处于结算前清理状态

    // 暂停相关
    bool m_paused = false;
    PauseOverlay *m_pauseOverlay = nullptr;
    qint64 m_pausedTime = 0; // 暂停时的elapsed值

    // 当前按住的Hold
    std::vector<GameNote*> m_activeHolds;

    // Filck轨道鼠标状态
    QPoint m_lastMousePos;
    QSet<int> m_activeFlickLanes; // Flick轨道编号

    // 按键映射
    QMap<int, int> m_keyLaneMap;

    // 音频播放相关
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;

    // 定时器
    QTimer *m_gameTimer = nullptr;

    // 其他辅助函数
    void checkTapHit(int lane);
    void checkHoldRelease(int lane);
    void checkFlickHit(int deltaY);
    void updateMissedNotes();
    qint64 currentMusicTime() const;

    void pauseGame();
    void resumeGame();
};

#endif // PLAYWINDOW_H
