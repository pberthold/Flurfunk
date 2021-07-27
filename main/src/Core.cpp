#include "Core.h"
#include <QNetworkDatagram>
#include <QNetworkInterface>

Core::Core(QObject *parent) : QObject(parent)
{
    connect(&socket, SIGNAL(readyRead()), this, SLOT(onSocketRead()));
    socket.bind(6000);
}

Core::~Core()
{
    stopPlayback();
    stopRecording();
}

QVector<Core::AudioDevice> Core::getPlaybackDevices()
{
    QVector<Core::AudioDevice> result;

    int a;
    BASS_DEVICEINFO info;
    for (a = 0; BASS_GetDeviceInfo(a, &info); a++)
    {
        //if (info.flags & BASS_DEVICE_ENABLED) // device is enabled
        result.append(Core::AudioDevice{QString::fromLocal8Bit(info.name), QString(info.driver)});
    }

    return result;
}

QVector<Core::AudioDevice> Core::getRecordingDevices()
{
    QVector<Core::AudioDevice> result;

    int a;
    BASS_DEVICEINFO info;
    for (a = 0; BASS_RecordGetDeviceInfo(a, &info); a++)
    {
        //if (info.flags & BASS_DEVICE_ENABLED) // device is enabled
        result.append(Core::AudioDevice{QString::fromLocal8Bit(info.name), QString(info.driver)});
    }

    return result;
}

const QMap<quint32, Core::Talker> &Core::getTalkers()
{
    return talkers;
}

BOOL CALLBACK RecordDataCallback(HRECORD handle, const void *buffer, DWORD length, void *user)
{
    Q_UNUSED(handle);
    reinterpret_cast<Core*>(user)->sendRecordedData(QByteArray(reinterpret_cast<const char*>(buffer),length));

    return TRUE; // continue recording
}

bool Core::startRecording(bool default_device, QString device_id)
{
    if (isRecording())
        stopRecording();

    int device = -1;
    if (!default_device)
    {
        QVector<Core::AudioDevice> devices = getRecordingDevices();

        for (int i = 0; i < devices.count(); i += 1)
            if (devices.at(i).id == device_id)
                device = i;

        if (device == -1)
            return false;
    }

    if (!BASS_RecordInit(device))
        return false;


    channelRecording = BASS_RecordStart(24000,1,MAKELONG(BASS_SAMPLE_8BITS,20),RecordDataCallback,this);

    if (channelRecording <= 0)
    {
        BASS_RecordFree();
        return false;
    }

    stateIsRecording = true;
    return true;
}

void Core::stopRecording()
{
   // if (isRecording())
    {
        BASS_ChannelStop(channelRecording);
        BASS_RecordFree();
    }

    stateIsRecording = false;
}

bool Core::isRecording()
{
    return stateIsRecording;
}

bool Core::startPlayback(bool default_device, QString device_id)
{
    if (isPlayback())
        stopPlayback();

    int device = -1;
    if (!default_device)
    {
        QVector<Core::AudioDevice> devices = getPlaybackDevices();

        for (int i = 0; i < devices.count(); i += 1)
            if (devices.at(i).id == device_id)
                device = i;

        if (device == -1)
            return false;
    }

    if (!BASS_Init(device, 24000, 0,0, nullptr))
        return false;

    stateIsPlayback = true;
    return true;
}

void Core::stopPlayback()
{
     stateIsPlayback = false;

     QMutableMapIterator<quint32, Core::Talker> i(talkers);
     while (i.hasNext())
     {
         i.next();
         if (i.value().stream)
         {
             BASS_ChannelFree(i.value().stream);
             i.value().stream = 0;
         }
     }

     BASS_Free();
}

bool Core::isPlayback()
{
    return stateIsPlayback;
}

void Core::sendRecordedData(QByteArray data)
{
    for (int i = 0; i < data.count(); i += 1)
        if (quint8(data.at(i)) != 0x80)
        {
            lastTalk.restart();
            break;
        }

    if (lastTalk.elapsed() < 5000)
        socket.writeDatagram(data, QHostAddress("255.255.255.255"), 6000);
}

void Core::onSocketRead()
{
    bool new_talker = false;

    while (socket.hasPendingDatagrams())
    {
        QNetworkDatagram datagram = socket.receiveDatagram();

        bool isLocal = false;
        auto local_addresses = QNetworkInterface::allAddresses();
        for (int i = 0; i < local_addresses.count(); i += 1)
            if (local_addresses.at(i).toIPv4Address() == datagram.senderAddress().toIPv4Address())
                isLocal = true;
        if (isLocal)
            continue;

        if (!talkers.contains(datagram.senderAddress().toIPv4Address()))
        {
            talkers[datagram.senderAddress().toIPv4Address()].address = datagram.senderAddress();
            new_talker = true;
        }

        auto& current_talker = talkers[datagram.senderAddress().toIPv4Address()];
        current_talker.lastActivity.restart();

        if (isPlayback())
        {
            if (!current_talker.stream)
            {
                current_talker.stream = BASS_StreamCreate(24000,1,BASS_SAMPLE_8BITS,STREAMPROC_PUSH,0);
                BASS_ChannelPlay(current_talker.stream, true);
            }

            BASS_StreamPutData(current_talker.stream, datagram.data().data(), datagram.data().count());
        }
    }

    if (new_talker)
        emit newTalker();
}
