#include "profilewindow.h"
#include "mainwindow.h"
#include "ui_profilewindow.h"

#include <QApplication>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QUrl>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStandardPaths>
#include <QTimer>


ProfileWindow::ProfileWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ProfileWindow)
    , m_mainWin(nullptr)
    , m_settings(new QSettings("RG", "Profile", this))
{
    ui->setupUi(this);

    // 音效最先初始化，避免后续重操作干扰音频状态
    m_click4Sfx = new QSoundEffect(this);
    m_click4Sfx->setSource(QUrl("qrc:/sfx/click4.wav"));
    m_click4Sfx->setVolume(0.8); // 稍降音量防破音
    m_click1Sfx = new QSoundEffect(this);
    m_click1Sfx->setSource(QUrl("qrc:/sfx/click1.wav"));

    // 加载背景图（与主菜单背景一致）
    m_background.load(":/img/play_bp.jpg");

    // 收集右侧6张游玩记录卡片的子控件指针
    // 卡片 QFrame
    m_cardFrames[0] = ui->cardFrame0;
    m_cardFrames[1] = ui->cardFrame1;
    m_cardFrames[2] = ui->cardFrame2;
    m_cardFrames[3] = ui->cardFrame3;
    m_cardFrames[4] = ui->cardFrame4;
    m_cardFrames[5] = ui->cardFrame5;

    // 灰色半透明蒙版 QLabel
    m_cardMasks[0] = ui->cardMask0;
    m_cardMasks[1] = ui->cardMask1;
    m_cardMasks[2] = ui->cardMask2;
    m_cardMasks[3] = ui->cardMask3;
    m_cardMasks[4] = ui->cardMask4;
    m_cardMasks[5] = ui->cardMask5;

    // 歌曲名称 QLabel
    m_cardSongNames[0] = ui->cardSongName0;
    m_cardSongNames[1] = ui->cardSongName1;
    m_cardSongNames[2] = ui->cardSongName2;
    m_cardSongNames[3] = ui->cardSongName3;
    m_cardSongNames[4] = ui->cardSongName4;
    m_cardSongNames[5] = ui->cardSongName5;

    // 分数 QLabel
    m_cardScores[0] = ui->cardScore0;
    m_cardScores[1] = ui->cardScore1;
    m_cardScores[2] = ui->cardScore2;
    m_cardScores[3] = ui->cardScore3;
    m_cardScores[4] = ui->cardScore4;
    m_cardScores[5] = ui->cardScore5;

    // 统计数据的数值标签
    m_countLabels[0] = ui->clearCountLabel;
    m_countLabels[1] = ui->fcCountLabel;
    m_countLabels[2] = ui->apCountLabel;

    applyStyles();
    connectSignals();

    // 安装事件过滤器 拦截头像和昵称标签的鼠标点击
    ui->avatarLabel->installEventFilter(this);
    ui->nicknameLabel->installEventFilter(this);
}

ProfileWindow::~ProfileWindow()
{
    delete ui;
}

void ProfileWindow::setMainWindow(MainWindow *win)
{
    m_mainWin = win;
}

// QLabel 点击 → 触发对应槽
bool ProfileWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (obj == ui->avatarLabel) {
            onAvatarClicked();
            return true;
        }
        if (obj == ui->nicknameLabel) {
            switchToEditMode();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// 给.ui中的控件应用代码样式
void ProfileWindow::applyStyles()
{
    // 头像 优先加载用户自定义头像 否则使用默认
    QString savedAvatarPath = m_settings->value("profile/avatar_path").toString();
    QPixmap avatarPix;
    bool hasCustomAvatar = false;
    if (!savedAvatarPath.isEmpty() && QFile::exists(savedAvatarPath)) {
        avatarPix.load(savedAvatarPath);
        if (!avatarPix.isNull()) {
            hasCustomAvatar = true;
        }
    }
    if (avatarPix.isNull()) {
        avatarPix.load(":/img/default_avatar.png");
    }
    if (!avatarPix.isNull()) {
        // 确保头像完全填满
        QPixmap scaled = avatarPix.scaled(186, 186,
                                          Qt::KeepAspectRatioByExpanding,
                                          Qt::SmoothTransformation);
        ui->avatarLabel->setPixmap(scaled);
    }
    // 若加载了自定义头像，清除默认背景图避免从边缘透出
    if (hasCustomAvatar) {
        ui->avatarLabel->setStyleSheet("background: transparent; border: none;");
    }

    // 初始隐藏昵称编辑框，显示昵称标签
    ui->nicknameEdit->hide();
    ui->nicknameLabel->show();

    // 返回按钮悬浮于顶层
    ui->backButton->raise();
}

// 信号槽连接
void ProfileWindow::connectSignals()
{
    // 返回按钮
    connect(ui->backButton, &QPushButton::clicked,
            this, &ProfileWindow::on_backButton_clicked);

    // 昵称编辑框完成编辑（回车键）
    connect(ui->nicknameEdit, &QLineEdit::returnPressed,
            this, &ProfileWindow::onNicknameEditFinished);

    // 昵称编辑框失去焦点（点击空白处）→ 自动保存
    connect(ui->nicknameEdit, &QLineEdit::editingFinished,
            this, [this]() {
                if (ui->nicknameEdit->isVisible()) {
                    switchToDisplayMode();
                }
            });
}

// 背景绘制
void ProfileWindow::paintEvent(QPaintEvent *event)
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

// 窗口大小变化时更新左右面板位置
void ProfileWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateOverlayPos();
    update();
}

