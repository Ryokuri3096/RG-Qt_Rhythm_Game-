#include "pauseoverlay.h"
#include "ui_pauseoverlay.h"

PauseOverlay::PauseOverlay(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PauseOverlay)
{
    ui->setupUi(this);
    setStyleSheet("background-color: rgba(0, 0, 0, 200);");

    connect(ui->resumeBtn, &QPushButton::clicked, this, &PauseOverlay::resumeGame);
    connect(ui->restartBtn, &QPushButton::clicked, this, &PauseOverlay::restartGame);
    connect(ui->backBtn, &QPushButton::clicked, this, &PauseOverlay::backToMenu);
}

PauseOverlay::~PauseOverlay()
{
    delete ui;
}