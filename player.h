#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QVector2D>
#include "card.h"

class Player : public QObject
{
    Q_OBJECT
public:
    explicit Player(QVector2D pos, int mode, QObject *parent = nullptr);

    QVector2D pos;
    QList<Card*> cards;
    QVector2D cardsCollectedPos;
    QList<Card*> cardsCollected;
    QList<QVector2D> cardsOrigins;
    QObject *parent;
    int mode;

    bool canPlay{false};
    bool onPlaying{false};

    bool cardsIsEmpty{false};

    int score{0};

    bool stopped{false};
    Card *cardChoosed{nullptr};

    void annonceCard(Card *card);
    void addCard(Card *card);
    void removeCard(Card *card);
    void initCardOrigins();
    void reposition();
    void togglePlay();
    void autoPlay();
    Player *getAdversary();
    void draw();
};

#endif // PLAYER_H
