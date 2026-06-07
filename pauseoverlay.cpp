#include "pauseoverlay.h"
#include "ui_pauseoverlay.h"

PauseOverlay::PauseOverlay(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PauseOverlay)
{
    ui->setupUi(this);
    setStyleSheet("background-color: rgba(0, 0, 0, 200);");

    // 初始化按钮音效
    m_click4Sfx = new QSoundEffect(this);
    m_click4Sfx->setSource(QUrl("qrc:/sfx/click4.wav"));

    connect(ui->resumeBtn, &QPushButton::clicked, this, [this](){ m_click4Sfx->play(); emit resumeGame(); });
    connect(ui->restartBtn, &QPushButton::clicked, this, [this](){ m_click4Sfx->play(); emit restartGame(); });
    connect(ui->backBtn, &QPushButton::clicked, this, [this](){ m_click4Sfx->play(); emit backToMenu(); });
}

PauseOverlay::~PauseOverlay()
{
    delete ui;
}