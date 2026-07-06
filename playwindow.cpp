#include "playwindow.h"
#include "ui_playwindow.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

PlayWindow::PlayWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PlayWindow)
{
    ui->setupUi(this);

    setFocusPolicy(Qt::StrongFocus); // 设置焦点确保能接收到键盘事件

    m_keyLaneMap[Qt::Key_S] = 0;
    m_keyLaneMap[Qt::Key_D] = 1;
    m_keyLaneMap[Qt::Key_J] = 2;
    m_keyLaneMap[Qt::Key_K] = 3;
    ui->centralwidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    this->setMouseTracking(true); // 把所有鼠标事件对接到PlayWindow 并且设置鼠标随时跟踪

    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput); // 音频播放器

    m_gameTimer = new QTimer(this); // 循环定时器
    connect(m_gameTimer, &QTimer::timeout, this, &PlayWindow::gameLoop);

    // 初始化音效
    m_tapSfx = new QSoundEffect(this);
    m_tapSfx->setSource(QUrl("qrc:/sfx/tap.wav"));
    m_flickSfx = new QSoundEffect(this);
    m_flickSfx->setSource(QUrl("qrc:/sfx/flick.wav"));
    m_click1Sfx = new QSoundEffect(this);
    m_click1Sfx->setSource(QUrl("qrc:/sfx/click1.wav"));
    m_click2Sfx = new QSoundEffect(this);

m_click2Sfx->setSource(QUrl("qrc:/sfx/click2.wav"));

    m_judgementOrigin = ui->labelJudgement->pos();

    // 加载游戏背景图
    m_background.load(":/img/play_bp.jpg");
}

PlayWindow::~PlayWindow()
{
    delete ui;
}

Ui::PlayWindow *PlayWindow::getUI()
{
    return ui;
}

void PlayWindow::loadChart(const QString &chartPath, GameManager &gameManager)
{
    loadGui();

    m_chartPath = chartPath; // 保存谱面路径，用于结算时生成 songId
    QFile file(chartPath); // 打开谱面文件
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Cannot open chart file!");
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    m_tapBlue.load(":/img/note_blue.png");
    m_holdBodyBlue.load(":/img/hold_blue.png");
    m_tapRed.load(":/img/note_red.png");
    m_holdBodyRed.load(":/img/hold_red.png");

    // 获取曲绘 乐曲名 作者 难度并显示
    QJsonObject metaObj = root["meta"].toObject();
    QJsonObject songObj = metaObj.value("song").toObject();
    m_songTitle = metaObj["song"].toObject()["title"].toString();
    m_songArtist = metaObj["song"].toObject()["artist"].toString();
    ui->labelTitle->setText(m_songTitle + " / " +m_songArtist);
    ui->labelTitle->hide();
    m_difficulty = songObj["difficulty"].toString();
    QString bgFile = metaObj["background"].toString();
    m_coverPath = QFileInfo(chartPath).absolutePath() + "/" + bgFile;

    // 获取音频文件名
    QString audioPath = QFileInfo(chartPath).dir().path();
    QString audioFileName;
    QDir dir(audioPath);
    QStringList filters; filters << "*ogg";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    if(dir.count() > 0) {
        audioFileName = dir[0];
        qDebug() << "Find audio files named" << audioFileName;
    } else {
        qDebug() << "Cannot find audio files in " << audioPath;
    }

    // 解析divide和speed
    QJsonObject extra = root["extra"].toObject();
    QJsonObject test = extra["test"].toObject();
    if (test.contains("divide")) {
        m_divide = test["divide"].toInt();
    }
    if (test.contains("speed")) {
        m_speedFactor = test["speed"].toDouble() / 100.0;
    }

    // 解析time 获取bpm
    QJsonArray timeArray = root["time"].toArray();
    if (!timeArray.isEmpty()) {
        QJsonObject firstTime = timeArray[0].toObject();
        m_bpm = firstTime["bpm"].toDouble(130.0);  // 默认130
    }

    // 将拍子数转换成ms的内联函数
    auto beatToMs = [this](const QJsonArray &beat) -> double {
        int bar = beat[0].toInt(); // 第bar+1个节拍
        int beatIdx = beat[1].toInt(); // 这第bar+1个节拍里的第beatIdx+1个小拍
        int sub = beat[2].toInt(); // 一个节拍里有几个小拍
        double totalBeats = bar + (double)beatIdx / (double)sub;
        return totalBeats * (60000.0 / m_bpm);
    };

    // 解析note
    double offset = 0;
    m_notes.clear();
    QJsonArray noteArray = root["note"].toArray();
    for (const auto &val : noteArray) {
        gameManager.addAllNoteNum();

        QJsonObject obj = val.toObject();
        QJsonArray beat = obj["beat"].toArray();
        int lane = obj["column"].toInt();

        NoteData nd;
        nd.timeMs = beatToMs(beat);
        nd.lane = lane;

        // 分辨note类型
        if (obj.contains("endbeat")) {
            QJsonArray endbeat = obj["endbeat"].toArray();
            nd.endTimeMs = beatToMs(endbeat);
            // qDebug() << nd.timeMs << ':' << nd.endTimeMs;
            gameManager.addAllNoteNum(); // Hold分头部和身体两次判定 算作两个note
            nd.type = NoteData::HOLD;
        } else if (lane == 4) {
            nd.type = NoteData::FLICK;
        } else {
            nd.type = NoteData::TAP;
        }

        if (obj.contains("type") && obj["type"].toInt() == 1) {
            offset = obj["offset"].toDouble(); // 获取offset
            continue;
        }

        m_notes.push_back({nd, false, false, false}); // 将解析出来的note加入储存所有note的vector
    }

    for (auto& note : m_notes) {
        note.data.timeMs -= offset; // 对所有note应用offset
        if (note.data.endTimeMs > 0) note.data.endTimeMs -= offset;
    }

    std::sort(m_notes.begin(), m_notes.end(), // 把note按时间排序
              [](const GameNote &a, const GameNote &b){
        return a.data.timeMs < b.data.timeMs;
    });

    qDebug() << audioPath + '/' + audioFileName;
    m_player->setSource(QUrl::fromLocalFile(audioPath + '/' + audioFileName)); // 把音乐加入播放器
    loadGui();
}

