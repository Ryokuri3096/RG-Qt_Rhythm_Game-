#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QVariant>

class MainWindow;

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

    void setMainWindow(MainWindow *win);

signals:
    // 当任何设置项数值改变时发出，参数为设置项名称和新值
    void settingChanged(const QString &key, const QVariant &value);

protected:
    void paintEvent(QPaintEvent *event) override;   // 绘制背景图
    void resizeEvent(QResizeEvent *event) override;  // 调整遮罩层位置
    void showEvent(QShowEvent *event) override;      // 首次显示时定位遮罩层

private slots:
    void on_backButton_clicked();

    // 滑动条值改变
    void onVolumeSliderChanged(int val);
    void onOffsetSliderChanged(int val);

    // 加减按钮
    void onMusicMinus();
    void onMusicPlus();
    void onSfxMinus();
    void onSfxPlus();
    void onBgmMinus();
    void onBgmPlus();
    void onOffsetMinus();
    void onOffsetPlus();

private:
    void applyStyles();        // 给 .ui 中控件应用代码样式
    void connectSignals();     // 连接信号槽
    void loadSettings();       // 加载保存的设置
    void saveSettings();       // 保存设置
    void updateVolumeLabels(); // 更新三个音量数值显示
    void updateOffsetLabel();  // 更新延时数值显示
    void updateButtonStates(); // 更新加减按钮可用状态
    void updateOverlayPos();   // 更新遮罩层位置

    Ui::SettingsWindow *ui;
    MainWindow *m_mainWin;
    QSettings *m_settings;

    // 当前设置值
    double m_musicVolume; // 0.0 ~ 1.0
    double m_sfxVolume;   // 0.0 ~ 1.0
    double m_bgmVolume;   // 0.0 ~ 1.0
    int m_offset;         // -200 ~ 200 ms

    // 控件指针数组 [music, sfx, bgm, offset]
    QSlider *m_sliders[4];     
    QLabel *m_valueLabels[4];       
    QPushButton *m_minusBtns[4];    
    QPushButton *m_plusBtns[4];    

    // 背景图
    QPixmap m_background;
};

#endif // SETTINGSWINDOW_H
