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

    // setupUI();
}

PauseOverlay::~PauseOverlay()
{
    delete ui;
}

void PauseOverlay::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    // 暂停提示文字
    QLabel *label = new QLabel("暂停", this);
    label->setStyleSheet("color: white; font-size: 32px; font-weight: bold;");
    label->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(label);

    mainLayout->addSpacing(30);

    // 按钮水平排列
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(20);

    auto createBtn = [&](const QString &text) {
        QPushButton *btn = new QPushButton(text, this);
        btn->setFixedSize(140, 50);
        btn->setStyleSheet(
            "QPushButton { color: white; background-color: #444; font-size: 18px; border-radius: 8px; }"
            "QPushButton:hover { background-color: #666; }"
            );
        return btn;
    };

    QPushButton *btnResume = createBtn("继续");
    QPushButton *btnRestart = createBtn("重新开始");
    QPushButton *btnBack = createBtn("返回选歌");

    connect(btnResume, &QPushButton::clicked, this, &PauseOverlay::resumeGame);
    connect(btnRestart, &QPushButton::clicked, this, &PauseOverlay::restartGame);
    connect(btnBack, &QPushButton::clicked, this, &PauseOverlay::backToMenu);

    btnLayout->addWidget(btnResume);
    btnLayout->addWidget(btnRestart);
    btnLayout->addWidget(btnBack);
    mainLayout->addLayout(btnLayout);
}