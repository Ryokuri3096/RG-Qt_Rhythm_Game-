#include "settingswindow.h"
#include "mainwindow.h"
#include "ui_settingswindow.h"

#include <QPainter>
#include <QResizeEvent>
#include <QShowEvent>

// ────────────────────────────────────────────
// 构造与析构
// ────────────────────────────────────────────
SettingsWindow::SettingsWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SettingsWindow)
    , m_mainWin(nullptr)
    , m_settings(new QSettings("RG", "Settings", this))
    , m_musicVolume(1.0)
    , m_sfxVolume(1.0)
    , m_bgmVolume(1.0)
    , m_offset(0)
{
    ui->setupUi(this);

    m_background.load(":/img/mainmenu.png");

    // 从 .ui 中收集控件指针，存入数组方便批量操作
    // [0]=music, [1]=sfx, [2]=bgm, [3]=offset
    m_sliders[0] = ui->musicVolumeSlider;
    m_sliders[1] = ui->sfxVolumeSlider;
    m_sliders[2] = ui->bgmVolumeSlider;
    m_sliders[3] = ui->offsetSlider;

    m_valueLabels[0] = ui->musicVolumeValueLabel;
    m_valueLabels[1] = ui->sfxVolumeValueLabel;
    m_valueLabels[2] = ui->bgmVolumeValueLabel;
    m_valueLabels[3] = ui->offsetValueLabel;

    m_minusBtns[0] = ui->musicVolumeMinusBtn;
    m_minusBtns[1] = ui->sfxVolumeMinusBtn;
    m_minusBtns[2] = ui->bgmVolumeMinusBtn;
    m_minusBtns[3] = ui->offsetMinusBtn;

    m_plusBtns[0] = ui->musicVolumePlusBtn;
    m_plusBtns[1] = ui->sfxVolumePlusBtn;
    m_plusBtns[2] = ui->bgmVolumePlusBtn;
    m_plusBtns[3] = ui->offsetPlusBtn;

    // 设置滑动条范围和初始值
    // 音量: 0~100, 延时: 0~400
    for (int i = 0; i < 3; i++) {
        m_sliders[i]->setRange(0, 100);
        m_sliders[i]->setValue(50);
    }
    m_sliders[3]->setRange(0, 400);
    m_sliders[3]->setValue(200);

    applyStyles();
    connectSignals();
    loadSettings();
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::setMainWindow(MainWindow *win)
{
    m_mainWin = win;
}

// ────────────────────────────────────────────
// 给 .ui 中的控件设置代码样式
// ────────────────────────────────────────────
void SettingsWindow::applyStyles()
{
    // 加减按钮通用样式
    QString btnStyle = QStringLiteral(
        "QPushButton {"
        "  color: white; font-size: 18px; font-weight: bold;"
        "  background: rgba(255, 255, 255, 30);"
        "  border: 1px solid rgba(255, 255, 255, 60);"
        "  border-radius: 4px;"
        "}"
        "QPushButton:hover { background: rgba(255, 255, 255, 60); }"
        "QPushButton:disabled { color: #666; background: transparent; border-color: #444; }"
    );
    for (int i = 0; i < 4; i++) {
        m_minusBtns[i]->setStyleSheet(btnStyle);
        m_plusBtns[i]->setStyleSheet(btnStyle);
    }

    // 滑动条通用样式
    QString sliderStyle = QStringLiteral(
        "QSlider::groove:horizontal {"
        "  height: 6px; background: #555; border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "  width: 16px; height: 16px; margin: -5px 0;"
        "  background: #888; border-radius: 8px;"
        "}"
        "QSlider::handle:horizontal:hover { background: #aaa; }"
        "QSlider::sub-page:horizontal {"
        "  background: #666; border-radius: 3px;"
        "}"
    );
    for (int i = 0; i < 4; i++) {
        m_sliders[i]->setStyleSheet(sliderStyle);
    }
}

// ────────────────────────────────────────────
// 信号槽连接
// ────────────────────────────────────────────
void SettingsWindow::connectSignals()
{
    // 三个音量滑动条共用同一个槽（通过 sender() 区分或直接用匿名 lambda）
    connect(ui->musicVolumeSlider, &QSlider::valueChanged, this, &SettingsWindow::onVolumeSliderChanged);
    connect(ui->sfxVolumeSlider,   &QSlider::valueChanged, this, &SettingsWindow::onVolumeSliderChanged);
    connect(ui->bgmVolumeSlider,   &QSlider::valueChanged, this, &SettingsWindow::onVolumeSliderChanged);
    connect(ui->offsetSlider,      &QSlider::valueChanged, this, &SettingsWindow::onOffsetSliderChanged);

    // 加减按钮
    connect(ui->musicVolumeMinusBtn, &QPushButton::clicked, this, &SettingsWindow::onMusicMinus);
    connect(ui->musicVolumePlusBtn,  &QPushButton::clicked, this, &SettingsWindow::onMusicPlus);
    connect(ui->sfxVolumeMinusBtn,   &QPushButton::clicked, this, &SettingsWindow::onSfxMinus);
    connect(ui->sfxVolumePlusBtn,    &QPushButton::clicked, this, &SettingsWindow::onSfxPlus);
    connect(ui->bgmVolumeMinusBtn,   &QPushButton::clicked, this, &SettingsWindow::onBgmMinus);
    connect(ui->bgmVolumePlusBtn,    &QPushButton::clicked, this, &SettingsWindow::onBgmPlus);
    connect(ui->offsetMinusBtn,      &QPushButton::clicked, this, &SettingsWindow::onOffsetMinus);
    connect(ui->offsetPlusBtn,       &QPushButton::clicked, this, &SettingsWindow::onOffsetPlus);
}

// ────────────────────────────────────────────
// 滑动条值改变（共用槽）
// ────────────────────────────────────────────
void SettingsWindow::onVolumeSliderChanged(int val)
{
    QSlider *s = qobject_cast<QSlider*>(sender());
    int idx = -1;
    for (int i = 0; i < 3; i++) {
        if (m_sliders[i] == s) { idx = i; break; }
    }
    if (idx < 0) return;

    // 通过指针数组更新对应成员变量
    double *volPtrs[] = {&m_musicVolume, &m_sfxVolume, &m_bgmVolume};
    *volPtrs[idx] = val / 100.0;

    updateVolumeLabels();
    updateButtonStates();
    saveSettings();

    const char *keys[] = {"musicVolume", "sfxVolume", "bgmVolume"};
    emit settingChanged(keys[idx], *volPtrs[idx]);
}

void SettingsWindow::onOffsetSliderChanged(int val)
{
    m_offset = val - 200;
    updateOffsetLabel();
    updateButtonStates();
    saveSettings();
    emit settingChanged("offset", m_offset);
}

// ────────────────────────────────────────────
// 加减按钮
// ────────────────────────────────────────────
void SettingsWindow::onMusicMinus()
{
    m_sliders[0]->setValue(qMax(m_sliders[0]->value() - 5, m_sliders[0]->minimum()));
}
void SettingsWindow::onMusicPlus()
{
    m_sliders[0]->setValue(qMin(m_sliders[0]->value() + 5, m_sliders[0]->maximum()));
}
void SettingsWindow::onSfxMinus()
{
    m_sliders[1]->setValue(qMax(m_sliders[1]->value() - 5, m_sliders[1]->minimum()));
}
void SettingsWindow::onSfxPlus()
{
    m_sliders[1]->setValue(qMin(m_sliders[1]->value() + 5, m_sliders[1]->maximum()));
}
void SettingsWindow::onBgmMinus()
{
    m_sliders[2]->setValue(qMax(m_sliders[2]->value() - 5, m_sliders[2]->minimum()));
}
void SettingsWindow::onBgmPlus()
{
    m_sliders[2]->setValue(qMin(m_sliders[2]->value() + 5, m_sliders[2]->maximum()));
}
void SettingsWindow::onOffsetMinus()
{
    m_sliders[3]->setValue(qMax(m_sliders[3]->value() - 5, m_sliders[3]->minimum()));
}
void SettingsWindow::onOffsetPlus()
{
    m_sliders[3]->setValue(qMin(m_sliders[3]->value() + 5, m_sliders[3]->maximum()));
}

// ────────────────────────────────────────────
// 数值显示更新
// ────────────────────────────────────────────
void SettingsWindow::updateVolumeLabels()
{
    m_valueLabels[0]->setText(QString::number(m_musicVolume, 'f', 2));
    m_valueLabels[1]->setText(QString::number(m_sfxVolume, 'f', 2));
    m_valueLabels[2]->setText(QString::number(m_bgmVolume, 'f', 2));
}

void SettingsWindow::updateOffsetLabel()
{
    m_valueLabels[3]->setText(QString("%1 ms").arg(m_offset));
}

void SettingsWindow::updateButtonStates()
{
    for (int i = 0; i < 4; i++) {
        m_minusBtns[i]->setEnabled(m_sliders[i]->value() > m_sliders[i]->minimum());
        m_plusBtns[i]->setEnabled(m_sliders[i]->value() < m_sliders[i]->maximum());
    }
}

// ────────────────────────────────────────────
// QSettings 持久化存储
// ────────────────────────────────────────────
void SettingsWindow::loadSettings()
{
    const char *keys[] = {"audio/musicVolume", "audio/sfxVolume", "audio/bgmVolume", "audio/offset"};
    int defaults[] = {50, 50, 50, 200};

    for (int i = 0; i < 4; i++) {
        int v = m_settings->value(keys[i], defaults[i]).toInt();
        m_sliders[i]->blockSignals(true);
        m_sliders[i]->setValue(v);
        m_sliders[i]->blockSignals(false);
    }

    m_musicVolume = m_sliders[0]->value() / 100.0;
    m_sfxVolume   = m_sliders[1]->value() / 100.0;
    m_bgmVolume   = m_sliders[2]->value() / 100.0;
    m_offset      = m_sliders[3]->value() - 200;

    updateVolumeLabels();
    updateOffsetLabel();
    updateButtonStates();
}

void SettingsWindow::saveSettings()
{
    const char *keys[] = {"audio/musicVolume", "audio/sfxVolume", "audio/bgmVolume", "audio/offset"};
    for (int i = 0; i < 4; i++) {
        m_settings->setValue(keys[i], m_sliders[i]->value());
    }
}

// ────────────────────────────────────────────
// 遮罩层定位（80%×80% 居中）
// ────────────────────────────────────────────
void SettingsWindow::updateOverlayPos()
{
    QWidget *cw = ui->centralwidget;
    if (ui->overlayFrame && cw) {
        int ow = cw->width() * 0.8;
        int oh = cw->height() * 0.8;
        int ox = (cw->width() - ow) / 2;
        int oy = (cw->height() - oh) / 2;
        ui->overlayFrame->setGeometry(ox, oy, ow, oh);
        ui->overlayFrame->raise();
    }
}

// ────────────────────────────────────────────
// 背景绘制（全屏铺满、比例不变形）
// ────────────────────────────────────────────
void SettingsWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    if (!m_background.isNull()) {
        QPixmap scaled = m_background.scaled(size(),
                                              Qt::KeepAspectRatioByExpanding,
                                              Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    }
}

void SettingsWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateOverlayPos();
    update();
}

void SettingsWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    updateOverlayPos();
}

// ────────────────────────────────────────────
// 返回主菜单
// ────────────────────────────────────────────
void SettingsWindow::on_backButton_clicked()
{
    if (m_mainWin) {
        m_mainWin->show();
    }
    this->close();
    this->deleteLater();
}
