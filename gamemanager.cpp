#include "gamemanager.h"
#include <QColor>

GameManager::GameManager(QObject *parent)
    : QObject{parent}
{}

void GameManager::reset()
{
    m_score = 0;
    m_combo = 0;
    m_maxCombo = 0;
    m_perfectCount = 0;
    m_greatCount = 0;
    m_goodCount = 0;
    m_missCount = 0;
    m_accuracy = 0;

    emit scoreChanged(m_score);
    emit comboChanged(m_combo);
    emit accuracyChanged(m_accuracy);
}

void GameManager::onJudgement(int diffMs)
{
    QString text;
    QColor color;
    int score = 0;

    if (diffMs == 999) {
        m_combo = 0;
        m_missCount++;
        text = "MISS";
        color = Qt::gray;
        score = 0;
    } else if (diffMs <= 70) {
        m_combo++;
        m_perfectCount++;
        text = "PERFECT";
        color = QColor(255, 215, 0);
        score = 1010000 / m_allNoteNum;
    } else if (diffMs <= 110) {
        m_combo++;
        m_greatCount++;
        text = "GREAT";
        color = QColor(255, 0, 127);
        score = 1010000 * 0.75 / m_allNoteNum;
    } else if (diffMs <= 200) {
        m_combo++;
        m_goodCount++;
        text="GOOD";
        color = QColor(169, 219, 140);
        score = 1010000 * 0.5 / m_allNoteNum;
    } else {
        m_combo = 0;
        m_missCount++;
        text = "MISS";
        color = Qt::gray;
        score = 0;
    }

    m_score += score;
    m_accuracy = (double) 100 * (m_allNoteNum - m_greatCount * 0.25 - m_goodCount * 0.5 - m_missCount) / m_allNoteNum;

    if (m_combo > m_maxCombo) {
        m_maxCombo = m_combo;
    }

    emit scoreChanged(m_score);
    emit comboChanged(m_combo);
    emit accuracyChanged(m_accuracy);
    emit judgementResult(text, this);
}