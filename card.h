#ifndef CARD_H
#define CARD_H

#include <QObject>
#include <QVector2D>
#include <QPropertyAnimation>
#include <QPixmap>

class Card : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
    Q_PROPERTY(qreal angleY READ angleY WRITE setAngleY)

public:
    Card(QString type, qreal index, QObject *parent = nullptr);
    /*Card &operator =(const Card& other);
    Card(const Card& other);*/

    QVector2D pos;
    QVector2D size;
    QString type;
    qreal index;
    bool ondrag;
    QVector2D target;
    bool switched = true;
    QVector2D unleashed_pos{-100, -100};
    qreal scaleX = 0;
    bool distributed{false};
    QObject *parent;

    QPixmap texture;
    QPixmap shadowTexture;
    QPixmap frontImage;
    QPixmap backImage;

    bool freezed{false};


    void initTexture();
    void reposition();
    void draw();
    void onDrag();
    void noDrag();
    void setToFirstPlace();
    void setShadow(bool allow);
    void toggleTurnCard();
    void followTarget(QVector2D _target, qreal percent = .1);
    void followTarget();

    qreal scale() const;
    void setScale(qreal newScale);
    qreal angleY() const;
    void setAngleY(qreal newAngleY);

private:
    bool allowShadow{false};
    qreal m_scale{0.4};
    QPropertyAnimation *scaleAnimation;
    QPropertyAnimation *rotationAnimation;
    qreal m_angleY;

    bool faceSwitched = true;
};

#endif // CARD_H