void PlayWindow::loadSettings()
{
    QSettings settings("RG", "Settings");

    // 从 settingswindow 读取音乐音量
    int vol = settings.value("audio/musicVolume", 100).toInt();
    m_audioOutput->setVolume(vol / 100.0);

    // 读取流速
    int spd = settings.value("gameplay/speed", 100).toInt();
    m_baseSpeed = spd / 100.0;

    // 音效音量只控制音符音效
    double sfxVol = settings.value("audio/sfxVolume", 100).toInt() / 100.0;
    m_tapSfx->setVolume(sfxVol);
    m_flickSfx->setVolume(sfxVol);

    // 读取键位映射
    int defaultKeys[4] = {Qt::Key_S, Qt::Key_D, Qt::Key_J, Qt::Key_K};
    m_keyLaneMap.clear();
    for (int i = 0; i < 4; i++) {
        int key = settings.value(QString("gameplay/keyMapping%1").arg(i), defaultKeys[i]).toInt();
        m_keyLaneMap[key] = i;
    }

    qDebug() << "PlayWindow loadSettings - Vol:" << vol << "Speed:" << m_baseSpeed;
}

void PlayWindow::startGame()
{
    if (m_notes.empty()) return;

    // 开局前加载用户设置
    loadSettings();

    // 游戏初始化
    m_gameStarted = true;
    m_elapsed.start();
    m_musicStartOffset = 0;
    this->grabMouse(); // 让窗口独占鼠标事件

    for (auto &note : m_notes) {
        note.missed = false;
        note.holding = false;
        note.judged = false;
    }

    m_activeHolds.clear();
    m_activeFlickLanes.clear();
    m_lastMousePos = mapFromGlobal(QCursor::pos());

    m_player->setPosition(0);
    m_player->play();

    m_gameTimer->start(1000 / m_fps);
}

qint64 PlayWindow::currentMusicTime() const
{
    return m_elapsed.elapsed() + m_musicStartOffset;
}

void PlayWindow::gameLoop()
{
    qint64 curTime = currentMusicTime();

    m_activeFlickLanes.clear();
    // 寻找进入判定窗口200ms的Flick
    for (auto &note : m_notes) {
        if (note.data.type == NoteData::FLICK && !note.missed && !note.judged) {
            // +-200ms
            if (curTime >= note.data.timeMs - 200 && curTime <= note.data.timeMs + 200){
                m_activeFlickLanes.insert(note.data.lane);
            }
        }
    }

    updateEffects();
    updateMissedNotes(); // 判定应该miss的note

    update(); // 重绘
}

void PlayWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // 最底层背景图
    if (!m_background.isNull()) {
        QPixmap scaled = m_background.scaled(size(),
                                              Qt::KeepAspectRatioByExpanding,
                                              Qt::SmoothTransformation);
        int bx = (width() - scaled.width()) / 2;
        int by = (height() - scaled.height()) / 2;
        p.drawPixmap(bx, by, scaled);
    }

    if (m_showResult) {
        p.fillRect(rect(), QColor(20, 20, 30));
        return;
    }

    int w = width();
    int h = height();

    // 轨道宽度定义
    const int leftLaneW = 80;
    const int rightLaneW = leftLaneW * 2;
    const int totalW = 4 * leftLaneW + rightLaneW;
    const int offsetX = (w - totalW) / 2;

    m_hitLineY = h - 120; // 判定线高度

    qint64 curTime = currentMusicTime();
    double speed = m_baseSpeed * m_speedFactor; // 像素/毫秒

    // 绘制轨道背景
    QColor lane01Color(0, 21, 35);
    p.fillRect(QRect(offsetX, 0, leftLaneW * 2, h), lane01Color);
    QColor lane23Color(39, 0, 23);
    p.fillRect(QRect(offsetX + leftLaneW * 2, 0, leftLaneW * 2, h), lane23Color);
    QColor lane4Color(59, 14, 14);
    p.fillRect(QRect(offsetX + leftLaneW * 4, 0, rightLaneW, h), lane4Color);
    p.fillRect(QRect(offsetX, m_hitLineY, leftLaneW * 6, h), QColor(39, 39, 39));

    // 绘制轨道线
    p.setPen(QPen(QColor(255, 255, 255, 80), 3));
    for (int i = 0; i <= 4; ++i) {
        int x = offsetX + i * leftLaneW;
        p.setPen(QPen(QColor(255, 255, 255, 80), 3));
        if (i == 1)
            p.setPen(QPen(QColor(5, 56, 99, 80), 3));
        else if (i == 2)
            p.setPen(QPen(QColor(68, 53, 122, 80), 3));
        else if (i == 3)
            p.setPen(QPen(QColor(95, 2, 59, 80), 3));
        else
            p.setPen(QPen(QColor(255, 255, 255, 80), 3));
        p.drawLine(x, 0, x, m_hitLineY);
        if (i == 0 || i == 4) {
            p.setPen(QPen(QColor(255, 255, 255, 80), 3));
            p.drawLine(x, m_hitLineY, x, h);
        } else {
            p.setPen(QPen(QColor(64, 64, 64, 80), 3));
            p.drawLine(x, m_hitLineY, x, h);
        }
    }
    // 最后一轨的右边界
    int xRight = offsetX + 4 * leftLaneW + rightLaneW;
    p.setPen(QPen(QColor(255, 255, 255, 80), 3));
    p.drawLine(xRight, 0, xRight, h);

    // 判定线
    p.setPen(QPen(Qt::yellow, 2));
    p.drawLine(offsetX, m_hitLineY, offsetX + totalW, m_hitLineY);

    // 绘制音符
    for (const auto &note : m_notes) {
        if (note.judged && !note.missed) continue;
        bool isDeadHold = false;
        if (note.missed) {
            if (note.data.type != NoteData::HOLD) continue;
            isDeadHold = true;
        }
        if (!isDeadHold && !note.judged && !note.holding
            && note.data.type == NoteData::HOLD
            && curTime > note.data.timeMs + 200) {
            isDeadHold = true;
        }

        double yOffset = (note.data.timeMs - curTime) * speed;
        int noteY = m_hitLineY - (int)yOffset;

        if (note.data.type != NoteData::HOLD) {
            if (noteY < -50 || noteY > height() + 50) continue;
        } else {
            double endYOffset = (note.data.endTimeMs - curTime) * speed;
            int endY = m_hitLineY - (int)endYOffset;
            int bodyTop = qMin(noteY, endY);
            int bodyBottom = qMax(noteY, endY);
            if (bodyBottom < -50 || bodyTop > height() + 50) continue; // 整个身体都不在屏幕内才跳过
        }

        int x, noteW;
        if (note.data.lane < 4) {
            x = offsetX + note.data.lane * leftLaneW;
            noteW = leftLaneW - 8;
        } else {
            x = offsetX + 4 * leftLaneW;
            noteW = rightLaneW - 8;
        }

        switch (note.data.type) {
        case NoteData::TAP: {
            QPixmap pic = tapPixmap(note.data.lane);
            QPixmap scaled = pic.scaled(noteW, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            p.drawPixmap(x + 4, noteY, scaled);
            break;
        }
        case NoteData::HOLD: {
            if (isDeadHold) {
                int endY = m_hitLineY - (int)((note.data.endTimeMs - curTime) * speed);
                int headY = note.wasHeld ? qMin(noteY, m_hitLineY) : noteY;
                int topY = qMin(headY, endY);
                int botY = qMax(headY, endY);
                const int headH = 20;

                QColor deadBody = (note.data.lane < 2) ? QColor(80, 90, 110, 180) : QColor(95, 55, 60, 180);
                int bodyTop = topY + headH;
                int bodyBot = botY - headH;
                if (bodyBot > bodyTop) {
                    p.fillRect(x + 4, bodyTop, noteW, bodyBot - bodyTop, deadBody);
                }
                QColor deadHead = (note.data.lane < 2) ? QColor(60, 65, 75, 220) : QColor(65, 45, 50, 220);
                p.fillRect(x + 4, topY, noteW, headH, deadHead);
                if (botY - topY > headH) {
                    p.fillRect(x + 4, botY - headH, noteW, headH, deadHead);
                }
            } else if (note.holding) {
                int headY = qMin(m_hitLineY, noteY);
                double endYOffset = (note.data.endTimeMs - curTime) * speed;
                int endY = m_hitLineY - (int)endYOffset;
                drawHold(p, note.data.lane, x, noteW, headY, endY, true);
            }
            else {
                int endY = m_hitLineY - (int)((note.data.endTimeMs - curTime) * speed);
                drawHold(p, note.data.lane, x, noteW, noteY, endY, false);
            }
            break;
        }
        case NoteData::FLICK: {
            QRectF rect(x + 4, noteY, noteW, 20);
            p.fillRect(rect, QColor(200, 0, 200));
            p.setPen(Qt::white);
            //p.drawText(rect, Qt::AlignCenter, "↕");
            break;
        }
        }
    }

    // 绘制打击特效
    for (const auto &e : m_effects) {
        qint64 elapsed = curTime - e.startTime;
        if (elapsed < 0 || elapsed > HitEffect::duration) continue;

        float progress = (float)elapsed / HitEffect::duration;
        float expand = 1.0f + progress; // 扩大倍数
        int alpha = 255 * (1.0f - progress);

        int x = laneX(e.lane); // 之前定义过的轨道坐标辅助函数
        int w = laneWidth(e.lane);
        int h = 18 + 2; // 音符高度为18

        QRectF baseRect(x + 4, m_hitLineY - h/2, w, h);
        QRectF expandedRect = baseRect.adjusted(-w*(expand-1)/2, -h*(expand-1)/2,
                                                w*(expand-1)/2, h*(expand-1)/2);
        QColor penColor = e.color;
        penColor.setAlpha(alpha);
        QPen pen(penColor, 3); // 颜色与画笔粗细
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRect(expandedRect);
    }
}

void PlayWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) { // 按Esc暂停
        if (!m_gameStarted) return;
        togglePause();
        m_click2Sfx->play();
        qDebug() << "Esc is pressed.";
        return;
    }
    if (m_paused) return; // 暂停时忽略其他按键

    if (event->isAutoRepeat()) return;
    if (!m_keyLaneMap.contains(event->key())) return;
    int lane = m_keyLaneMap[event->key()];
    checkTapHit(lane);
}

void PlayWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;
    if (!m_keyLaneMap.contains(event->key())) return;
    int lane = m_keyLaneMap[event->key()];
    checkHoldRelease(lane);
}

void PlayWindow::mouseMoveEvent(QMouseEvent *event)
{
    int deltaY = event->globalPos().y() - m_lastMousePos.y();
    m_lastMousePos = event->globalPos();
    // qDebug() << m_lastMousePos;
    // if (this->mouseGrabber() == this) qDebug()<<"grabed";
    if (qAbs(deltaY) < 10) return; // 鼠标纵向移动距离大于10px才判定
    checkFlickHit(deltaY);
}

void PlayWindow::checkTapHit(int lane)
{
    qint64 curTime = currentMusicTime();
    GameNote *bestNote = nullptr;
    qint64 minDiff = 1000000;

    // 寻找最近的note
    for (auto &note : m_notes) {
        if (note.missed || note.judged || note.data.lane != lane) continue;
        if (note.data.type == NoteData::FLICK) continue;

        qint64 diff = qAbs((qint64)(note.data.timeMs) - curTime);
        if (diff < 200 && diff < minDiff) {
            minDiff = diff;
            bestNote = &note;
        }
    }

    if (bestNote) {
        // 播放tap音效
        m_tapSfx->play();
        if (bestNote->data.type == NoteData::HOLD) {
            bestNote->holding = true;
            bestNote->wasHeld = true;
            m_activeHolds.push_back(bestNote);
            qint64 headDiff = (curTime > bestNote->data.timeMs) ? minDiff : 0;
            QColor col = judgementColor(headDiff);
            m_effects.append({bestNote->data.lane, col, currentMusicTime()});
            emit judgement(headDiff);
        } else {
            bestNote->judged = true;
            QColor col = judgementColor(minDiff);
            m_effects.append({bestNote->data.lane, col, currentMusicTime()});
            emit judgement(minDiff); // 发出判定信号
        }
    }
}

