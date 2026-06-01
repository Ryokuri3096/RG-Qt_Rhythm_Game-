#include "playwindow.h"
#include "ui_playwindow.h"

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
    QFile file(chartPath); // 打开谱面文件
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Cannot open chart file!");
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    // 获取曲绘 乐曲名 作者 难度并显示
    QJsonObject meta = root["meta"].toObject();
    m_songTitle = meta["song"].toObject()["title"].toString();
    m_songArtist = meta["song"].toObject()["artist"].toString();
    ui->labelTitle->setText(m_songTitle + " / " +m_songArtist);
    ui->labelTitle->hide();
    m_difficulty = meta["version"].toString();
    QString bgFile = meta["background"].toString();
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
}

void PlayWindow::startGame()
{
    if (m_notes.empty()) return;

    // 游戏初始化
    m_elapsed.start();
    m_musicStartOffset = 0;

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

    if (m_showResult) {
        // 只填背景，不画任何游戏内容
        p.fillRect(rect(), QColor(20, 20, 30)); // 深色背景，可自定义
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

    // 绘制轨道线（5条竖线，分隔6条边）
    p.setPen(QPen(QColor(80, 80, 80), 1));
    for (int i = 0; i <= 4; ++i) {
        int x = offsetX + i * leftLaneW;
        p.drawLine(x, 0, x, h);
    }
    // 最后一轨的右边界
    int xRight = offsetX + 4 * leftLaneW + rightLaneW;
    p.drawLine(xRight, 0, xRight, h);

    // 判定线
    p.setPen(QPen(Qt::yellow, 2));
    p.drawLine(offsetX, m_hitLineY, offsetX + totalW, m_hitLineY);

    // 绘制音符
    for (const auto &note : m_notes) {
        if (note.judged || note.missed) continue;

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

        // 计算音符的 X 坐标和宽度
        int x, noteW;
        if (note.data.lane < 4) {
            x = offsetX + note.data.lane * leftLaneW;
            noteW = leftLaneW - 8;  // 左右各留4像素
        } else {
            x = offsetX + 4 * leftLaneW;
            noteW = rightLaneW - 8;
        }

        switch (note.data.type) {
        case NoteData::TAP: {
            QRectF rect(x + 4, noteY, noteW, 18);
            p.fillRect(rect, QColor(0, 200, 255));
            break;
        }
        case NoteData::HOLD: {
            if (note.holding) {
                // 按住状态
                int headY = m_hitLineY; // 头部固定在判定线
                double endYOffset = (note.data.endTimeMs - curTime) * speed;
                int endY = m_hitLineY - (int)endYOffset; // 结束时间对应的 Y

                // 绘制身体
                int bodyTop = qMax(endY, -50);
                int bodyBottom = m_hitLineY;
                if (bodyTop < bodyBottom) {
                    QRectF bodyRect(x + 4, bodyTop, noteW, bodyBottom - bodyTop);
                    p.fillRect(bodyRect, QColor(255, 200, 0, 200));
                }
                // 绘制头部
                p.fillRect(QRectF(x + 4, headY - 9, noteW, 18), QColor(255, 255, 0));
            }
            else {
                // 未按住状态
                int endY = m_hitLineY - (int)((note.data.endTimeMs - curTime) * speed);
                int bodyTop = qMin(noteY, endY);
                int bodyBottom = qMax(noteY, endY);
                int visibleTop = qMax(bodyTop, -50);
                int visibleBottom = qMin(bodyBottom, height() + 50);
                if (visibleBottom > visibleTop) {
                    QRectF bodyRect(x + 4, visibleTop, noteW, visibleBottom - visibleTop);
                    p.fillRect(bodyRect, QColor(255, 200, 0, 200));
                }
                // 头部
                if (noteY >= -50 && noteY <= height() + 50) {
                    p.fillRect(QRectF(x + 4, noteY, noteW, 18), QColor(255, 255, 0));
                }
            }
            break;
        }
        case NoteData::FLICK: {
            QRectF rect(x + 4, noteY, noteW, 18);
            p.fillRect(rect, QColor(200, 0, 200));
            p.setPen(Qt::white);
            p.drawText(rect, Qt::AlignCenter, "↕");
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
        togglePause();
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
    int deltaY = event->pos().y() - m_lastMousePos.y();
    m_lastMousePos = event->pos();

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
        if (bestNote->data.type == NoteData::HOLD) {
            bestNote->holding = true;
            m_activeHolds.push_back(bestNote);
        } else {
            bestNote->judged = true;
        }
        QColor col = judgementColor(minDiff);
        m_effects.append({bestNote->data.lane, col, currentMusicTime()});
        emit judgement(minDiff); // 发出判定信号
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
            // 提前释放 则计算时间与尾部的差值
            qint64 diff = qAbs((qint64)(holdNote->data.endTimeMs - curTime));
            if (diff < (holdNote->data.endTimeMs - holdNote->data.timeMs) * 4 / 5) { // 按住时间占比80%即为Perfect
                emit judgement(0);
            } else {
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
        m_activeFlickLanes.remove(bestNote->data.lane);
        if (minDiff <= 200) { // 如果在200ms内滑动则判定为Perfect 否则不判定
            QColor col = judgementColor(0);
            m_effects.append({bestNote->data.lane, col, currentMusicTime()});
            emit judgement(0);
        }
    }
}

void PlayWindow::updateMissedNotes()
{
    qint64 curTime = currentMusicTime();

    // 自动完成按住到结束的 Hold
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
            // 对于 Hold，整个长条都错过了才判 Miss
            if (curTime > note.data.endTimeMs + 200) {
                note.missed = true;
                if (note.holding) {
                    note.holding = false;
                    auto it = std::find(m_activeHolds.begin(), m_activeHolds.end(), &note);
                    if (it != m_activeHolds.end())
                        m_activeHolds.erase(it);
                }
                emit judgement(999);
                emit judgement(999); // 头尾都算作miss 故发射两次miss信号
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
    if (allJudged) {
        m_gameEnded = true;
        m_gameTimer->stop(); // 停止刷新
        m_player->stop(); // 停止音乐
        emit gameFinished();
    }
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
    m_showResult = true;
    update();

    // 隐藏 UI 控件...
    ui->labelScore->hide();
    ui->labelCombo->hide();
    ui->labelAccuracy->hide();
    ui->labelJudgement->hide();

    // 加载曲绘（如果失败就用空图）
    QPixmap cover;
    if (!m_coverPath.isEmpty()) {
        cover.load(m_coverPath);
    }

    // 创建覆盖层
    ResultOverlay *overlay = new ResultOverlay(gm, m_songTitle, m_songArtist,
                                               cover, m_difficulty, this);
    overlay->move(75, 0);
    overlay->show();
    overlay->raise();

    connect(overlay, &ResultOverlay::backToMenu, this, [this]() {
        emit returnToMenu();  // 通知外部（SongWindow）
        this->close();        // 关闭游戏窗口
    });
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
            this->close();
        });
    }

    int x = (1280 - 400) / 2;
    int y = (720 - 200) / 2;
    qDebug() << x << ',' << y;
    m_pauseOverlay->move(x, y);
    m_pauseOverlay->show();
    m_pauseOverlay->raise();
}

void PlayWindow::resumeGame()
{
    if (!m_paused) return;
    m_paused = false;

    // 隐藏覆盖层
    if (m_pauseOverlay)
        m_pauseOverlay->hide();

    // 补偿暂停期间流逝的时间
    qint64 now = m_elapsed.elapsed();
    m_musicStartOffset -= (now - m_pausedTime);   // 将暂停时长从偏移中扣除

    // 恢复音频
    m_player->play();
    m_gameTimer->start(1000 / m_fps);
}

void PlayWindow::restartGame()
{
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
    if (diffMs <= 50)  return QColor(255,215,0); // Perfect
    if (diffMs <= 100) return QColor("#FF1493"); // Great
    if (diffMs <= 150) return Qt::green; // Good
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