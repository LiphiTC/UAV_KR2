#include "flightaudio.h"
#include <QMediaPlayer>
#include <QUrl>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#else
#include <QMediaPlaylist>
#endif

FlightAudio::FlightAudio(QObject* parent)
    : QObject(parent), flight_(new QMediaPlayer(this)), warning_(new QMediaPlayer(this))
{
    flight_->setObjectName("flightSoundPlayer");
    warning_->setObjectName("warningSoundPlayer");
    for (QMediaPlayer* player : {flight_, warning_}) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto* output = new QAudioOutput(player);
        output->setVolume(player == flight_ ? 0.25f : 0.5f);
        player->setAudioOutput(output);
#else
        player->setVolume(player == flight_ ? 25 : 50);
#endif
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    flight_->setLoops(QMediaPlayer::Infinite);
    flight_->setSource(QUrl("qrc:/assets/drone_sound.mp3"));
    warning_->setSource(QUrl("qrc:/assets/critical.mp3"));
#else
    auto* playlist = new QMediaPlaylist(flight_);
    playlist->addMedia(QUrl("qrc:/assets/drone_sound.mp3"));
    playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    flight_->setPlaylist(playlist);
    warning_->setMedia(QUrl("qrc:/assets/critical.mp3"));
#endif
}
void FlightAudio::set_enabled(bool enabled)
{
    enabled_ = enabled;
    if (!enabled_) {
        flight_->stop();
        warning_->stop();
    } else if (flying_) {
        flight_->play();
    }
}
void FlightAudio::start()
{
    flying_ = true;
    warning_->stop();
    if (enabled_) flight_->play();
}
void FlightAudio::stop()
{
    flying_ = false;
    flight_->stop();
    warning_->stop();
}
void FlightAudio::alert()
{
    stop();
    if (enabled_) {
        warning_->setPosition(0);
        warning_->play();
    }
}