void PlayWindow::checkHoldRelease(int lane)
{
    qint64 curTime = currentMusicTime();
    for (auto it = m_activeHolds.begin(); it != m_activeHolds.end(); it++){
        GameNote *holdNote = *it;
        if (curTime >= holdNote->data.endTimeMs) {
            // 按住到结束
            holdNote->holding = false;
            holdNote->judged = true;
            m_activeHolds.erase(it);
            emit judgement(0);
            break;
        }
        if (holdNote->data.lane == lane && holdNote->holding) {
            qint64 diff = qAbs((qint64)(holdNote->data.endTimeMs - curTime));
            if (diff <= (holdNote->data.endTimeMs - holdNote->data.timeMs) / 10) {
                emit judgement(0);
            } else {
                holdNote->missed = true;
                emit judgement(999);
            }
            holdNote->holding = false;
            holdNote->judged = true;
            m_activeHolds.erase(it);
            break;
        }
    }
}

void PlayWindow::checkFlickHit(int deltaY)
{
    qint64 curTime = currentMusicTime();
    GameNote *bestNote = nullptr;
    qint64 minDiff = 1000000;

    for (auto &note : m_notes) {
        if (note.data.type != NoteData::FLICK || note.missed || note.judged) continue;
        qint64 diff = qAbs((qint64)(note.data.timeMs) - curTime);
        if (diff < 200 && diff < minDiff) {
            minDiff = diff;
            bestNote = &note;
        }
    }

    if (bestNote) {
        bestNote->judged = true;
        m_flickSfx->play();
        m_activeFlickLanes.remove(bestNote->data.lane);
        if (minDiff <= 200) {
            QColor col = judgementColor(0);
            m_effects.append({bestNote->data.lane, col, currentMusicTime()});
            emit judgement(0);
        }
    }
}

void PlayWindow::updateMissedNotes()
{
    qint64 curTime = currentMusicTime();

    for (auto it = m_activeHolds.begin(); it != m_activeHolds.end(); ) {
        GameNote* hold = *it;
        if (curTime >= hold->data.endTimeMs) {
            hold->holding = false;
            hold->judged = true;
            it = m_activeHolds.erase(it);
            emit judgement(0);
        } else {
            ++it;
        }
    }

    for (auto &note : m_notes) {
        if (note.judged || note.missed) continue;

        if (note.data.type == NoteData::HOLD) {
            if (curTime > note.data.endTimeMs + 200) {
                note.missed = true;
                if (note.holding) {
                    note.holding = false;
                    auto it = std::find(m_activeHolds.begin(), m_activeHolds.end(), &note);
                    if (it != m_activeHolds.end())
                        m_activeHolds.erase(it);
                }
                emit judgement(999);
                emit judgement(999);
            }
        } else {
            // TAP 和 FLICK 在头部错过 200ms 时判 Miss
            if (curTime > note.data.timeMs + 200) {
                note.missed = true;
                emit judgement(999);
            }
        }
    }

    // 检查游戏是否结束
    if (m_gameEnded) return;
    bool allJudged = true;
    for (const auto &note : m_notes) {
        if (!note.judged && !note.missed) {
            allJudged = false;
            break;
        }
    }
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this] {
        if (m_player->mediaStatus() == QMediaPlayer::EndOfMedia) {
            m_gameEnded = true;
            m_gameTimer->stop(); // 停止刷新
            m_player->stop(); // 停止音乐
            emit gameFinished();
        }
    }) ;
}

