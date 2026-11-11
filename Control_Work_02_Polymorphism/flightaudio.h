#pragma once

#include <QObject>
class QMediaPlayer;

class FlightAudio : public QObject {
public:
    explicit FlightAudio(QObject* parent = nullptr);
    void set_enabled(bool enabled);
    void start();
    void stop();
    void alert();
private:
    bool enabled_ = true;
    bool flying_ = false;
    QMediaPlayer* flight_;
    QMediaPlayer* warning_;
};
