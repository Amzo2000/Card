#include "animation.h"
#include <QDateTime>
#include "game.h"

Animation::Animation(int durationMs, QObject *parent)
{
    this->parent = parent;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Animation::update);
    duration = durationMs;

    Game *game = qobject_cast<Game*>(parent);
    game->animations.append(this);
}

Animation::Animation(qreal startValue, qreal endValue, int durationMs, QObject *parent) : Animation(durationMs, parent)
{
    this->startValue = startValue;
    this->endValue = endValue;
}

void Animation::start()
{
    if (!timer->isActive()) {
        timer->start(1000 / 120);
        startTime = QDateTime::currentMSecsSinceEpoch();
        emit onUpdate(0);
    }
}

void Animation::restart()
{
    startTime = QDateTime::currentMSecsSinceEpoch();
}

void Animation::stop()
{
    if (timer->isActive()) {
        timer->stop();
        emit onFinished();
        Game *game = qobject_cast<Game*>(parent);
        game->animations.removeOne(this);
        delete this;
    }
}

void Animation::update()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qreal elapsedTime = (qreal) (currentTime - startTime);

    if (elapsedTime < duration) {
        qreal percent = elapsedTime / (qreal) duration;
        if (startValue == endValue)
            emit onUpdate(percent);
        else emit onUpdate(startValue + (endValue - startValue) * percent);
    } else {
        if (startValue == endValue)
            emit onUpdate(1.0);
        else emit onUpdate(endValue);
        stop();
    }

}