void PlayWindow::setFps(int fps) {
    if (fps == m_fps) return;
    m_fps = fps;
}

int PlayWindow::fps() const {
    return m_fps;
}

void PlayWindow::showResult(GameManager *gm)
{
    m_showResult = false;
    this->releaseMouse();

    // 隐藏 UI 控件
    ui->labelScore->hide();
    ui->labelCombo->hide();
    ui->labelAccuracy->hide();
    ui->labelJudgement->hide();
    ui->label->hide();
    ui->label_2->hide();
    ui->labelDiff->hide();
    ui->labelJudgetext->hide();
    ui->labelNote->hide();

    // 加载曲绘（如果失败就用空图）
    QPixmap cover;
    if (!m_coverPath.isEmpty()) {
        cover.load(m_coverPath);
    }

    QWidget *blackScreen = new QWidget(this);
    blackScreen->setGeometry(0, 0, width(), height());
    blackScreen->setStyleSheet("background-color: black;");
    blackScreen->show();
    blackScreen->raise();

    QGraphicsOpacityEffect *blackEffect = new QGraphicsOpacityEffect(blackScreen);
    blackScreen->setGraphicsEffect(blackEffect);
    blackEffect->setOpacity(0.0);

    QPropertyAnimation *fadeToBlack = new QPropertyAnimation(blackEffect, "opacity", blackScreen);
    fadeToBlack->setDuration(1500);
    fadeToBlack->setStartValue(0.0);
    fadeToBlack->setEndValue(1.0);

    // 动画完成后展示结算界面
    connect(fadeToBlack, &QPropertyAnimation::finished, this, [this, gm, cover, blackScreen]() {
        m_showResult = true;
        update();

    // songId = 谱面所在目录名
    QString songId = QFileInfo(m_chartPath).dir().dirName();
    QString jsonDir  = QCoreApplication::applicationDirPath() + "/data";
    QString jsonPath = jsonDir + "/play_history.json";

    // 读取已有记录
    QJsonArray historyArr;
    QFile readFile(jsonPath);
    if (readFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(readFile.readAll());
        if (doc.isArray()) {
            historyArr = doc.array();
        }
        readFile.close();
    }

    // 构建新记录
    QString relativeCover = QDir(QCoreApplication::applicationDirPath()).relativeFilePath(m_coverPath);
    QJsonObject newRec;
    newRec["songId"]    = songId;
    newRec["songName"]  = m_songTitle;
    newRec["score"]     = gm->score();
    newRec["coverPath"] = relativeCover;
    newRec["playTime"]  = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    historyArr.prepend(newRec);

    // 超过6条则移除最早的
    while (historyArr.size() > 6) {
        historyArr.removeLast();
    }

    // 写回 JSON 文件
    QDir().mkpath(jsonDir); // 确保 data 目录存在
    QFile writeFile(jsonPath);
    if (writeFile.open(QIODevice::WriteOnly)) {
        writeFile.write(QJsonDocument(historyArr).toJson());
        writeFile.close();
        qDebug() << "play history saved to" << jsonPath << ", count:" << historyArr.size();
    }

    // 更新统计数据
    QSettings profileSettings("RG", "Profile");

    // 判断是否首次游玩该曲目 → clear+1
    QStringList playedSongs = profileSettings.value("profile/played_songs").toStringList();
    if (!playedSongs.contains(songId)) {
        playedSongs.append(songId);
        profileSettings.setValue("profile/played_songs", playedSongs);

        int clearCount = profileSettings.value("profile/clear_count", 0).toInt() + 1;
        profileSettings.setValue("profile/clear_count", clearCount);
        qDebug() << "first clear of" << songId << "— clearCount:" << clearCount;
    }

    // 如果 miss=0 → Full Combo+1
    if (gm->missCount() == 0) {
        int fcCount = profileSettings.value("profile/fc_count", 0).toInt() + 1;
        profileSettings.setValue("profile/fc_count", fcCount);
        qDebug() << "Full Combo! fcCount:" << fcCount;
    }

    // 如果 great=good=miss=0 → All Perfect+1
    if (gm->greatCount() == 0 && gm->goodCount() == 0 && gm->missCount() == 0) {
        int apCount = profileSettings.value("profile/ap_count", 0).toInt() + 1;
        profileSettings.setValue("profile/ap_count", apCount);
        qDebug() << "All Perfect! apCount:" << apCount;
    }

        // 创建结算覆盖层
        ResultOverlay *overlay = new ResultOverlay(gm, m_songTitle, m_songArtist,
                                                   cover, m_difficulty, this);
        overlay->move(75, 0);
        overlay->show();
        overlay->raise();

        // 移除黑色遮罩
        blackScreen->deleteLater();

        connect(overlay, &ResultOverlay::backToMenu, this, [this]() {
            emit returnToMenu();
            QTimer::singleShot(100, this, &QWidget::close);
        });
    });

    fadeToBlack->start(QAbstractAnimation::DeleteWhenStopped);
}

