#ifndef TABLE_H
#define TABLE_H

#include <QObject>
#include "card.h"

class Table : public QObject
{
    Q_OBJECT
public:
    explicit Table(QVector2D pos, QObject *parent = nullptr);

    QVector2D pos;
    QVector2D size;
    QList<Card*> cards;
    QList<QVector2D> cardsOrigins;
    bool onCollecting{false};
    bool onDarbaCollecting{false};
    Card *cardCollector;
    QList<Card*> cardsCollected;
    Card *cardToFollow;
    bool isFinishCollecting{false};
    int playerToCollect;
    bool onCollectingLastCards{false};
    qreal time{0};
    QObject *parent;

    Card *lastCard{nullptr};

    bool derbaRepeated{false};
    int derbaCollecteur;
    int derbaCount{0};
    QList<Card*> derbaCards;
    qreal darbaProgress{0};
    QVector2D darbaPos;

    void onCollectingCards();
    void onDerbaCollect(Card *derbaTarget);
    void reposition();
    void draw();
    void addCard(Card *card);
    void removeCard(Card *card);
    void initSize();
    void initCardOrigins();
    void collectCard(Card *card, int mode);
    bool collide(QVector2D pos);

signals:

};

#endif // TABLE_H
