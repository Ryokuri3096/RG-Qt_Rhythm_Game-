#include "resultoverlay.h"
#include "ui_resultoverlay.h"

ResultOverlay::ResultOverlay(GameManager *gm, const QString &songTitle,
                             const QString &artist, const QPixmap &cover,
                             const QString &difficulty, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ResultOverlay)
{
    ui->setupUi(this);

    setStyleSheet("background-color: rgba(0, 0, 0, 200);");
    setupUI(gm, songTitle, artist, cover, difficulty);
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
    emit backToMenu();
}