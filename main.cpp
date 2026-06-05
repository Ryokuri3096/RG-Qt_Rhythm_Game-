#include "mainwindow.h"

#include <QApplication>
#include <QFontDatabase>
#include <QDebug>

void fontInit();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    fontInit();
    MainWindow w;
    w.resize(1280, 720);
    w.show();
    //w.showMaximized();
    return QCoreApplication::exec();
}

void fontInit()
{
    int exoRegularId = QFontDatabase::addApplicationFont(":/fonts/Exo-Regular.otf");

    if (exoRegularId == -1) {
        qWarning() << "Exo-Regular字体加载失败！";
    } else {
        // 获取字体家族名称
        QStringList exoFamilies = QFontDatabase::applicationFontFamilies(exoRegularId);
        if (!exoFamilies.isEmpty()) {
            QString exoFamily = exoFamilies.first();
            // qDebug() << "Exo-Regular字体家族名称：" << exoFamily; // "Exo"
        }
    }

    int geoSansId = QFontDatabase::addApplicationFont(":/fonts/GeosansLight-1.ttf");

    if (geoSansId == -1) {
        qWarning() << "GeosansLight-1字体加载失败！";
    } else {
        // 获取字体家族名称
        QStringList geoFamilies = QFontDatabase::applicationFontFamilies(geoSansId);
        if (!geoFamilies.isEmpty()) {
            QString geoFamily = geoFamilies.first();
            // qDebug() << "GeosansLight-1字体家族名称：" << geoFamily; // "GeosansLight"
        }
    }

    int kazeId = QFontDatabase::addApplicationFont(":/fonts/Kazesawa-Regular.ttf");

    if (kazeId == -1) {
        qWarning() << "Kazesawa-Regular字体加载失败！";
    } else {
        // 获取字体家族名称
        QStringList kazeFamilies = QFontDatabase::applicationFontFamilies(kazeId);
        if (!kazeFamilies.isEmpty()) {
            QString kazeFamily = kazeFamilies.first();
            // qDebug() << "Kazesawa-Regular字体家族名称：" << kazeFamily; // "Kazesawa"
        }
    }
}