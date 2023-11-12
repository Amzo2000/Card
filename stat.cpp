#include "stat.h"
#include "game.h"
#include <QPainter>

Stat::Stat(QVector2D pos, QVector2D size, QObject *parent)
{
    this->pos = pos;
    this->size = size;
    this->parent = parent;
}

void Stat::draw() {
    Game *game = qobject_cast<Game*>(parent);
    QPainter painter(game);

    QString message = game->winner == 1 ? "🎉 Bravo, vous avez gagné(e)." : game->winner == 0 ? "🤝 Équivalent." : "😕 Vous avez perdu(e).";

    painter.fillRect(0, 0, game->width() * scaleX, game->height(), QBrush(QColor(0, 0, 0, 200)));

    painter.setPen(QPen(QColor(255, 255, 255)));
    painter.setFont(QFont("Times", 18, 600));
    painter.drawText(0, 0, game->width() * scaleX, game->height(), Qt::AlignCenter, message);
}
