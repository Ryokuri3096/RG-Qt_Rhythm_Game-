#include "settingswindow.h"
#include "mainwindow.h"
#include "ui_settingswindow.h"

#include <QPainter>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>


SettingsWindow::SettingsWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SettingsWindow)
    , m_mainWin(nullptr)
    , m_settings(new QSettings("RG", "Settings", this))
    , m_musicVolume(1.0)
    , m_sfxVolume(1.0)
    , m_speed(1.0)
{
    ui->setupUi(this);

    m_background.load(":/img/play_bp.jpg");

    // [0]=music, [1]=sfx, [2]=speed
    m_sliders[0] = ui->musicVolumeSlider;
    m_sliders[1] = ui->sfxVolumeSlider;
    m_sliders[2] = ui->speedSlider;

    m_valueLabels[0] = ui->musicVolumeValueLabel;
    m_valueLabels[1] = ui->sfxVolumeValueLabel;
    m_valueLabels[2] = ui->speedValueLabel;

    m_minusBtns[0] = ui->musicVolumeMinusBtn;
    m_minusBtns[1] = ui->sfxVolumeMinusBtn;
    m_minusBtns[2] = ui->speedMinusBtn;

    m_plusBtns[0] = ui->musicVolumePlusBtn;
    m_plusBtns[1] = ui->sfxVolumePlusBtn;
    m_plusBtns[2] = ui->speedPlusBtn;

    // 键位映射按钮
    m_keyBtns[0] = ui->lane0Btn;
    m_keyBtns[1] = ui->lane1Btn;
    m_keyBtns[2] = ui->lane2Btn;
    m_keyBtns[3] = ui->lane3Btn;

    // 默认键位映射
    int defaultKeys[] = {Qt::Key_S, Qt::Key_D, Qt::Key_J, Qt::Key_K};
    for (int i = 0; i < 4; i++) {
        m_keyMapping[i] = defaultKeys[i];
    }

    // 设置滑动条范围和初始值
    for (int i = 0; i < 2; i++) {
        m_sliders[i]->setRange(0, 100);
        m_sliders[i]->setValue(50);
    }
    m_sliders[2]->setRange(1, 200);
    m_sliders[2]->setValue(100);

    setFocusPolicy(Qt::StrongFocus); // 用于捕获按键重新映射

    // 初始化音效
    m_click1Sfx = new QSoundEffect(this);
    m_click1Sfx->setSource(QUrl("qrc:/sfx/click1.wav"));
    m_click3Sfx = new QSoundEffect(this);
    m_click3Sfx->setSource(QUrl("qrc:/sfx/click3.wav"));
    m_click4Sfx = new QSoundEffect(this);
    m_click4Sfx->setSource(QUrl("qrc:/sfx/click4.wav"));

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


// 给 .ui 中的控件设置代码样式
void SettingsWindow::applyStyles()
{
    // 加减按钮
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
    for (int i = 0; i < 3; i++) {
        m_minusBtns[i]->setStyleSheet(btnStyle);
        m_plusBtns[i]->setStyleSheet(btnStyle);
    }

    // 滑动条
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
    for (int i = 0; i < 3; i++) {
        m_sliders[i]->setStyleSheet(sliderStyle);
    }
}

// 信号槽连接
void SettingsWindow::connectSignals()
{
    // 音量滑动条
    connect(ui->musicVolumeSlider, &QSlider::valueChanged, this, &SettingsWindow::onVolumeSliderChanged);
    connect(ui->sfxVolumeSlider,   &QSlider::valueChanged, this, &SettingsWindow::onVolumeSliderChanged);
    // 流速滑动条
    connect(ui->speedSlider,       &QSlider::valueChanged, this, &SettingsWindow::onSpeedSliderChanged);

    // 音量加减按钮
    connect(ui->musicVolumeMinusBtn, &QPushButton::clicked, this, &SettingsWindow::onMusicMinus);
    connect(ui->musicVolumePlusBtn,  &QPushButton::clicked, this, &SettingsWindow::onMusicPlus);
    connect(ui->sfxVolumeMinusBtn,   &QPushButton::clicked, this, &SettingsWindow::onSfxMinus);
    connect(ui->sfxVolumePlusBtn,    &QPushButton::clicked, this, &SettingsWindow::onSfxPlus);
    // 流速加减按钮
    connect(ui->speedMinusBtn,       &QPushButton::clicked, this, &SettingsWindow::onSpeedMinus);
    connect(ui->speedPlusBtn,        &QPushButton::clicked, this, &SettingsWindow::onSpeedPlus);

    // 四个键位映射按钮
    for (int i = 0; i < 4; i++) {
        connect(m_keyBtns[i], &QPushButton::clicked, this, &SettingsWindow::onKeyMappingBtnClicked);
    }
}

// 滑动条值改变
void SettingsWindow::onVolumeSliderChanged(int val)
{
    QSlider *s = qobject_cast<QSlider*>(sender());
    int idx = -1;
    for (int i = 0; i < 2; i++) {
        if (m_sliders[i] == s) { idx = i; break; }
    }
    if (idx < 0) return;

    // 更新对应成员变量
    double *volPtrs[] = {&m_musicVolume, &m_sfxVolume};
    *volPtrs[idx] = val / 100.0;

    updateValueLabels();
    updateButtonStates();
    saveSettings();

    const char *keys[] = {"musicVolume", "sfxVolume"};
    emit settingChanged(keys[idx], *volPtrs[idx]);
}

void SettingsWindow::onSpeedSliderChanged(int val)
{
    m_speed = val / 100.0;
    updateValueLabels();
    updateButtonStates();
    saveSettings();
    emit settingChanged("speed", m_speed);
}

// 加减按钮
void SettingsWindow::onMusicMinus()
{
    m_click3Sfx->play();
    m_sliders[0]->setValue(qMax(m_sliders[0]->value() - 5, m_sliders[0]->minimum()));
}
void SettingsWindow::onMusicPlus()
{
    m_click3Sfx->play();
    m_sliders[0]->setValue(qMin(m_sliders[0]->value() + 5, m_sliders[0]->maximum()));
}
void SettingsWindow::onSfxMinus()
{
    m_click3Sfx->play();
    m_sliders[1]->setValue(qMax(m_sliders[1]->value() - 5, m_sliders[1]->minimum()));
}
void SettingsWindow::onSfxPlus()
{
    m_click3Sfx->play();
    m_sliders[1]->setValue(qMin(m_sliders[1]->value() + 5, m_sliders[1]->maximum()));
}
void SettingsWindow::onSpeedMinus()
{
    m_click3Sfx->play();
    m_sliders[2]->setValue(qMax(m_sliders[2]->value() - 5, m_sliders[2]->minimum()));
}
void SettingsWindow::onSpeedPlus()
{
    m_click3Sfx->play();
    m_sliders[2]->setValue(qMin(m_sliders[2]->value() + 5, m_sliders[2]->maximum()));
}

// 键位映射按钮点击 — 进入等待按键状态
void SettingsWindow::onKeyMappingBtnClicked()
{
    m_click4Sfx->play();
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    for (int i = 0; i < 4; i++) {
        if (m_keyBtns[i] == btn) {
            m_waitingForLane = i;
            btn->setText("[...]");
            this->setFocus(); // 让窗口获取焦点，捕获按键
            break;
        }
    }
}

// 捕获用户按键用于键位映射
void SettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_waitingForLane >= 0) {
        int key = event->key();
        // 忽略修饰键单独按下
        if (key == Qt::Key_Shift || key == Qt::Key_Control ||
            key == Qt::Key_Alt || key == Qt::Key_Meta || key == Qt::Key_Escape) {
            return;
        }

        // 如果这个键已被其他轨道占用，则交换两者
        for (int i = 0; i < 4; i++) {
            if (i != m_waitingForLane && m_keyMapping[i] == key) {
                int oldKey = m_keyMapping[m_waitingForLane];
                m_keyMapping[i] = oldKey;
                m_keyBtns[i]->setText(QKeySequence(oldKey).toString());
                break;
            }
        }

        m_keyMapping[m_waitingForLane] = key;
        m_keyBtns[m_waitingForLane]->setText(QKeySequence(key).toString());
        m_waitingForLane = -1;
        saveSettings();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

// 数值显示更新
void SettingsWindow::updateValueLabels()
{
    m_valueLabels[0]->setText(QString::number(m_musicVolume, 'f', 2));
    m_valueLabels[1]->setText(QString::number(m_sfxVolume, 'f', 2));
    m_valueLabels[2]->setText(QString::number(m_speed, 'f', 2));
}

void SettingsWindow::updateButtonStates()
{
    for (int i = 0; i < 3; i++) {
        m_minusBtns[i]->setEnabled(m_sliders[i]->value() > m_sliders[i]->minimum());
        m_plusBtns[i]->setEnabled(m_sliders[i]->value() < m_sliders[i]->maximum());
    }
}

// QSettings 持久化存储
void SettingsWindow::loadSettings()
{
    // 读取滑块值
    const char *keys[] = {"audio/musicVolume", "audio/sfxVolume", "gameplay/speed"};
    int defaults[] = {50, 50, 100};

    for (int i = 0; i < 3; i++) {
        int v = m_settings->value(keys[i], defaults[i]).toInt();
        m_sliders[i]->blockSignals(true);
        m_sliders[i]->setValue(v);
        m_sliders[i]->blockSignals(false);
    }

    m_musicVolume = m_sliders[0]->value() / 100.0;
    m_sfxVolume   = m_sliders[1]->value() / 100.0;
    m_speed       = m_sliders[2]->value() / 100.0;

    updateValueLabels();
    updateButtonStates();

    // 读取键位映射
    int defaultKeys[] = {Qt::Key_S, Qt::Key_D, Qt::Key_J, Qt::Key_K};
    for (int i = 0; i < 4; i++) {
        m_keyMapping[i] = m_settings->value(
            QString("gameplay/keyMapping%1").arg(i), defaultKeys[i]).toInt();
        m_keyBtns[i]->setText(QKeySequence(m_keyMapping[i]).toString());
    }
}

void SettingsWindow::saveSettings()
{
    const char *keys[] = {"audio/musicVolume", "audio/sfxVolume", "gameplay/speed"};
    for (int i = 0; i < 3; i++) {
        m_settings->setValue(keys[i], m_sliders[i]->value());
    }
    // 保存键位映射
    for (int i = 0; i < 4; i++) {
        m_settings->setValue(QString("gameplay/keyMapping%1").arg(i), m_keyMapping[i]);
    }
}

// 遮罩层定位
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

// 背景绘制
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

// 返回主菜单
void SettingsWindow::on_backButton_clicked()
{
    m_click4Sfx->play();
    if (m_mainWin) {
        m_mainWin->show();
    }
    this->hide(); // 先隐藏，避免视觉残留
    // 延迟关闭，确保音效播放完毕
    QTimer::singleShot(200, this, &QWidget::deleteLater);
}
