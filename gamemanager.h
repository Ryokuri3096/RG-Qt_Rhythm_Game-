#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>

class GameManager : public QObject
{
    Q_OBJECT
public:
    explicit GameManager(QObject *parent = nullptr);

    void reset(); // 重置游戏数据

    // 获取当前数据
    int score() const { return m_score; }
    int combo() const { return m_combo; }
    int maxCombo() const { return m_maxCombo; }
    double accuracy() const { return m_accuracy; }
    int perfectCount() const { return m_perfectCount; }
    int greatCount() const { return m_greatCount; }
    int goodCount() const { return m_goodCount; }
    int missCount() const { return m_missCount; }

    void addAllNoteNum() { m_allNoteNum++; };

signals:
    // 分数 连击数变化时发射 用于更新PlayWindow显示的信息 由SongWindow接收
    void scoreChanged(int score);
    void comboChanged(int combo);
    void accuracyChanged(double accuracy);
    void judgementResult(const QString &text, const QColor &color);

public slots:
    // 接受PlayWindow发射的信号 处理音符判定 999表示miss
    void onJudgement(int diffMs);

private:
    int m_score = 0;
    int m_combo = 0;
    int m_maxCombo = 0;
    double m_accuracy = 100.0;
    int m_perfectCount = 0;
    int m_greatCount = 0;
    int m_goodCount = 0;
    int m_missCount = 0;
    int m_allNoteNum = -1; // 去除特殊note的影响
};

#endif // GAMEMANAGER_H
