#include "resultoverlay.h"
#include "ui_resultoverlay.h"
#include <QTimer>

ResultOverlay::ResultOverlay(GameManager *gm, const QString &songTitle,
                             const QString &artist, const QPixmap &cover,
                             const QString &difficulty, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ResultOverlay)
{
    ui->setupUi(this);

    // 初始化返回按钮音效
    m_click4Sfx = new QSoundEffect(this);
    m_click4Sfx->setSource(QUrl("qrc:/sfx/click4.wav"));

    // 初始化结算循环音效
    m_overSfx = new QSoundEffect(this);
    m_overSfx->setSource(QUrl("qrc:/sfx/over.wav"));
    m_overSfx->setLoopCount(QSoundEffect::Infinite);

    // setStyleSheet("background-color: rgba(0, 0, 0, 200);");
    setupUI(gm, songTitle, artist, cover, difficulty);

    // 开始循环播放结算音效
    m_overSfx->play();
}

ResultOverlay::~ResultOverlay()
{
    delete ui;
}

void ResultOverlay::setupUI(GameManager *gm, const QString &songTitle,
                            const QString &artist, const QPixmap &cover,
                            const QString &difficulty)
{
    ui->titleLabel->setText(songTitle);
    ui->artistLabel->setText(artist);
    ui->coverLabel->setPixmap(cover.scaled(ui->coverLabel->width(),
                                       ui->coverLabel->height(),
                                       Qt::IgnoreAspectRatio, // 强制拉伸 把曲绘变成正方形
                                       Qt::SmoothTransformation));
    ui->coverLabel->setStyleSheet("border: 4px solid #360054;");
    ui->diffLabel->setText("Lv." + difficulty);

    ui->dataLabel->setText(QString("<span style='color:white;'>Score: %1</span><br>"
                                   "<span style='color:white;'>MaxCombo: %2</span><br>"
                                   "<span style='color:rgb(255,215,0);'>Perfect: %3</span><br>"
                                   "<span style='color:rgb(255,0,127);'>Great: %4</span><br>"
                                   "<span style='color:rgb(169,219,140);'>Good: %5</span><br>"
                                   "<span style='color:gray;'>Miss: %6</span><br>"
                                   "<span style='color:white;'>Accuracy: %7%</span><br>"
                                   ).arg(gm->score()).arg(gm->maxCombo()).arg(gm->perfectCount()).arg(gm->greatCount())
        .arg(gm->goodCount()).arg(gm->missCount()).arg(QString::number(gm->accuracy(), 'f', 4)));
}
void ResultOverlay::on_backButton_clicked()
{
    m_overSfx->stop(); // 停止结算循环音效
    m_click4Sfx->play();
    // 延迟发射信号，确保音效播放完毕再关闭窗口
    QTimer::singleShot(200, this, [this]() {
        emit backToMenu();
    });
}