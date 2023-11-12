#include "scoreAnimation.h"
#include "game.h"
#include <QPainter>
#include <QDebug>

ScoreAnimation::ScoreAnimation(QVector2D pos, int score, QObject *parent)
{
    this->pos = pos;
    this->origin = pos;
    this->score = score;
    this->parent = parent;
    Game *game = qobject_cast<Game*>(parent);
    game->scoreAnimations.append(this);
    start();
}

void ScoreAnimation::start()
{
    Game *game = qobject_cast<Game*>(parent);
    animation = new Animation(1000, game);
    connect(animation, animation->onUpdate, [this](float x) {
        size = 14.0 * sqrt(x);
        pos = origin + QVector2D(0, -50) * x;
    });
    connect(animation, animation->onFinished, [this]() {
        Game *game = qobject_cast<Game*>(parent);
        game->scoreAnimations.removeOne(this);
        delete this;
    });
    animation->start();
}

void ScoreAnimation::draw()
{
    Game *game = qobject_cast<Game*>(parent);
    QPainter painter(game);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.setFont(QFont("Times", size, 600));
    painter.setPen(QColor(255, 255, 255, 255 * (1 - pow(size / 14.0, 2))));
    painter.translate(pos.x(), pos.y());
    painter.drawText(32 * -.5, 32 * -.5, 32, 32, Qt::AlignCenter, "+" + QString::number(score));
    painter.resetTransform();
}
