#include "songwindow.h"
#include "ui_playwindow.h"
#include "ui_songwindow.h"
#include "mainwindow.h"
#include "playwindow.h"
#include "gamemanager.h"
#include <QTimer>

SongWindow::SongWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui(new Ui::SongWindow)
    , m_mainWin(nullptr)
{
    ui->setupUi(this);
    ui->startButton->hide();

    // 加载背景图
    m_background.load(":/img/play_bp.jpg");

    // 初始化按钮音效（不受sfxVolume控制）
    m_click1Sfx = new QSoundEffect(this);
    m_click1Sfx->setSource(QUrl("qrc:/sfx/click1.wav"));
    m_click4Sfx = new QSoundEffect(this);
    m_click4Sfx->setSource(QUrl("qrc:/sfx/click4.wav"));

    loadCharts();
}

SongWindow::~SongWindow()
{
    delete ui;
}

// ── 背景绘制 ──
void SongWindow::paintEvent(QPaintEvent *event)
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

void SongWindow::on_backButton_clicked()
{
    m_click4Sfx->play();
    qDebug() << "backButton is clicked";

    if (m_mainWin) {
        m_mainWin->show(); // 显示原来的主窗口
    }

    // 延迟关闭，确保音效播放完毕
    QTimer::singleShot(200, this, &QWidget::deleteLater);
}

// 保存主窗口指针的函数
void SongWindow::setMainWindow(MainWindow *win)
{
    m_mainWin = win;
}

void SongWindow::loadCharts()
{
    // 确定 charts 根目录
    QString chartsRoot = QCoreApplication::applicationDirPath() + "/charts";
    QDir rootDir(chartsRoot);
    if (!rootDir.exists()) {
        qDebug() << "Charts directory not found:" << chartsRoot;
        return;
    }

    // 准备容器和布局
    QWidget *container = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(4);

    // 递归遍历所有 .json 文件
    QStringList nameFilters;
    nameFilters << "*.json";
    QDirIterator it(chartsRoot, nameFilters, QDir::Files, QDirIterator::Subdirectories);

    QString firstSongPath;      // 第一首歌的路径
    QJsonObject firstSongJson;  // 第一首歌的JSON数据
    bool isFirst = true;

    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        QString relativePath = rootDir.relativeFilePath(fileInfo.absoluteFilePath());
        QString absolutePath = fileInfo.absoluteFilePath();

        // 读取 .json 文件内的歌曲标题与难度信息
        QMap<QString, QJsonObject> loadedData;
        QJsonObject jsonObj = loadJsonFile(absolutePath);
        if (jsonObj.isEmpty()) {
            continue; // 读取失败或格式错误，跳过该文件
        }
        loadedData.insert(absolutePath, jsonObj); // 保存数据

        // 记录第一首有效歌曲，用于默认展示
        if (isFirst) {
            firstSongPath = absolutePath;
            firstSongJson = jsonObj;
            isFirst = false;
        }

        // 从 JSON 中提取按钮文本和等级
        QJsonObject metaObj = jsonObj.value("meta").toObject();
        QJsonObject songObj = metaObj.value("song").toObject();
        QString songTitle = songObj.value("title").toString();
        if (songTitle.isEmpty()) {
            songTitle = relativePath; // 如果 JSON 中没有 name，回退为相对路径
        }
        QString difficulty = songObj.value("difficulty").toString("?");

        // 创建按钮：内嵌 layout，曲名居左 + 等级居右
        QPushButton *btn = new QPushButton(container);
        btn->setMinimumHeight(90);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet("QPushButton {"
                           "  background: rgba(255,255,255,15);"
                           "  border: 1px solid rgba(255,255,255,40);"
                           "  border-radius: 6px;"
                           "}"
                           "QPushButton:hover {"
                           "  background: rgba(255,255,255,40);"
                           "  border: 1px solid rgba(255,255,255,100);"
                           "}");

        QHBoxLayout *btnLayout = new QHBoxLayout(btn);
        btnLayout->setContentsMargins(14, 0, 14, 0);

        QLabel *titleLbl = new QLabel(songTitle, btn);
        titleLbl->setStyleSheet("color: white; font-size: 20px; font-family: 'Exo'; background: transparent; border: none;");
        QLabel *diffLbl = new QLabel(QString("Lv.%1").arg(difficulty), btn);
        diffLbl->setStyleSheet("color: #88bbee; font-size: 20px; font-family: 'Exo'; background: transparent; border: none;");

        btnLayout->addWidget(titleLbl);
        btnLayout->addStretch();
        btnLayout->addWidget(diffLbl);

        // 连接点击信号
        connect(btn, &QPushButton::clicked, this, [this, absolutePath, songTitle, jsonObj]() {
            m_click4Sfx->play();
            qDebug() << "Clicked:" << songTitle;
            updateLeftPanel(absolutePath, jsonObj);
            ui->startButton->show();
            songPath = absolutePath;
        });

        layout->addWidget(btn);
    }

    // 加个弹簧 按钮不会拉伸占满
    layout->addStretch();

    // 设置到 QScrollArea
    ui->chooseArea->setWidgetResizable(true);
    ui->chooseArea->setWidget(container);

    // 默认展示第一首歌曲的简介和曲绘
    if (!firstSongPath.isEmpty()) {
        updateLeftPanel(firstSongPath, firstSongJson);
        ui->startButton->show();
        songPath = firstSongPath;
    }
}

