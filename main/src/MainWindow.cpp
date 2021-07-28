#include "MainWindow.h"
#include "ui_MainWindow.h"


// Needed only for Android
#include <QStandardPaths>

// System Tray (not available for Android, but does compile... -> no define switch)
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

#include "Core.h"

#include <QDebug>
QDebug operator<<(QDebug debug, const Core::AudioDevice &c)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << c.name << " -- " << c.id << ')';

    return debug;
}

MainWindow::MainWindow(Core* core, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , core(core)
    , settings(CONFIG_DIR, QSettings::IniFormat)
{
    ui->setupUi(this);
    ui->tableTalkers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setWindowTitle("Flurfunk");
    createTray();
    updateTray();
    trayIcon->show();

    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(onRefreshTimer()));
    connect(core, SIGNAL(newTalker()), this, SLOT(onRefreshTimer()));
    refreshTimer.setInterval(200);
    refreshTimer.start();

    if (settings.value("Start/SkipSettings", false).toBool())
        showPage_main();
    else
        showPage_settings();

    if (settings.value("Start/StartHidden", false).toBool())
        hide();
    else
        show();

    if (settings.value("Start/ImmediatePlayback", false).toBool())
        ui->btnListen->setChecked(true);

    if (settings.value("Start/ImmediateRecord", false).toBool())
        ui->btnTalk->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::reloadDevices()
{
    currentDevices.playback = core->getPlaybackDevices();
    currentDevices.record = core->getRecordingDevices();
    qDebug() << currentDevices.record;

    ui->listPlayback->clear();
    for (int i = 0; i < currentDevices.playback.count(); i += 1)
    {
        ui->listPlayback->addItem(currentDevices.playback.at(i).name);
        if (!settings.value("Playback/Device/auto",true).toBool() && (settings.value("Playback/Device/id","").toString() == currentDevices.playback.at(i).id) && (settings.value("Playback/Device/name","").toString() == currentDevices.playback.at(i).name))
            ui->listPlayback->setCurrentRow(i);
    }

    ui->listRecord->clear();
    for (int i = 0; i < currentDevices.record.count(); i += 1)
    {
        ui->listRecord->addItem(currentDevices.record.at(i).name);
        if (!settings.value("Record/Device/auto",true).toBool() && (settings.value("Record/Device/id","").toString() == currentDevices.record.at(i).id) && (settings.value("Record/Device/name","").toString() == currentDevices.record.at(i).name))
            ui->listRecord->setCurrentRow(i);
    }
}

void MainWindow::reloadSettings()
{
    ui->checkSkipSettings->setChecked(settings.value("Start/SkipSettings", false).toBool());
    ui->checkStartHidden->setChecked(settings.value("Start/StartHidden", false).toBool());
    ui->checkImmediatePlayback->setChecked(settings.value("Start/ImmediatePlayback", false).toBool());
    ui->checkImmediateRecord->setChecked(settings.value("Start/ImmediateRecord", false).toBool());
}

void MainWindow::showPage_settings()
{
    //trayIcon->hide();
    //core->stopPlayback();
    //core->stopRecording();
    //ui->btnListen->setChecked(false);
    //ui->btnTalk->setChecked(false);
    reloadDevices();
    reloadSettings();
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::showPage_main()
{
    ui->stackedWidget->setCurrentIndex(0);
    //trayIcon->show();
}

void MainWindow::createTray()
{
    auto hideAction = new QAction(tr("&Hide"), this);
    connect(hideAction, &QAction::triggered, this, [this](){
        if (!isVisible())
            showNormal();
        else
            hide();
        updateTray();
    });
    hideAction->setCheckable(true);

    auto playbackAction = new QAction(tr("&Playback"), this);
    connect(playbackAction, &QAction::triggered, ui->btnListen, &QPushButton::click);
    playbackAction->setCheckable(true);

    auto recordAction = new QAction(tr("&Record"), this);
    connect(recordAction, &QAction::triggered, ui->btnTalk, &QPushButton::click);
    recordAction->setCheckable(true);

    auto quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    auto trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(hideAction);
    //trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(playbackAction);
    trayIconMenu->addAction(recordAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    trayIcon->setIcon(QIcon(":/icon/idle"));
    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason)
    {
        switch (reason) {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            ui->btnTalk->click();
            break;
        default:
            break;
        }
    });
}

void MainWindow::updateTray()
{
    if (core->isRecording())
        if (core->isPlayback())
            trayIcon->setIcon(QIcon(":/icon/bi"));
        else
            trayIcon->setIcon(QIcon(":/icon/record"));
    else
        if (core->isPlayback())
            trayIcon->setIcon(QIcon(":/icon/playback"));
        else
            trayIcon->setIcon(QIcon(":/icon/idle"));

    trayIcon->contextMenu()->actions().at(0)->setChecked(!isVisible());
    trayIcon->contextMenu()->actions().at(2)->setChecked(core->isPlayback());
    trayIcon->contextMenu()->actions().at(3)->setChecked(core->isRecording());
}


void MainWindow::on_btnPlayback_clicked()
{

}

void MainWindow::on_btnRecord_clicked()
{

}

void MainWindow::on_btnOK_clicked()
{
    bool wasPlaying = core->isPlayback();
    bool wasRecording = core->isRecording();
    on_btnListen_toggled(false);
    on_btnTalk_toggled(false);

    if (ui->listPlayback->currentRow() < 0)
    {
        settings.setValue("Playback/Device/auto", true);
    }
    else
    {
        settings.setValue("Playback/Device/auto", false);
        settings.setValue("Playback/Device/id", currentDevices.playback.at(ui->listPlayback->currentRow()).id);
        settings.setValue("Playback/Device/name", currentDevices.playback.at(ui->listPlayback->currentRow()).name);
    }

    if (ui->listRecord->currentRow() < 0)
    {
        settings.setValue("Record/Device/auto", true);
    }
    else
    {
        settings.setValue("Record/Device/auto", false);
        settings.setValue("Record/Device/id", currentDevices.record.at(ui->listRecord->currentRow()).id);
        settings.setValue("Record/Device/name", currentDevices.record.at(ui->listRecord->currentRow()).name);
    }

    settings.setValue("Start/SkipSettings", ui->checkSkipSettings->isChecked());
    settings.setValue("Start/StartHidden", ui->checkStartHidden->isChecked());
    settings.setValue("Start/ImmediatePlayback", ui->checkImmediatePlayback->isChecked());
    settings.setValue("Start/ImmediateRecord", ui->checkImmediateRecord->isChecked());

    settings.sync();
    if (wasPlaying)
        on_btnListen_toggled(true); // equal to "start Playback"
    if (wasRecording)
        on_btnTalk_toggled(true); // equal to "start Recording"
    showPage_main();
}

void MainWindow::onRefreshTimer()
{
    QMap<quint32, Core::Talker> const& talkers = core->getTalkers();
    QMapIterator<quint32, Core::Talker> i(talkers);

    int count = 0;
    while (i.hasNext())
    {
        i.next();
        count++;

        //cout << i.key() << ": " << i.value() << Qt::endl;
        if (ui->tableTalkers->rowCount() < count)
        {
            ui->tableTalkers->insertRow(ui->tableTalkers->rowCount());
            ui->tableTalkers->setItem(ui->tableTalkers->rowCount()-1, 0, new QTableWidgetItem());
            ui->tableTalkers->setItem(ui->tableTalkers->rowCount()-1, 1, new QTableWidgetItem());
        }
        ui->tableTalkers->item(count - 1, 0)->setText(i.value().address.toString());
        ui->tableTalkers->item(count - 1, 1)->setText(QString("%1 ms").arg(i.value().lastActivity.elapsed()));
    }

    updateTray();
}

void MainWindow::on_btnTalk_clicked()
{

}

void MainWindow::on_btnListen_toggled(bool checked)
{
    if (checked)
    {
        if (!core->startPlayback(settings.value("Playback/Device/auto",true).toBool(), Core::AudioDevice{settings.value("Playback/Device/name","").toString(), settings.value("Playback/Device/id","").toString()}))
            ui->btnListen->setChecked(false);
    }
    else
        core->stopPlayback();

    updateTray();
}

void MainWindow::on_btnTalk_toggled(bool checked)
{
    if (checked)
    {
        if (!core->startRecording(settings.value("Record/Device/auto",true).toBool(), Core::AudioDevice{settings.value("Record/Device/name","").toString(), settings.value("Record/Device/id","").toString()}))
            ui->btnTalk->setChecked(false);
    }
    else
        core->stopRecording();

    updateTray();
}

void MainWindow::on_btnSettings_clicked()
{
    showPage_settings();
}

void MainWindow::on_btnResetPlaybackDevice_clicked()
{
    ui->listPlayback->setCurrentRow(-1);
}

void MainWindow::on_btnResetRecordDevice_clicked()
{
    ui->listRecord->setCurrentRow(-1);
}
