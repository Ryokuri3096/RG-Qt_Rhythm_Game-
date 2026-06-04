#ifndef RESULTOVERLAY_H
#define RESULTOVERLAY_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>

#include "gamemanager.h"

namespace Ui {
class ResultOverlay;
}

class ResultOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit ResultOverlay(GameManager *gm, const QString &songTitle,
                           const QString &artist, const QPixmap &cover,
                           const QString &difficulty, QWidget *parent = nullptr);
    ~ResultOverlay();

signals:
    void backToMenu();

private slots:
    void on_backButton_clicked();

private:
    Ui::ResultOverlay *ui;

    void setupUI(GameManager *gm, const QString &songTitle,
                 const QString &artist, const QPixmap &cover,
                 const QString &difficulty);
};

#endif // RESULTOVERLAY_H
