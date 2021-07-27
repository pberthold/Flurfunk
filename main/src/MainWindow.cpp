#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QStandardPaths>

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
    //, settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), QSettings::IniFormat)
    , settings(CONFIG_DIR, QSettings::IniFormat)
{
    ui->setupUi(this);
    ui->tableTalkers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setWindowTitle("Flurfunk");

    reloadDevices();
    ui->stackedWidget->setCurrentIndex(1);
    //ui->tableTalkers->setColumnWidth(0, this->geometry().width() / 3);

    connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(onRefreshTimer()));
    connect(core, SIGNAL(newTalker()), this, SLOT(onRefreshTimer()));
    refreshTimer.setInterval(200);
    refreshTimer.start();
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
        if (!settings.value("Playback/Device/auto",true).toBool() && (settings.value("Playback/Device/id","").toString() == currentDevices.playback.at(i).id))
            ui->listPlayback->setCurrentRow(i);
    }

    ui->listRecord->clear();
    for (int i = 0; i < currentDevices.record.count(); i += 1)
    {
        ui->listRecord->addItem(currentDevices.record.at(i).name);
        if (!settings.value("Record/Device/auto",true).toBool() && (settings.value("Record/Device/id","").toString() == currentDevices.record.at(i).id))
            ui->listRecord->setCurrentRow(i);
    }
}


void MainWindow::on_btnPlayback_clicked()
{
    if (ui->listPlayback->currentRow() < 0)
    {
        settings.setValue("Playback/Device/auto", true);
    }
    else
    {
        settings.setValue("Playback/Device/auto", false);
        settings.setValue("Playback/Device/id", currentDevices.playback.at(ui->listPlayback->currentRow()).id);
    }

    settings.sync();
}

void MainWindow::on_btnRecord_clicked()
{
    if (ui->listRecord->currentRow() < 0)
    {
        settings.setValue("Record/Device/auto", true);
    }
    else
    {
        settings.setValue("Record/Device/auto", false);
        settings.setValue("Record/Device/id", currentDevices.record.at(ui->listRecord->currentRow()).id);
    }

    settings.sync();
}

void MainWindow::on_btnOK_clicked()
{
    settings.sync();
    ui->stackedWidget->setCurrentIndex(0);
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
}

void MainWindow::on_btnTalk_clicked()
{

}

void MainWindow::on_btnListen_toggled(bool checked)
{
    if (checked)
    {
        if (!core->startPlayback(settings.value("Playback/Device/auto",true).toBool(), settings.value("Playback/Device/id","").toString()))
            ui->btnListen->setChecked(false);
    }
    else
        core->stopPlayback();
}

void MainWindow::on_btnTalk_toggled(bool checked)
{
    if (checked)
    {
        if (!core->startRecording(settings.value("Record/Device/auto",true).toBool(), settings.value("Record/Device/id","").toString()))
            ui->btnTalk->setChecked(false);
    }
    else
        core->stopRecording();
}

void MainWindow::on_btnSettings_clicked()
{
    core->stopPlayback();
    core->stopRecording();
    ui->btnListen->setChecked(false);
    ui->btnTalk->setChecked(false);
    reloadDevices();
    ui->stackedWidget->setCurrentIndex(1);
}
