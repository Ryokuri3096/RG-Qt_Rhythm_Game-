#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QKeyEvent>
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

    // 获取当前键位映射，供 PlayWindow 读取
    const int* getKeyMapping() const { return m_keyMapping; }

signals:
    // 当任何设置项数值改变时发出，参数为设置项名称和新值
    void settingChanged(const QString &key, const QVariant &value);

protected:
    // 绘制背景图
    void paintEvent(QPaintEvent *event) override;   
    void resizeEvent(QResizeEvent *event) override; 
    void showEvent(QShowEvent *event) override;
    
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_backButton_clicked();

    // 音量滑动条值改变
    void onVolumeSliderChanged(int val);
    // 流速滑动条值改变
    void onSpeedSliderChanged(int val);

    // 音量加减按钮
    void onMusicMinus();
    void onMusicPlus();
    void onSfxMinus();
    void onSfxPlus();
    // 流速加减按钮
    void onSpeedMinus();
    void onSpeedPlus();
    // 键位映射按钮点击
    void onKeyMappingBtnClicked();

private:
    void applyStyles();        // 给 .ui 中控件应用代码样式
    void connectSignals();     // 连接信号槽
    void loadSettings();       // 加载保存的设置
    void saveSettings();       // 保存设置
    void updateValueLabels();  // 更新所有数值显示（音量+流速）
    void updateButtonStates(); // 更新加减按钮可用状态
    void updateOverlayPos();   // 更新遮罩层位置

    Ui::SettingsWindow *ui;
    MainWindow *m_mainWin;
    QSettings *m_settings;

    // 当前设置值
    double m_musicVolume; // 0.0 ~ 1.0
    double m_sfxVolume;   // 0.0 ~ 1.0
    double m_speed;       // 0.01 ~ 2.00（流速）

    // 控件指针数组 [music, sfx, speed]（共3项，删除了offset）
    QSlider *m_sliders[3];
    QLabel *m_valueLabels[3];
    QPushButton *m_minusBtns[3];
    QPushButton *m_plusBtns[3];

    // 键位映射
    int m_keyMapping[4];          // 存储 Qt::Key 值
    QPushButton *m_keyBtns[4];    // 四个轨道按钮
    int m_waitingForLane = -1;    // -1=不在等待，0~3=等待用户按下的轨道

    // 背景图
    QPixmap m_background;
};

#endif // SETTINGSWINDOW_H
