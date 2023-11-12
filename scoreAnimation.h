#ifndef SCOREANIMATION_H
#define SCOREANIMATION_H

#include <QObject>
#include <QVector2D>
#include "animation.h"

class ScoreAnimation : public QObject
{
    Q_OBJECT
public:
    ScoreAnimation(QVector2D pos, int score, QObject *parent);
    QVector2D pos;
    QVector2D origin;
    int score;
    qreal size;

    QObject *parent;

    Animation *animation;

    void start();
    void draw();
};

#endif // SCOREANIMATION_H
