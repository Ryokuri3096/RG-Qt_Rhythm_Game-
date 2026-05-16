#include "songwindow.h"
#include "ui_songwindow.h"
#include "mainwindow.h"

SongWindow::SongWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui(new Ui::SongWindow)
    , m_mainWin(nullptr)
{
    ui->setupUi(this);

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
    // 1. 确定 charts 根目录（与 exe 同级）
    QString chartsRoot = QCoreApplication::applicationDirPath() + "/charts";
    QDir rootDir(chartsRoot);
    if (!rootDir.exists()) {
        qDebug() << "Charts directory not found:" << chartsRoot;
        return;
    }

    // 2. 准备容器和布局
    QWidget *container = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(4);

    // 3. 递归遍历所有 .json 文件
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

        // ---- 从 JSON 中提取按钮文本 ----
        QJsonObject metaObj = jsonObj.value("meta").toObject();
        QJsonObject songObj = metaObj.value("song").toObject();
        QString btnText = songObj.value("title").toString();
        if (btnText.isEmpty()) {
            btnText = relativePath;   // 如果 JSON 中没有 name，回退为相对路径
        }

        // 创建按钮 设置按钮样式
        QPushButton *btn = new QPushButton(btnText, container);
        btn->setMinimumHeight(100);

        // 连接点击信号（示例：打印绝对路径）
        connect(btn, &QPushButton::clicked, this, [absolutePath, btnText]() {
            qDebug() << "Clicked:" << btnText;
            // 在这里可以打开文件、加载图表等
        });

        layout->addWidget(btn);
    }

    // 加个弹簧，按钮不会拉伸占满
    layout->addStretch();

    // 4. 设置到 QScrollArea
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