QJsonObject SongWindow::loadJsonFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << path;
        return QJsonObject(); // 返回空对象表示失败
    }

    QByteArray rawData = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Invalid JSON in:" << path << "-" << error.errorString();
        return QJsonObject();
    }

    return doc.object();
}

QString SongWindow::findCoverImage(const QString &dirPath)
{
    QDir dir(dirPath);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
    QFileInfoList images = dir.entryInfoList(filters, QDir::Files);
    if (!images.isEmpty())
        return images.first().absoluteFilePath();
    return QString();
}

void SongWindow::updateLeftPanel(const QString &jsonPath, const QJsonObject &jsonObj)
{
    //  处理曲绘
    QFileInfo jsonFileInfo(jsonPath);
    QString dirPath = jsonFileInfo.absolutePath();
    QString imagePath = findCoverImage(dirPath);

    auto *coverLabel = ui->coverLabel;
    auto *infoLabel = ui->infoLabel;

    if (!imagePath.isEmpty()) {
        QPixmap cover(imagePath);
        coverLabel->setPixmap(cover.scaled(coverLabel->width(),
                                           coverLabel->height(),
                                           Qt::IgnoreAspectRatio, // 强制拉伸 把曲绘变成正方形
                                           Qt::SmoothTransformation));
        ui->coverLabel->setStyleSheet("border: 4px solid #360054;");
    } else {
        coverLabel->setPixmap(QPixmap()); // 清空图片
    }

    // 提取并显示歌曲信息
    QJsonObject metaObj = jsonObj.value("meta").toObject();
    QJsonObject songObj = metaObj.value("song").toObject();

    QString title = songObj.value("title").toString("unknown");
    QString artist = songObj.value("artist").toString("unknown");
    QString difficulty = songObj.value("difficulty").toString("?");

    QString infoHtml = QString(
                           "<table width='100%' cellspacing='0' cellpadding='0' style='color:white;'>"
                           "<tr>"
                           // 歌曲标题与艺术家
                           "<td valign='bottom' style='padding-right:10px;'>"
                           "<b style='font-size:24px;'>%1</b><br>"
                           "<span style='color:#ccc; font-size:14px;'>%2</span>"
                           "</td>"
                           // 难度
                           "<td align='right' valign='bottom'>"
                           "<span style='font-size:32px; font-weight:bold;'>Lv.%3</span>"
                           "</td>"
                           "</tr>"
                           "</table>"
                           ).arg(title, artist, difficulty);

    infoLabel->setText(infoHtml);
    infoLabel->setWordWrap(true);
}

