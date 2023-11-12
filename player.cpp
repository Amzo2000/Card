#include "player.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include "game.h"
#include "animation.h"
#include <QRandomGenerator>

Player::Player(QVector2D pos, int mode, QObject *parent)
    : QObject{parent}
{
    this->pos = pos;
    this->mode = mode;
    this->parent = parent;
}

void Player::annonceCard(Card *card)
{
    Table *table = qobject_cast<Game*>(parent)->table;
    card->setToFirstPlace();
    table->collectCard(card, mode);
    removeCard(card);
    togglePlay();
}

void Player::addCard(Card *card)
{
    card->setShadow(true);
    card->setToFirstPlace();
    cards.append(card);

    initCardOrigins();
}

void Player::removeCard(Card *card)
{
    card->setShadow(false);
    cards.removeOne(card);
    initCardOrigins();
}

void Player::initCardOrigins()
{
    cardsOrigins.clear();
    for (qreal i = (qreal) cards.size() * -.5f; i < (qreal) cards.size() * .5f; i++) {
        const int step = 90;
        cardsOrigins.append(pos + QVector2D(step * (i + .5), 0));
    }
}

bool collideToTable(QVector2D pos, QRectF rect)
{
    if (pos.x() >= rect.x() - rect.width() * .5 && pos.x() <= rect.x() + rect.width() * .5) {
        return (pos.y() >= rect.y() - rect.height() * .5 && pos.y() <= rect.y() + rect.height() * .5);
    }
    return false;
}

Player *Player::getAdversary()
{
    Game *game = qobject_cast<Game*>(parent);
    return mode == Game::PLAYER ? game->ai : game->player;
}

void Player::reposition()
{
    Game *game = qobject_cast<Game*>(parent);
    if (mode == Game::AI && game->canPlay) autoPlay();
    if (cardsCollected.size() > 0) {
        int i = 0;
        for (Card *card : cardsCollected) {
            card->followTarget(cardsCollectedPos + QVector2D(0,  - i * .5));
            i++;
        }
    }

    if (cards.size() == 0 && game->isCardsConfigured) {
        cardsIsEmpty = true;
    }

    if (!game->paused && stopped && mode == Game::AI) {
        onPlaying = false;
        stopped = false;
    }

    if (cardsIsEmpty && !game->table->onCollecting) {
        if (game->remainCards != 0) {
            if (game->distributionCards(mode, cards, 3, mode == Game::PLAYER)) {
                cardsIsEmpty = false;
                if (getAdversary()->cards.size() == 3) {
                    game->isReloadedCards = true;
                }
            }
        }
    }

    int i = 0;
    for (Card *card : cards) {
        card->followTarget(cardsOrigins[i]);
        i++;
    }

    for (Card *card : cards) {
        if (!card->ondrag && mode == Game::PLAYER) {
            Table *table = qobject_cast<Game*>(parent)->table;
            if (table->collide(card->unleashed_pos)) {
                annonceCard(card);
            }
            card->unleashed_pos = QVector2D(-100, -100);
        }
    }
}

void Player::togglePlay()
{
    Game *game = qobject_cast<Game*>(parent);
    canPlay = false;
    getAdversary()->canPlay = true;
}

void Player::autoPlay()
{
    if (!onPlaying && canPlay) {
        Game *game = qobject_cast<Game*>(parent);
        if (cards.size() == 0) return;

        Animation *waitToSwitchedCard = new Animation(500, game);
        connect(waitToSwitchedCard, waitToSwitchedCard->onFinished, [this, game, waitToSwitchedCard]() {
            if (game->paused) { stopped = true; return; }
            Card *card = cards[QRandomGenerator::global()->bounded(cards.size())];
            if (card->freezed) { stopped = true; return; }
            if (cardChoosed == nullptr) cardChoosed = card;
            if (cardChoosed->switched) cardChoosed->toggleTurnCard();

            Animation *waitToAnnonce = new Animation(1000, game);
            connect(waitToAnnonce, waitToAnnonce->onFinished, [this, game, waitToAnnonce]() {
                if (game->paused) { stopped = true; return; }
                annonceCard(cardChoosed);
                onPlaying = false;
                cardChoosed = nullptr;
            });
            waitToAnnonce->start();
        });
        waitToSwitchedCard->start();

        onPlaying = true;
    }
}

void Player::draw()
{
    /*Game *game = qobject_cast<Game*>(parent);

    QPainter painter(game);
    QBrush brush(QColor(255, 0, 0), Qt::SolidPattern);
    for (const QVector2D vector : cardsOrigins) {
        QPainterPath path;
        const int radius = 10;
        path.addEllipse(QPointF(vector.x(), vector.y()), radius, radius);
        painter.fillPath(path, brush);
    }*/
}
