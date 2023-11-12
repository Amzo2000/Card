#ifndef STAT_H
#define STAT_H

#include <QObject>
#include <QVector2D>

class Stat : public QObject
{
    Q_OBJECT
public:
    Stat(QVector2D pos, QVector2D size, QObject *parent);

    QVector2D pos;
    QVector2D size;
    qreal scaleX{0};
    QObject *parent;

    bool isActive{false};


    void draw();
};

#endif // STAT_H
