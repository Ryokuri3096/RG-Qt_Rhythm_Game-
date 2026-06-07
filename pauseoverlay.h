#ifndef PAUSEOVERLAY_H
#define PAUSEOVERLAY_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSoundEffect>

namespace Ui {
class PauseOverlay;
}

class PauseOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit PauseOverlay(QWidget *parent = nullptr);
    ~PauseOverlay();

signals:
    void resumeGame(); // 继续
    void restartGame(); // 重新开始
    void backToMenu(); // 返回选歌

private:
    Ui::PauseOverlay *ui;
    QSoundEffect *m_click4Sfx = nullptr; // 功能按钮音效

    void setupUI();
};

#endif // PAUSEOVERLAY_H
