#include "card.h"
#include "game.h"
#include <QPainter>
#include <QRandomGenerator>

Card::Card(QString type, qreal index, QObject *parent)
    : QObject{parent}
{
    this->type = type;
    this->index = index;
    this->pos = QVector2D(100, 100);
    this->size = QVector2D(210, 330) * scale();
    this->initTexture();
    this->shadowTexture.load(":/textures/shadow.png");
    this->ondrag = false;
    this->scaleAnimation = new QPropertyAnimation(this, "scale", this);
    this->rotationAnimation = new QPropertyAnimation(this, "angleY", this);
    this->parent = parent;

    connect(scaleAnimation, scaleAnimation->finished, [this]() {
        initTexture();
    });
}

void Card::initTexture() {
    texture.load(":/textures/" + type + qvariant_cast<QString>(index) + ".png");
    frontImage = texture.scaled(size.x() - size.x() * scaleX, size.y(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    texture.load(":/textures/cardBackground.png");
    backImage = texture.scaled(size.x() - size.x() * scaleX, size.y(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    texture = switched ? backImage : frontImage;
}

void Card::onDrag()
{
    ondrag = true;

    scaleAnimation->stop();
    scaleAnimation->setStartValue(scale());
    scaleAnimation->setEndValue(0.55);
    scaleAnimation->setEasingCurve(QEasingCurve::Linear);
    scaleAnimation->setDuration(100);
    scaleAnimation->start();
}

void Card::noDrag()
{
    ondrag = false;

    scaleAnimation->stop();
    scaleAnimation->setStartValue(scale());
    scaleAnimation->setEndValue(0.4);
    scaleAnimation->setEasingCurve(QEasingCurve::Linear);
    scaleAnimation->setDuration(100);
    scaleAnimation->start();
}

void Card::setToFirstPlace()
{
    Game *game = qobject_cast<Game*>(parent);

    for (int i = 0; i < game->cards.size(); i++) {
        if (game->cards[i] != this) continue;
        game->cards.remove(i);
        break;
    }
    game->cards.append(this);
}

void Card::setShadow(bool allow)
{
    allowShadow = allow;
}

void Card::toggleTurnCard()
{
    rotationAnimation->stop();
    rotationAnimation->setStartValue(0);
    rotationAnimation->setEndValue(1);
    rotationAnimation->setEasingCurve(QEasingCurve::Linear);
    rotationAnimation->setDuration(200);
    rotationAnimation->start();

    connect(rotationAnimation, rotationAnimation->finished, [=]() {
        faceSwitched = switched;
    });
}

void Card::followTarget(QVector2D _target, qreal percent)
{
    if (!ondrag) {
        pos += (_target - pos) * percent;
    }
}

void Card::followTarget()
{
    pos += (target - pos) * 0.1;
}

void Card::reposition()
{
    /*if (!ondrag) {
        pos += (target - pos) * 0.1;
    }*/
}

void Card::draw()
{
    Game *game = qobject_cast<Game*>(parent);
    QPainter painter(game);
    painter.setRenderHints(
        QPainter::Antialiasing,
        QPainter::SmoothPixmapTransform
    );

    painter.translate(pos.x(), pos.y());
    // Ombre.
    if (allowShadow) {
        painter.setOpacity(.8);
        QVector2D s = QVector2D(300, 420) * scale();
        QVector2D shadowDir = pos - QVector2D(game->width() * .5, game->height() * .5);
        shadowDir *= .25;
        shadowDir *= (scale() / 0.4) + (scale() - 0.4) * 5;

        //qreal shMargin = 15 + 30 * (scale() - 0.4) * 10;
        painter.drawPixmap((s.x() - shadowDir.x() - s.x() * scaleX) * -.5, (s.y() - shadowDir.y()) * -.5, s.x() - s.x() * scaleX, s.y(), shadowTexture);
        painter.setOpacity(1);
    }
    // Texture.
    painter.drawPixmap((size.x() - size.x() * scaleX) * -.5, size.y() * -.5, size.x() - size.x() * scaleX, size.y(), texture);
    // Bordure.
    QPen pen(QColor(61, 61, 61));
    pen.setJoinStyle(Qt::MiterJoin);
    if (ondrag) pen.setColor(QColor(0, 255, 0)), pen.setWidth(4);
    painter.setPen(pen);
    painter.drawRect((size.x() - size.x() * scaleX) * -.5, size.y() * -.5, size.x() - size.x() * scaleX, size.y());
    painter.resetTransform();
}

qreal Card::scale() const
{
    return m_scale;
}

void Card::setScale(qreal newScale)
{
    m_scale = newScale;

    this->size = QVector2D(210, 330) * scale();
}

qreal Card::angleY() const
{
    return m_angleY;
}

void Card::setAngleY(qreal newAngleY)
{
    m_angleY = newAngleY;

    scaleX = qAbs(qSin(angleY() * M_PI));

    if (angleY() >= 0.5 && !switched && !faceSwitched) {
        texture = backImage;
        switched = true;
    }
    if (angleY() >= 0.5 && switched && faceSwitched) {
        texture = frontImage;
        switched = false;
    }
}
