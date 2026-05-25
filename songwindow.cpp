#include "songwindow.h"
#include "ui_playwindow.h"
#include "ui_songwindow.h"
#include "mainwindow.h"
#include "playwindow.h"
#include "gamemanager.h"

SongWindow::SongWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui(new Ui::SongWindow)
    , m_mainWin(nullptr)
{
    ui->setupUi(this);
    ui->startButton->hide();

    loadCharts();
}

SongWindow::~SongWindow()
{
    delete ui;
}

void SongWindow::on_backButton_clicked()
{
    qDebug() << "backButton is clicked";

    if (m_mainWin) {
        m_mainWin->show(); // 显示原来的主窗口
    }

    this->close(); //关闭自己
    this->deleteLater(); // 安全释放自己
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
    QDirIterator it(chartsRoot, nameFilters, QDir::Files,
                    QDirIterator::Subdirectories);

    QMap<QString, QJsonObject> chartData;
    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        QString relativePath = rootDir.relativeFilePath(fileInfo.absoluteFilePath());
        QString absolutePath = fileInfo.absoluteFilePath();

        // 读取 .json 文件内的歌曲标题与难度信息
        QMap<QString, QJsonObject> loadedData;
        QJsonObject jsonObj = loadJsonFile(absolutePath);
        if (jsonObj.isEmpty()) {
            continue;    // 读取失败或格式错误，跳过该文件
        }
        loadedData.insert(absolutePath, jsonObj);  // 保存数据

        // 从 JSON 中提取按钮文本
        QJsonObject metaObj = jsonObj.value("meta").toObject();
        QJsonObject songObj = metaObj.value("song").toObject();
        QString btnText = songObj.value("title").toString();
        if (btnText.isEmpty()) {
            btnText = relativePath;   // 如果 JSON 中没有 name，回退为相对路径
        }

        // 创建按钮 设置按钮样式
        QPushButton *btn = new QPushButton(btnText, container);
        btn->setMinimumHeight(100);
        // btn->setStyleSheet(R"(QPushButton:hover {border: 1px solid #5078a0;border-radius: 2px;})");

        // 连接点击信号
        connect(btn, &QPushButton::clicked, this, [this, absolutePath, btnText, jsonObj]() {
            qDebug() << "Clicked:" << btnText;
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
}

QJsonObject SongWindow::loadJsonFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << path;
        return QJsonObject();   // 返回空对象表示失败
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
    // 1. 处理封面图片
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
        coverLabel->setPixmap(QPixmap());   // 清空图片
    }

    // 2. 提取并显示歌曲信息
    QJsonObject metaObj = jsonObj.value("meta").toObject();
    QJsonObject songObj = metaObj.value("song").toObject();

    QString title = songObj.value("title").toString("unknown");
    QString artist = songObj.value("artist").toString("unknown");
    QString difficulty = songObj.value("difficulty").toString("?");
    // 可根据需要继续提取其他字段

    QString infoHtml = QString(
                           "<table width='100%' cellspacing='0' cellpadding='0' style='color:white;'>"
                           "<tr>"
                           // 左侧 歌曲标题与艺术家
                           "<td valign='bottom' style='padding-right:10px;'>"
                           "<b style='font-size:24px;'>%1</b><br>"
                           "<span style='color:#ccc; font-size:14px;'>%2</span>"
                           "</td>"
                           // 右侧 难度
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
        play->getUI()->labelScore->setText(QString("Score: %1").arg(score));
        });
    connect(gameManager, &GameManager::comboChanged, this, [this, play](int combo) {
        play->getUI()->labelCombo->setText(QString("Combo: %1").arg(combo));
        });
    connect(gameManager, &GameManager::judgementResult, this, [this, play](const QString &text, const QColor &color) {
        play->getUI()->labelJudgement->setText(text);
        play->getUI()->labelJudgement->setStyleSheet(QString("color: %1; font-size: 28px;").arg(color.name()));
        });

    play->show();
    this->hide();
    QTimer::singleShot(2000, play, &PlayWindow::startGame); // 等待2s再开始游戏
}