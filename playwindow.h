#ifndef PLAYWINDOW_H
#define PLAYWINDOW_H

#include <QKeyEvent>
#include <QMainWindow>

namespace Ui {
class PlayWindow;
}

class PlayWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlayWindow(QWidget *parent = nullptr);
    ~PlayWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::PlayWindow *ui;
};

#endif // PLAYWINDOW_H
