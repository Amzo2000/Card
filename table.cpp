#include "table.h"
#include <QVector2D>
#include "game.h"
#include <QPainter>
#include <QPainterPath>
#include <QTime>
#include "player.h"
#include "animation.h"

Table::Table(QVector2D pos, QObject *parent)
    : QObject{parent}
{
    this->pos = pos;
    this->parent = parent;
    initSize();
    initCardOrigins();
}

void Table::addCard(Card *card)
{
    card->setToFirstPlace();
    cards.append(card);

    initCardOrigins();
}

void Table::removeCard(Card *card)
{
    cards.removeOne(card);
}

void Table::initSize()
{
    Game *game = qobject_cast<Game*>(parent);

    if (cards.size() <= 5) {
        size = QVector2D(game->width(), 150);
    } else {
        size = QVector2D(game->width(), 290);
    }
}

void Table::initCardOrigins()
{
    cardsOrigins.clear();

    int index = 0;
    qreal length = cards.size() < 5 ? cards.size() : 5;
    for (qreal i = -length * .5; i < length * .5; i++) {
        if (cardsOrigins.size() == 5) break;
        const qreal yStep = cards.size() > 5 ? -70 : 0;
        const qreal step = 93;
        cardsOrigins.append(pos + QVector2D(step * (i + .5), yStep));
        cards[index]->target = cardsOrigins[index];
        index++;
    }
    if (cards.size() > 5) {
        length = cards.size() - 5;
        for (qreal i = -length * .5; i < length * .5; i++) {
            const qreal step = 93;
            cardsOrigins.append(pos + QVector2D(step * (i + .5), 70));
            cards[index]->target = cardsOrigins[index];
            index++;
        }
    }
    initSize();
}

Card* isMatchToCards(int cardIndex, QList<Card*> cards)
{
    Card *cardToReturn = nullptr;
    for (Card *_card : cards) {
        if (cardIndex != _card->index) continue;
        cardToReturn = _card;
        break;
    }
    return cardToReturn;
}

void freezCards(int index, QList<Card*> _cards) {
    for (Card *card : _cards) {
        if (card->index != index) card->freezed = true;
    }
}

void unFreezCards(QList<Card*> _cards) {
    for (Card *card : _cards) {
        card->freezed = false;
    }
}

void Table::onDerbaCollect(Card *derbaTarget)
{
    Game *game = qobject_cast<Game*>(parent);

    Animation *derbaAnimation = new Animation(3000, game);
    connect(derbaAnimation, derbaAnimation->onUpdate, [=](float value) {
        darbaProgress = value;
        for (Card *card : derbaCards) {
            card->followTarget(derbaTarget->pos);
        }
        if (derbaRepeated) { derbaAnimation->restart(); derbaRepeated = false; }

        if (derbaCount > 2) derbaAnimation->stop();
    });
    connect(derbaAnimation, derbaAnimation->onFinished, [this, game, derbaTarget]() {
        Player *currentPlayer = derbaCollecteur == Game::PLAYER ? game->player : game->ai;
        currentPlayer->score += derbaCount == 1 ? 1 : derbaCount == 2 ? 5 : 10;

        derbaCount = 0;

        onDarbaCollecting = false;
        game->canPlay = false;

        lastCard = nullptr;
        playerToCollect = derbaCollecteur;
        onCollecting = true;
        cardCollector = derbaTarget;

        for (Card *card : derbaCards) {
            cardsCollected.append(card);
        }
        derbaCards.clear();
    });
    derbaAnimation->start();
}

void Table::collectCard(Card *card, int mode)
{
    Game *game = qobject_cast<Game*>(parent);
    Card *isMatch = isMatchToCards(card->index, cards);
    if (lastCard != nullptr && card->index == lastCard->index) {
        Player *currentPlayer = mode == Game::PLAYER ? game->player : game->ai;
        freezCards(isMatch->index, currentPlayer->getAdversary()->cards);
        derbaCollecteur = mode;
        derbaCards.append(card);
        if (!onDarbaCollecting) onDerbaCollect(isMatch);
        else derbaRepeated = true;
        onDarbaCollecting = true;

        int derbaScore = derbaCount == 0 ? 1 : derbaCount == 1 ? 5 : 10;

        Animation *showScoreDerba = new Animation(200, game);
        connect(showScoreDerba, showScoreDerba->onFinished, [=]() {
            new ScoreAnimation(isMatch->pos, derbaScore, game);
        });
        showScoreDerba->start();
        darbaPos = lastCard->pos;
        derbaCount++;
    } else {
        if (isMatch == nullptr) {
            lastCard = card;
            addCard(card);
        } else {
            lastCard = nullptr;
            playerToCollect = mode;
            onCollecting = true;
            cardCollector = card;
            game->canPlay = false;
        }
    }
}

