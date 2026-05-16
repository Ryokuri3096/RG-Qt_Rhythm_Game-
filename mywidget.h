#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QScrollArea>
#include <QWidget>

class MyWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit MyWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // MYWIDGET_H