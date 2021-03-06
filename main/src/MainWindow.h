#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>

#include "Core.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QSystemTrayIcon;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Core* core, QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_btnOK_clicked();

    void onRefreshTimer();

    void on_btnListen_toggled(bool checked);

    void on_btnTalk_toggled(bool checked);

    void on_btnSettings_clicked();

    void on_btnResetPlaybackDevice_clicked();

    void on_btnResetRecordDevice_clicked();

private:
    Ui::MainWindow *ui;
    Core* core;
    QSettings settings;
    void reloadDevices();
    void reloadSettings();
    QTimer refreshTimer;
    void showPage_settings();
    void showPage_main();

private:
    void createTray();
    void updateTray();
    QSystemTrayIcon* trayIcon;

private:
    struct
    {
        QVector<Core::AudioDevice> playback;
        QVector<Core::AudioDevice> record;
    }   currentDevices;
};
#endif // MAINWINDOW_H