void SongWindow::on_startButton_clicked()
{
    m_click1Sfx->play();
    // 创建游戏管理器
    GameManager *gameManager = new GameManager(this);
    gameManager->reset();

    // 创建游戏窗口
    PlayWindow *play = new PlayWindow;
    play->loadChart(songPath, *gameManager);

    // 连接判定信号
    connect(play, &PlayWindow::judgement, gameManager, &GameManager::onJudgement);

    // 连接UI更新信号
    connect(gameManager, &GameManager::scoreChanged, this, [this, play](int score) {
        play->getUI()->labelScore->setText(QString("%1").arg(score));
        });
    connect(gameManager, &GameManager::comboChanged, this, [this, play](int combo) {
        play->getUI()->labelCombo->setText(QString("<span style='font-size: 22px; color: white;'>Combo</span><br>"
                                                   "<span style='font-size: 32px; color: white;'>%1</span>").arg(combo));
        });
    connect(gameManager, &GameManager::accuracyChanged, this, [this, play](double accuracy) {
        play->getUI()->labelAccuracy->setText(QString("%1%").arg(QString::number(accuracy, 'f', 4)));
        });
    connect(gameManager, &GameManager::judgementResult, this, [this, play](const QString &text, GameManager *gm) {
        judgementManage(play, text, gm);
        });

    // 连接Restart信号
    connect(play, &PlayWindow::restartRequested, gameManager, &GameManager::reset);

    // 连接游戏结束
    connect(play, &PlayWindow::gameFinished, this, [this, play, gameManager]() {
        QTimer::singleShot(1000, this, [play, gameManager]() {
            play->showResult(gameManager);   // 直接在 play 上显示覆盖层
        });
    });

    play->show();
    this->hide();

    connect(play, &PlayWindow::returnToMenu, this, &QWidget::show);
    QTimer::singleShot(2000, play, &PlayWindow::startGame); // 等待2s再开始游戏
}

void SongWindow::judgementManage(PlayWindow *play, const QString &text, GameManager *gm)
{
    play->shakeLabelJudgement();

    QString mystery("▪");
    QString textShowed = mystery + text + mystery; // 神秘符号装饰
    play->getUI()->labelJudgement->setText(textShowed);
    if (text == "PERFECT")
        play->getUI()->labelJudgement->setStyleSheet(QString("color: qlineargradient(x1:0, x2:0, y1:1, y2:0, stop:0 #FF7A00, stop:1 #FFD700); font-family: Exo;"));
    else if (text == "GREAT")
        play->getUI()->labelJudgement->setStyleSheet(QString("color: qlineargradient(x1:0, x2:0, y1:1, y2:0, stop:0 #FF1493, stop:1 #FFB6C1); font-family: Exo;"));
    else if (text == "GOOD")
        play->getUI()->labelJudgement->setStyleSheet(QString("color: qlineargradient(x1:0, x2:0, y1:1, y2:0, stop:0 #008000, stop:1 #90EE90); font-family: Exo;"));
    else
        play->getUI()->labelJudgement->setStyleSheet(QString("color: qlineargradient(x1:0, x2:0, y1:1, y2:0, stop:0 #696969, stop:1 #D3D3D3); font-family: Exo;"));
    play->getUI()->labelNote->setText(QString("<span style='color:white;'>%1</span><br>"
                                    "<span style='color:white;'>%2</span><br>"
                                    "<span style='color:white;'>%3</span><br>"
                                    "<span style='color:white;'>%4</span><br>"
                                              "<span style='color:white;'>%5</span>").arg(gm->maxCombo()).arg(gm->perfectCount()).arg(gm->greatCount()).arg(gm->goodCount()).arg(gm->missCount()));
}