void Table::onCollectingCards()
{
    Game *game = qobject_cast<Game*>(parent);
    time += 1000.0 / game->FPS;

    Card *isMatch = isMatchToCards(cardCollector->index, cards);
    if (isMatch != nullptr) cardToFollow = isMatch;

    if (time >= 500.0 || isMatch == nullptr) {
        if (isMatch == nullptr) {
            isMatch = isMatchToCards(cardCollector->index + 1, cards);
            if (isMatch != nullptr) cardToFollow = isMatch;
            if (time - 200 >= 200.0 || isMatch == nullptr) {
                if (isMatch != nullptr) {
                    if (playerToCollect == Game::PLAYER) game->player->score += 1;
                    else game->ai->score += 1;
                    new ScoreAnimation(isMatch->pos, 1, game);
                    removeCard(isMatch);
                    cardCollector->setToFirstPlace();
                    cardsCollected.append(cardCollector);
                    cardCollector = isMatch;
                    cardCollector->setToFirstPlace();
                } else {
                    cardsCollected.append(cardCollector);
                    onCollecting = false;
                    isFinishCollecting = true;
                    cardCollector = nullptr;
                }
                time = 0;
            }
        } else {
            if (playerToCollect == Game::PLAYER) game->player->score += 2;
            else game->ai->score += 2;
            new ScoreAnimation(isMatch->pos, 2, game);
            removeCard(isMatch);
            isMatch->setToFirstPlace();
            cardCollector->setToFirstPlace();
            cardsCollected.append(isMatch);
            time = 0;
        }
    }

    if(cardCollector != nullptr) cardCollector->followTarget(cardToFollow->pos);
}

void Table::reposition()
{
    Game *game = qobject_cast<Game*>(parent);

    if (onCollecting) onCollectingCards();
    if (cardsCollected.size() > 0) {
        for (Card *card : cardsCollected) {
            if (cardCollector == nullptr) continue;
            card->followTarget(cardCollector->pos, 0.5);
        }
        if (isFinishCollecting) {
            Player *playerCollector = playerToCollect == Game::PLAYER ? game->player : game->ai;
            for (Card *card : cardsCollected) {
                card->setToFirstPlace();
                card->toggleTurnCard();
                playerCollector->cardsCollected.append(card);
            }
            cardsCollected.clear();
            initCardOrigins();
            isFinishCollecting = false;
            game->canPlay = true;
            unFreezCards(game->player->cards);
            unFreezCards(game->ai->cards);

            // Verfie s'il y a de Missa.
            if (cards.size() == 0) {
                game->isMissa = true;
                Animation *missaAnimation = new Animation(1000, game);
                connect(missaAnimation, missaAnimation->onUpdate, [game](float x) {
                    qreal f = sin(M_PI * x);
                    game->missaProgress = f;
                });
                connect(missaAnimation, missaAnimation->onFinished, [playerCollector, game]() {
                    playerCollector->score += 1;
                    game->isMissa = false;
                    game->missaProgress = 0;
                    game->canPlay = true;
                });
                missaAnimation->start();
                game->canPlay = false;
            }
        }
    }
    for (Card *card : cards) {
        card->followTarget();
    }

    if (game->isGameFinish && !onCollectingLastCards) {
        QList<Animation*> animations;

        if (!cards.size()) {
            game->finished = true;
            onCollectingLastCards = true;
            return;
        }

        animations.append(new Animation(1000, game));
        animations.append(new Animation(1000, game));

        connect(animations[0], animations[0]->onFinished, [this, game, animations]() {
            for (Card *card : cards) {
                new ScoreAnimation(card->pos, 1, game);
                card->setToFirstPlace();
                card->toggleTurnCard();
            }
            animations[1]->start();
        });
        animations[0]->start();

        connect(animations[1], animations[1]->onFinished, [this, game]() {
            for (Card *card : cards) {
                if (playerToCollect == Game::PLAYER) {
                    game->player->cardsCollected.append(card);
                    game->player->score += 1;
                } else {
                    game->ai->cardsCollected.append(card);
                    game->ai->score += 1;
                }
                removeCard(card);
            }
            game->finished = true;
        });

        onCollectingLastCards = true;
    }
}

bool Table::collide(QVector2D mouse_pos)
{
    if (mouse_pos.x() >= pos.x() - size.x() * .5 && mouse_pos.x() <= pos.x() + size.x() * .5) {
        return (mouse_pos.y() >= pos.y() - size.y() * .5 && mouse_pos.y() <= pos.y() + size.y() * .5);
    }
    return false;
}

void Table::draw()
{
    Game *game = qobject_cast<Game*>(parent);
    Player *player = game->player;

    bool ondrag = false;
    bool collided = false;
    for (Card *card : player->cards) {
        if (!card->ondrag) continue;
        if (collide(game->mouse.pos)) {
            collided = true;
        }
        ondrag = true;
        break;
    }
    if (ondrag) {
        QTime time;
        qreal offset = time.currentTime().msecsSinceStartOfDay();

        QPainter painter(game);
        painter.setOpacity(.5);
        QList<QLine> lines;
        lines.append(QLine(0, pos.y() - size.y() * .5, size.x(), pos.y() - size.y() * .5));
        lines.append(QLine(0, pos.y() + size.y() * .5, size.x(), pos.y() + size.y() * .5));
        QPen pen(QColor(61, 61, 61));
        if (collided) pen.setColor(QColor(150, 255, 255));
        pen.setWidth(4);
        pen.setStyle(Qt::DashLine);
        pen.setDashOffset(offset * .01);
        painter.setPen(pen);
        painter.drawLine(lines[0]);
        pen.setDashOffset(offset * -.01);
        painter.setPen(pen);
        painter.drawLine(lines[1]);
    }

    /*QBrush brush(QColor(255, 0, 0), Qt::SolidPattern);
    for (const QVector2D vector : cardsOrigins) {
        QPainterPath path;
        const int radius = 10;
        path.addEllipse(QPointF(vector.x(), vector.y()), radius, radius);
        painter.fillPath(path, brush);
    }*/

}
