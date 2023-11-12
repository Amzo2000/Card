#ifndef ANIMATION_H
#define ANIMATION_H

#include <QObject>
#include <QTimer>

class Animation : public QObject
{
    Q_OBJECT
public:
    Animation(int durationMs, QObject *parent);
    Animation(qreal startValue, qreal endValue, int durationMs, QObject *parent);

    QObject *parent;

    void start();
    void restart();
    void stop();

signals:
    void onUpdate(float progress);
    void onFinished();

private slots:
    void update();

private:
    qreal startValue{0};
    qreal endValue{0};
    QTimer *timer;
    int duration;
    qint64 startTime;

};

#endif // ANIMATION_H