// 每次显示时加载最新数据
void ProfileWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    updateOverlayPos();
    loadProfileData();  
    loadPlayHistory();  
    refreshCardDisplay();
}

// 更新左侧面板和右侧区域的位置与尺寸
void ProfileWindow::updateOverlayPos()
{
    QWidget *cw = ui->centralwidget;
    if (!cw) return;

    int cwWidth = cw->width();
    int cwHeight = cw->height();

    int panelHeight = static_cast<int>(cwHeight * 0.8);
    int panelY = (cwHeight - panelHeight) / 2 + 5;

    int leftPanelWidth = static_cast<int>(cwWidth / 3.0);
    int leftPanelX = 20;

    int rightAreaWidth = cwWidth - leftPanelWidth - 60;
    int rightAreaX = leftPanelX + leftPanelWidth + 20;

    if (ui->leftPanelFrame) {
        ui->leftPanelFrame->setGeometry(leftPanelX, panelY, leftPanelWidth, panelHeight);
        ui->leftPanelFrame->raise();
    }

    if (ui->rightAreaWidget) {
        ui->rightAreaWidget->setGeometry(rightAreaX, panelY, rightAreaWidth, panelHeight);
        ui->rightAreaWidget->raise();
    }

    if (ui->backButton) {
        ui->backButton->raise();
    }
}

// 返回主菜单
void ProfileWindow::on_backButton_clicked()
{
    m_click4Sfx->play();
    qDebug() << "profile backButton is clicked";
    if (m_mainWin) {
        m_mainWin->show();
    }
    // 延迟关闭，确保音效播放完毕（不hide避免打断音频线程）
    QTimer::singleShot(300, this, &QWidget::deleteLater);
}

// 昵称编辑：标签→输入框
void ProfileWindow::switchToEditMode()
{
    qDebug() << "nicknameLabel clicked — switch to edit mode";
    QString currentName = ui->nicknameLabel->text();
    ui->nicknameEdit->setText(currentName);
    ui->nicknameLabel->hide();
    ui->nicknameEdit->show();
    ui->nicknameEdit->setFocus();
    ui->nicknameEdit->selectAll();
}

// 昵称编辑：输入框→标签（保存并刷新）
void ProfileWindow::switchToDisplayMode()
{
    QString newName = ui->nicknameEdit->text().trimmed();
    if (newName.isEmpty()) {
        newName = "暂无昵称"; // 不允许空昵称
    }
    qDebug() << "nicknameEdit finished — save:" << newName;

    ui->nicknameLabel->setText(newName);
    saveNickname(newName);

    ui->nicknameEdit->hide();
    ui->nicknameLabel->show();
}

void ProfileWindow::onNicknameEditFinished()
{
    // 回车键触发
    switchToDisplayMode();
}

// 点击头像→打开文件对话框更换头像
void ProfileWindow::onAvatarClicked()
{
    qDebug() << "avatar clicked — open file dialog";
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择头像图片",
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (filePath.isEmpty()) {
        qDebug() << "avatar selection cancelled";
        return;
    }

    QPixmap newAvatar(filePath);
    if (newAvatar.isNull()) {
        qDebug() << "failed to load avatar image:" << filePath;
        return;
    }

    // 缩放并设置头像
    QPixmap scaled = newAvatar.scaled(186, 186,
                                      Qt::KeepAspectRatioByExpanding,
                                      Qt::SmoothTransformation);
    ui->avatarLabel->setPixmap(scaled);
    // 清除 QLabel 默认背景图，避免从边缘透出
    ui->avatarLabel->setStyleSheet("background: transparent; border: none;");
    ui->avatarLabel->update(); // 强制立即重绘
    saveAvatarPath(filePath);

    qDebug() << "avatar changed to:" << filePath;
}