void PlayWindow::togglePause()
{
    if (m_paused)
        resumeGame();
    else
        pauseGame();
}

void PlayWindow::pauseGame()
{
    if (m_paused) return;
    m_paused = true;
    this->releaseMouse();

    // 保存当前时间
    m_pausedTime = m_elapsed.elapsed();

    // 暂停音频和计时器
    m_player->pause();
    m_gameTimer->stop();

    // 显示暂停覆盖层
    if (!m_pauseOverlay) {
        m_pauseOverlay = new PauseOverlay(this);
        m_pauseOverlay->setGeometry(rect());
        // 连接三个信号
        connect(m_pauseOverlay, &PauseOverlay::resumeGame, this, &PlayWindow::resumeGame);
        connect(m_pauseOverlay, &PauseOverlay::restartGame, this, &PlayWindow::restartGame);
        connect(m_pauseOverlay, &PauseOverlay::backToMenu, this, [this]() {
            emit returnToMenu();
            // 延迟关闭，确保 PauseOverlay 的 click4 音效播放完毕
            QTimer::singleShot(200, this, &QWidget::close);
        });
    }

    int x = (1280 - 480) / 2;
    int y = (720 - 260) / 2;
    qDebug() << x << ',' << y;
    m_pauseOverlay->move(x, y);
    m_pauseOverlay->show();
    m_pauseOverlay->raise();
}

void PlayWindow::resumeGame()
{
    if (!m_paused) return;
    m_paused = false;

    this->grabMouse();

    // 隐藏覆盖层
    if (m_pauseOverlay)
        m_pauseOverlay->hide();

    // 补偿暂停期间流逝的时间
    qint64 now = m_elapsed.elapsed();
    m_musicStartOffset -= (now - m_pausedTime);

    // 恢复音频
    m_player->play();
    m_gameTimer->start(1000 / m_fps);
}

void PlayWindow::restartGame()
{
    loadGui();
    this->grabMouse();

    // 重置所有音符状态
    for (auto &note : m_notes) {
        note.judged = false;
        note.missed = false;
        note.holding = false;
    }
    m_activeHolds.clear();
    m_activeFlickLanes.clear();

    // 重置时间偏移
    m_musicStartOffset = 0;
    m_paused = false;

    // 隐藏暂停覆盖层
    if (m_pauseOverlay)
        m_pauseOverlay->hide();

    // 重新开始计时和音频
    m_elapsed.start();
    m_player->setPosition(0);
    m_player->play();
    m_gameTimer->start(1000 / m_fps);

    ui->labelJudgement->clear(); // 清除判定文字 否则会显示MISS

    emit restartRequested(); // 通知外部重置 GameManager
}

QColor PlayWindow::judgementColor(int diffMs) {
    if (diffMs == 999) return Qt::red; // Miss
    if (diffMs <= 70)  return QColor(255,215,0); // Perfect
    if (diffMs <= 110) return QColor("#FF1493"); // Great
    if (diffMs <= 200) return Qt::green; // Good
    return Qt::red; // 超出也当 Miss
}

void PlayWindow::updateEffects() {
    qint64 curTime = currentMusicTime();
    m_effects.erase(
        std::remove_if(m_effects.begin(), m_effects.end(),
                       [curTime](const HitEffect &e) {
                           return curTime - e.startTime > HitEffect::duration;
                       }),
        m_effects.end());
}

