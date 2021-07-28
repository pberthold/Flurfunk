#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <bass.h>
#include <QUdpSocket>
#include <QElapsedTimer>

class Core : public QObject
{
    Q_OBJECT

public:
    struct AudioDevice
    {
        QString name;
        QString id;
    };

    struct Talker
    {
        QHostAddress address;
        QElapsedTimer lastActivity;
        HSTREAM stream{0};
    };

public:
    explicit Core(QObject *parent = nullptr);
    ~Core();
    QVector<AudioDevice> getPlaybackDevices();
    QVector<AudioDevice> getRecordingDevices();
    QMap<quint32, Talker> const& getTalkers();
    bool startRecording(bool default_device, AudioDevice device_id);
    void stopRecording();
    bool isRecording();
    bool startPlayback(bool default_device, AudioDevice device_id);
    void stopPlayback();
    bool isPlayback();
    void sendRecordedData(QByteArray data);

private:
    QUdpSocket socket;
    QMap<quint32, Talker> talkers;
    bool stateIsRecording{false};
    HRECORD channelRecording;
    bool stateIsPlayback{false};
    QElapsedTimer lastTalk;

private slots:
    void onSocketRead();

signals:
    void newTalker();

};

#endif // CORE_H