// 从 QSettings 加载昵称和统计数据
void ProfileWindow::loadProfileData()
{
    // 加载昵称
    QString nickname = m_settings->value("profile/nickname", "暂无昵称").toString();
    ui->nicknameLabel->setText(nickname);
    qDebug() << "loaded nickname:" << nickname;

    // 加载统计数据
    int clearCount = m_settings->value("profile/clear_count", 0).toInt();
    int fcCount    = m_settings->value("profile/fc_count", 0).toInt();
    int apCount    = m_settings->value("profile/ap_count", 0).toInt();

    m_countLabels[0]->setText(QString::number(clearCount));
    m_countLabels[1]->setText(QString::number(fcCount));
    m_countLabels[2]->setText(QString::number(apCount));

    qDebug() << "loaded stats — Clear:" << clearCount
             << "FC:" << fcCount << "AP:" << apCount;
}

// 保存昵称到 QSettings
void ProfileWindow::saveNickname(const QString &name)
{
    m_settings->setValue("profile/nickname", name);
    qDebug() << "saved nickname:" << name;
}

// 保存头像路径到 QSettings
void ProfileWindow::saveAvatarPath(const QString &path)
{
    m_settings->setValue("profile/avatar_path", path);
    qDebug() << "saved avatar path:" << path;
}

// 从 data/play_history.json 加载游玩记录
void ProfileWindow::loadPlayHistory()
{
    m_playHistory.clear();

    QString jsonPath = QCoreApplication::applicationDirPath() + "/data/play_history.json";
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "play_history.json not found, starting with empty history";
        return;
    }

    QByteArray rawData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(rawData);
    if (!doc.isArray()) {
        qDebug() << "play_history.json format error";
        return;
    }

    QJsonArray arr = doc.array();
    for (const auto &val : arr) {
        QJsonObject obj = val.toObject();
        PlayRecord rec;
        rec.songId    = obj["songId"].toString();
        rec.songName  = obj["songName"].toString();
        rec.score     = obj["score"].toInt();
        rec.coverPath = obj["coverPath"].toString();
        rec.playTime  = obj["playTime"].toString();
        m_playHistory.append(rec);
    }

    qDebug() << "loaded" << m_playHistory.size() << "play records";
}

// 刷新右侧6张卡片显示
void ProfileWindow::refreshCardDisplay()
{
    int count = m_playHistory.size();

    for (int i = 0; i < 6; i++) {
        // 加载曲绘图：优先本地文件，失败则用默认占位
        QPixmap coverPix;
        if (i < count) {
            const PlayRecord &rec = m_playHistory[i];
            // 兼容旧绝对路径 + 新相对路径
            QString resolvedPath = rec.coverPath;
            if (QDir::isRelativePath(resolvedPath)) {
                resolvedPath = QCoreApplication::applicationDirPath() + "/" + resolvedPath;
            }
            if (!rec.coverPath.isEmpty() && QFile::exists(resolvedPath)) {
                coverPix.load(resolvedPath);
                qDebug() << "loading cover:" << resolvedPath
                         << "success:" << !coverPix.isNull();
            }
        }
        if (coverPix.isNull()) {
            coverPix.load(":/img/song_placeholder.jpg");
        }

        // 曲绘+灰色半透明蒙版 合成为一张图，设到cardMask上
        const int cardW = 251;
        const int cardH = 278;
        QPixmap result(cardW, cardH);
        result.fill(Qt::transparent);
        QPainter p(&result);
        if (!coverPix.isNull()) {
            // 等比放大填满卡片
            QPixmap scaled = coverPix.scaled(cardW, cardH,
                                             Qt::KeepAspectRatioByExpanding,
                                             Qt::SmoothTransformation);
            // 居中绘制
            int sx = (cardW - scaled.width()) / 2;
            int sy = (cardH - scaled.height()) / 2;
            p.drawPixmap(sx, sy, scaled);
        }
        // 覆盖灰色半透明蒙版
        p.fillRect(0, 0, cardW, cardH, QColor(50, 50, 50, 178));
        p.end();
        m_cardMasks[i]->setPixmap(result);
        m_cardMasks[i]->setScaledContents(true);

        // 清除 QFrame 旧 background-image，避免与 mask 冲突
        m_cardFrames[i]->setStyleSheet(
            QString("#%1 { border-radius: 4px; }").arg(m_cardFrames[i]->objectName()));

        // 设置文字
        if (i < count) {
            const PlayRecord &rec = m_playHistory[i];
            m_cardSongNames[i]->setText(rec.songName);
            m_cardScores[i]->setText(QString("%1").arg(rec.score, 7, 10, QChar('0')));
        } else {
            m_cardSongNames[i]->setText("暂无记录");
            m_cardScores[i]->setText("0000000");
        }
    }
}
