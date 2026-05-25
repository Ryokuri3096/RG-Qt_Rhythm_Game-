#ifndef PLAYWINDOW_H
#define PLAYWINDOW_H

#include <QKeyEvent>
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
#include "gamemanager.h"

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

    Ui::PlayWindow *getUI();

signals:
    void judgement(int dissMs);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void gameLoop();

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

    // 游戏状态
    QElapsedTimer m_elapsed; // 游戏时长
    qint64 m_musicStartOffset = 0; // 偏移量
    int m_hitLineY = 0; // 判定线Y坐标
    float m_baseSpeed = 1.2f; // 基础下落速度 单位为px/ms

    // 当前按住的Hold
    std::vector<GameNote*> m_activeHolds;

    // Filck轨道鼠标状态
    QPoint m_lastMousePos;
    QSet<int> m_activeFilckLanes; // Flick轨道编号

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
};

#endif // PLAYWINDOW_H