int PlayWindow::laneX(int lane) const {
    const int leftLaneW = 80;
    const int rightLaneW = 160;
    const int totalW = 4 * leftLaneW + rightLaneW;
    const int offsetX = (width() - totalW) / 2;
    if (lane < 4) return offsetX + lane * leftLaneW;
    else return offsetX + 4 * leftLaneW;
}

int PlayWindow::laneWidth(int lane) const {
    return (lane < 4) ? 80 - 8 : 160 - 8; // 减去边距，保持与音符大小一致
}

void PlayWindow::loadGui()
{
    ui->labelAccuracy->setText(QString("100.0000%"));
    ui->labelCombo->setText(QString("<span style='font-size: 22px; color: white;'>Combo</span><br>"));
    ui->labelJudgement->clear();
    ui->labelScore->setText(QString("0"));
    ui->labelJudgetext->setText(QString("<span style='color:white;'>MaxCombo</span><br>"
                                        "<span style='color:rgb(255,215,0);'>Perfect</span><br>"
                                        "<span style='color:rgb(255,0,127);'>Great</span><br>"
                                        "<span style='color:rgb(169,219,140);'>Good</span><br>"
                                        "<span style='color:gray;'>Miss</span>"));
    ui->labelNote->setText(QString("<span style='color:white;'>0</span><br>"
                                   "<span style='color:white;'>0</span><br>"
                                   "<span style='color:white;'>0</span><br>"
                                   "<span style='color:white;'>0</span><br>"
                                   "<span style='color:white;'>0</span>"));
    ui->labelDiff->setText(QString("Lv." + m_difficulty));
}

void PlayWindow::shakeLabelJudgement()
{
    // 停止并销毁之前的动画组
    if (m_judgeAnimGroup) {
        m_judgeAnimGroup->stop();
        delete m_judgeAnimGroup;
        m_judgeAnimGroup = nullptr;
    }

    if (m_judgementOrigin == QPoint(0, 0)) {
        m_judgementOrigin = ui->labelJudgement->pos();
    }
    // 强制复位到原点
    ui->labelJudgement->move(m_judgementOrigin);

    int jumpHeight = 15; // 弹跳高度
    // 向下移动
    auto *down = new QPropertyAnimation(ui->labelJudgement, "pos");
    down->setDuration(80);
    down->setStartValue(m_judgementOrigin);
    down->setEndValue(m_judgementOrigin + QPoint(0, jumpHeight));
    down->setEasingCurve(QEasingCurve::OutQuad);

    // 弹回原位
    auto *up = new QPropertyAnimation(ui->labelJudgement, "pos");
    up->setDuration(120);
    up->setStartValue(m_judgementOrigin + QPoint(0, jumpHeight));
    up->setEndValue(m_judgementOrigin);
    up->setEasingCurve(QEasingCurve::OutBounce);

    m_judgeAnimGroup = new QSequentialAnimationGroup(this);
    m_judgeAnimGroup->addAnimation(down);
    m_judgeAnimGroup->addAnimation(up);
    m_judgeAnimGroup->start();
}

QPixmap PlayWindow::tapPixmap(int lane) const {
    return (lane <= 1) ? m_tapBlue : m_tapRed;
}

QPixmap PlayWindow::holdBodyPixmap(int lane) const {
    return (lane <= 1) ? m_holdBodyBlue : m_holdBodyRed;
}

void PlayWindow::drawHold(QPainter &p, int lane, int x, int noteW, int topY, int bottomY, bool headFixed) {
    const int headH = 20; // 头/尾高度

    QPixmap tap = tapPixmap(lane);
    QPixmap body = holdBodyPixmap(lane);

    int actualTop = qMin(topY, bottomY);
    int actualBottom = qMax(topY, bottomY);
    int bodyTop = actualTop + headH;
    int bodyBottom = actualBottom - headH;

    // 绘制身体
    if (bodyBottom > bodyTop) {
        int bodyH = bodyBottom - bodyTop;
        QPixmap scaledBody = body.scaled(noteW, bodyH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        p.drawPixmap(x + 4, bodyTop, scaledBody);
    }

    // 绘制头部
    QPixmap scaledHead = tap.scaled(noteW, headH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    p.drawPixmap(x + 4, actualTop, scaledHead);

    // 绘制尾部
    if (actualBottom - actualTop > headH) {  // 避免完全重叠时画两次
        p.drawPixmap(x + 4, actualBottom - headH, scaledHead);
    }
}