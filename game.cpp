#include "game.h"
#include "./ui_game.h"

#include <QPainter>
#include <QPaintEngine>
#include <QTimer>
#include <QVector2D>
#include <QRandomGenerator>
#include <QMouseEvent>
#include "animation.h"
#include <QDateTime>
#include <QPainterPath>
#include <QPushButton>
#include <QStyle>

Game::Game(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Game)
{
    ui->setupUi(this);

    setMinimumSize(800, 635);
    setWindowTitle("Ronda");

    QPushButton *restartButton = ui->restartButton;
    restartButton->setStyleSheet("background-color: #00000000");
    restartButton->setIcon(QIcon(":/textures/restart.png"));
    restartButton->setIconSize(QSize(32, 32));

    QPushButton *pauseButton = ui->pauseButton;
    pauseButton->setStyleSheet("background-color: #00000000");
    pauseButton->setIcon(QIcon(":/textures/pause.png"));
    pauseButton->setIconSize(QSize(32, 32));

    connect(pauseButton, pauseButton->clicked, [=]() {
        paused = !paused;
        pauseButton->setIcon(QIcon(paused ? ":/textures/play.png" : ":/textures/pause.png"));
    });

    connect(restartButton, restartButton->clicked, [=]() {
        restart();
        restartButton->setEnabled(false);
        restartButton->setIcon(QIcon(":/textures/restart-disabled.png"));

        Animation *waitToEnableRestartButton = new Animation(1000, this);
        connect(waitToEnableRestartButton, waitToEnableRestartButton->onFinished, [restartButton]() {
            restartButton->setIcon(QIcon(":/textures/restart.png"));
            restartButton->setEnabled(true);
        });
        waitToEnableRestartButton->start();
    });


    initCards();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (!paused) reposition();

        now = QDateTime::currentMSecsSinceEpoch();
        if (now - lastTime >= 1000) {
            frame_to_show = frame;
            lastTime = now;
            frame = 0;
        }
        frame ++;
    });
    timer->start(1000 / FPS);

    margin = QVector2D(0, 40);
    dealer_pos = QVector2D(width() - 60, (height() - margin.y()) * .5);

    table = new Table(QVector2D(width() * .5, (height() - margin.y()) * .5), this);

    player = new Player(QVector2D(width() * .5, height() - 120), PLAYER, this);
    player->cardsCollectedPos = QVector2D(80, height() - (100 + margin.y()));
    player->canPlay = true;

    ai = new Player(QVector2D(width() * .5, 80), AI, this);
    ai->cardsCollectedPos = QVector2D(80, 100);

    now = QDateTime::currentMSecsSinceEpoch();
    lastTime = QDateTime::currentMSecsSinceEpoch();

    light.load(":/textures/light.png");

    stat = new Stat(QVector2D(0, 0), QVector2D(width(), height()), this);
}

// Distribution des cartes.
bool Game::distributionCards(int mode, QList<Card*> &parent_cards, int cardLength, bool show)
{
    if (parent_cards.size() == cardLength) return true;

    time += 1000.0 / FPS;

    if (time >= 100) {
        Card *cardToDistribute;
        if (mode != TABLE) {
            for (int i = cards.size() - 1; i >= 0; i--) {
                Card *card = cards[i];
                if (card->distributed) continue;
                card->setToFirstPlace();
                cardToDistribute = card;
                break;
            }
        }
        // Distribuer les cartes vers la table.
        if (mode == TABLE) {
            Card *cardToMiddle;
            for (int i = cards.size() - 1; i >= 0; i--) {
                Card *card = cards[i];
                if (card->distributed) continue;
                bool reject = false;
                for (Card *_card : table->cards) {
                    if (card->index == _card->index - 1) reject = true;
                    if (card->index == _card->index) reject = true;
                    if (card->index == _card->index + 1) reject = true;
                }
                if (reject) continue;
                card->setToFirstPlace();
                cardToMiddle = card;
                break;
            }

            table->addCard(cardToMiddle);
            if (show) cardToMiddle->toggleTurnCard();
            cardToMiddle->distributed = true;

        } else if (mode == PLAYER) {
            player->addCard(cardToDistribute);
            if (show) cardToDistribute->toggleTurnCard();
            cardToDistribute->distributed = true;
        } else if (mode == AI) {
            ai->addCard(cardToDistribute);
            if (show) cardToDistribute->toggleTurnCard();
            cardToDistribute->distributed = true;
        }
        time = 0;
    }

    return parent_cards.size() == cardLength;
}

void Game::initCards()
{
    QString types[] = { "A", "B", "C", "D" };

    for (int i = 0; i < 40; i++) {
        QString type = types[i / 10];
        qreal index = i % 10 + 1;
        Card *card = new Card(type, index, this);
        card->pos = dealer_pos;
        cards.append(card);
    }

    for (int i = 0; i < cards.size(); i++) {
        int j = QRandomGenerator::global()->bounded(cards.size());
        cards.swapItemsAt(i, j);
    }
}

Game::~Game()
{
    delete ui;
}

void Game::restart()
{

    delete table;
    delete player;
    delete ai;

    for (Animation *anim : animations) {
        delete anim;
    }
    animations.clear();

    for (ScoreAnimation *sAnim : scoreAnimations) {
        delete sAnim;
    }
    scoreAnimations.clear();

    for (Card *card : cards) {
        delete card;
    }
    cards.clear();
    initCards();

    paused = false;
    QPushButton *pauseButton = ui->pauseButton;
    pauseButton->setVisible(true);
    pauseButton->setIcon(QIcon(":/textures/pause.png"));

    QPushButton *restartButton = ui->restartButton;
    restartButton->setIcon(QIcon(":/textures/restart.png"));

    finished = false;

    canPlay = false;
    remainCards = 0;
    time = 0;
    isCardsConfigured = false;
    isGameFinish = false;
    isReloadedCards = false;
    onRonda = false;
    rondaBonusCount.clear();
    rondaScores.clear();
    winnersRonda.clear();
    onRondaProgress = 0;

    isMissa = false;
    missaProgress = 0;

    firstToPlay = !firstToPlay;

    table = new Table(QVector2D(width() * .5, (height() - margin.y()) * .5), this);

    player = new Player(QVector2D(width() * .5, height() - 120), PLAYER, this);
    player->cardsCollectedPos = QVector2D(80, height() - (100 + margin.y()));
    player->canPlay = firstToPlay;

    Animation *statAnimation = new Animation(1, 0, 200, this);
    connect(statAnimation, statAnimation->onUpdate, [this](float value) {
        stat->scaleX = value;
    });
    connect(statAnimation, statAnimation->onFinished, [this]() {
        stat->isActive = false;
    });
    statAnimation->start();

    ai = new Player(QVector2D(width() * .5, 80), AI, this);
    ai->cardsCollectedPos = QVector2D(80, 100);
    ai->canPlay = !firstToPlay;
}

void Game::background(QColor color)
{
    QPainter painter(this);
    QBrush brush(color, Qt::SolidPattern);
    painter.fillRect(0, 0, width(), height(), brush);
}

QList<Card*> sameCards(QList<Card*> _cards) {
    QList<Card*> _sameCards;
    for (int i = 0; i < _cards.size(); i++) {
        Card *card = _cards[i];
        for (int j = i + 1; j < _cards.size(); j++) {
            Card *nextCard = _cards[j];
            if (card->index == nextCard->index) {
                if (!_sameCards.contains(card)) _sameCards.append(card);
                if (!_sameCards.contains(nextCard)) _sameCards.append(nextCard);
            }
        }
    }
    return _sameCards;
}

void Game::rondaBonus()
{
    QList<Card*> player_sameCards = sameCards(player->cards);
    QList<Card*> ai_sameCards = sameCards(ai->cards);
    Player *currentPlayer = nullptr;
    if (player_sameCards.size() >= 2) currentPlayer = player;
    else if (ai_sameCards.size() >= 2) currentPlayer = ai;
    if (currentPlayer == nullptr) return;

    onRonda = true;

    Player *adversary = currentPlayer->getAdversary();

    QList<Card*> _sameCards = sameCards(currentPlayer->cards);
    QList<Card*> _anotherSameCards = sameCards(adversary->cards);

    int score = 0;
    int winner;

    if (_sameCards.size() == 2) {
        if (_anotherSameCards.size() == 2) {
            if (_sameCards[0]->index < _anotherSameCards[0]->index) {
                score = 2;
                winner = adversary->mode;
            } else if (_sameCards[0]->index == _anotherSameCards[0]->index) {
                score = 1;
                winner = -1;
            } else {
                score = 2;
                winner = currentPlayer->mode;
            }
        } else if (_anotherSameCards.size() == 3) {
            score = 6;
            winner = adversary->mode;
        } else {
            score = 1;
            winner = currentPlayer->mode;
        }
    } else if (_sameCards.size() == 3) {
        if (_anotherSameCards.size() == 3) {
            if (_sameCards[0]->index < _anotherSameCards[0]->index) {
                score = 10;
                winner = adversary->mode;
            } else if (_sameCards[0]->index == _anotherSameCards[0]->index) {
                score = 5;
                winner = -1;
            } else {
                score = 10;
                winner = currentPlayer->mode;
            }
        } else if (_anotherSameCards.size() == 2) {
            score = 6;
            winner = currentPlayer->mode;
        } else {
            score = 5;
            winner = currentPlayer->mode;
        }
    }

    if (winner == currentPlayer->mode) {
        winnersRonda.append(currentPlayer);
        rondaScores.append(score);
        rondaBonusCount.append(_sameCards.size());
        if (_anotherSameCards.size() >= 2) {
            winnersRonda.append(adversary);
            rondaScores.append(0);
            rondaBonusCount.append(_anotherSameCards.size());
        }
    } else if (winner == adversary->mode) {
        winnersRonda.append(adversary);
        rondaScores.append(score);
        rondaBonusCount.append(_anotherSameCards.size());
        if (_sameCards.size() >= 2) {
            winnersRonda.append(currentPlayer);
            rondaScores.append(0);
            rondaBonusCount.append(_sameCards.size());
        }
    } else {
        winnersRonda.append(currentPlayer);
        winnersRonda.append(adversary);
        rondaScores.append(score);
        rondaScores.append(score);
        rondaBonusCount.append(_sameCards.size());
        rondaBonusCount.append(_anotherSameCards.size());
    }

    Animation *ronda = new Animation(1000, this);
    connect(ronda, ronda->onUpdate, [=](float value) {
        onRondaProgress = value;
        canPlay = false;
    });
    connect(ronda, ronda->onFinished, [=]() {
        for (int i = 0; i < winnersRonda.size(); i++) {
            winnersRonda[i]->score += rondaScores[i];
        }
        canPlay = true;
        onRonda = false;
        winnersRonda.clear();
        rondaScores.clear();
        rondaBonusCount.clear();
        onRondaProgress = 0;
    });
    ronda->start();
}

void Game::reposition()
{
    // Distribuer les cartes.
    if (!isCardsConfigured) {
        if (distributionCards(TABLE, table->cards, 4, true)) {
            if (distributionCards(PLAYER, player->cards, 3, true)) {
                if (distributionCards(AI, ai->cards, 3, false)) {
                    isCardsConfigured = true;
                    isReloadedCards = true;
                    canPlay = true;
                }
            }
        }
    }
    // Verifier si tout les joueurs on eu leurs cartes.
    if (isReloadedCards) {
        table->lastCard = nullptr;
        // Verifier s'il y a ronda ou tringa.
        rondaBonus();
        isReloadedCards = false;
    }

    int i = 0;
    remainCards = 0;
    for (Card *card : cards) {
        if (!card->distributed) {
            remainCards++;
            card->pos = dealer_pos + QVector2D(0, -i * .25);
        }
        card->reposition();
        i++;
    }
    table->reposition();
    player->reposition();
    ai->reposition();

    // Verifier si le jeu est terminé.
    if (!remainCards && !player->cards.size() && !ai->cards.size() && !table->onCollecting && !table->onDarbaCollecting) {
        isGameFinish = true;
        if (!stat->isActive && finished) {
            // Setup Winner.
            winner = player->score > ai->score ? 1 : player->score == ai->score ? 0 : -1;
            Animation *waitStatAnimation = new Animation(500, this);
            connect(waitStatAnimation, waitStatAnimation->onFinished, [this]() {
                QPushButton *pauseButton = ui->pauseButton;
                pauseButton->setVisible(false);
                Animation *statAnimation = new Animation(200, this);
                connect(statAnimation, statAnimation->onUpdate, [this](float value) {
                    stat->scaleX = value;
                });
                statAnimation->start();
            });
            waitStatAnimation->start();
            stat->isActive = true;
        }
    }

    update();
}


void Game::paintEvent(QPaintEvent *event)
{
    background(QColor(226, 156, 38));
    table->draw();
    for (Card *card : cards) card->draw();
    player->draw();
    ai->draw();

    QPainter painter(this);
    painter.setRenderHints(
        QPainter::Antialiasing,
        QPainter::SmoothPixmapTransform
    );

    // Afficher la lumiere.
    painter.setOpacity(.85);
    painter.drawImage(QRect(0, 0, width(), height()), light);
    painter.setOpacity(1);


    // Rectangle qui est en bottom.
    qreal h = 35;
    QVector2D size(width(), h);
    painter.fillRect(0, height() - size.y(), size.x(), size.y(), QBrush(QColor(255, 255, 255)));

    qreal r = 21;

    QPainterPath path;
    path.addEllipse(-(r + (r + 5)), -(r + 5), r * 2, r * 2);
    path.addEllipse(-(r - (r + 5)), -(r + 5), r * 2, r * 2);

    QBrush brush(QColor(61, 61, 61));
    QPen pen(QColor(255, 255, 255));
    pen.setWidth(5);
    painter.setPen(pen);
    painter.translate(width() * .5, height() - h * .5);
    painter.fillPath(path, brush);
    painter.drawPath(path);

    painter.setFont(QFont("Times", 14, QFont::Bold));
    // Score du joueur.
    painter.drawText(-(r + (r + 5)), -(r + 5), r * 2, r * 2, Qt::AlignCenter, QString::number(player->score));
    // Score du AI.
    painter.drawText(-(r - (r + 5)), -(r + 5), r * 2, r * 2, Qt::AlignCenter, QString::number(ai->score));
    painter.setPen(QColor(61, 61, 61));
    painter.drawText(-(r + 130), -r, 100, r * 2, Qt::AlignCenter, "PLAYER");
    painter.drawText(-(r - 45), -r, 100, r * 2, Qt::AlignCenter, "AI");
    painter.resetTransform();

    // Afficher le ronda ou tringa lorsqu'il y a l'un d'eux.
    if (onRonda) {
        for (int i = 0; i < winnersRonda.size(); i++) {
            qreal x = onRondaProgress;
            qreal f = pow(x, 1 / (20 * x));
            qreal fc = sin(M_PI * x);

            QVector2D startPoint = winnersRonda[i] == player ? player->pos + QVector2D(0, 100) : ai->pos + QVector2D(0, -100);
            QVector2D target = winnersRonda[i] == player ? player->pos : ai->pos;
            QVector2D currentPos = startPoint + (target - startPoint) * f + QVector2D(150, 10);

            QString bonusName = rondaBonusCount[i] == 2 ? "RONDA" : "TRINGA";
            painter.setPen(QColor(230, 230, 230, int(255 * fc)));
            painter.setFont(QFont("Times", 18, 600));
            painter.drawText(currentPos.x(), currentPos.y(), bonusName + " +" + QString::number(rondaScores[i]));
        }

    }

    // Afficher le FPS.
    painter.setPen(QPen(QColor(255, 255, 255)));
    painter.setFont(QFont("Times", 10, 500));
    painter.drawText(width() - 80, 20, "FPS: " + QString::number(frame_to_show));

    // Afficher le missa lorsqu'il est disponible.
    if (isMissa) {
        painter.setPen(QPen(QColor(255, 255, 255, (int) (255 * missaProgress))));
        painter.setFont(QFont("Times", 18, 800));
        painter.drawText(0, 0, width(), height() - margin.y(), Qt::AlignCenter, "Missa +1");
    }

    // Afficher le darba.
    if (table->onDarbaCollecting) {
        int count = ceil(table->darbaProgress * 3);

        qreal f = abs(sin(M_PI * table->darbaProgress * 3));

        painter.setFont(QFont("Times", ceil(f * 26), 800));
        painter.translate(table->darbaPos.x(), table->darbaPos.y());
        painter.drawText(100 * -.5, 100 * -.5, 100, 100, Qt::AlignCenter, QString::number(count));
    }

    // Animation pour les scores obtenus.
    for (ScoreAnimation *scoreAnim : scoreAnimations) {
        scoreAnim->draw();
    }

    // Afficher le stat du jeu.
    if (stat->isActive) stat->draw();
}

bool collideWithCard(QVector2D pos, Card *card)
{
    if (pos.x() >= card->pos.x() - card->size.x() * .5 && pos.x() <= card->pos.x() + card->size.x() * .5) {
        return (pos.y() >= card->pos.y() - card->size.y() * .5 && pos.y() <= card->pos.y() + card->size.y() * .5);
    }
    return false;
}

void Game::mousePressEvent(QMouseEvent *event)
{
    mouse.clicked = true;
    if (paused) return;
    mouse.last_pos = QVector2D(event->pos().x(), event->pos().y());
    mouse.pos = QVector2D(event->pos().x(), event->pos().y());

    for (int i = player->cards.size() - 1; i >= 0; i--) {
        if (!canPlay) continue;
        if (!player->canPlay) continue;
        Card *card = player->cards[i];
        if (card->freezed) continue;
        if (collideWithCard(mouse.pos, card)) {
            setCursor(Qt::ClosedHandCursor);
            card->initTexture();
            card->setToFirstPlace();
            card->onDrag();
            break;
        }
    }
}

void Game::mouseReleaseEvent(QMouseEvent *event)
{
    mouse.clicked = false;
    for (Card *card : player->cards) {
        if (!card->ondrag) continue;
        card->unleashed_pos = QVector2D(event->pos().x(), event->pos().y());
        card->noDrag();
    }

    bool collided = false;
    for (Card *card : player->cards) {
        if (!canPlay) continue;
        if (!player->canPlay) continue;
        if (card->freezed) continue;
        if (collideWithCard(mouse.pos, card)) {
            collided = true;
            break;
        }
    }

    if (!collided) setCursor(Qt::ArrowCursor);
    else setCursor(Qt::OpenHandCursor);

}

bool Game::event(QEvent *event)
{
    if (event->type() == QEvent::HoverMove) {
        QHoverEvent *mouseEvent = static_cast<QHoverEvent*>(event);

        bool collided = false;
        for (Card *card : player->cards) {
            if (!canPlay) continue;
            if (!player->canPlay) continue;
            if (card->freezed) continue;
            if (collideWithCard(mouse.pos, card)) {
                collided = true;
                break;
            }
        }
        if (collided && !mouse.clicked) setCursor(Qt::OpenHandCursor);
        if (!collided) setCursor(Qt::ArrowCursor);

        mouse.pos = QVector2D(mouseEvent->position().x(), mouseEvent->position().y());
        QVector2D movement = mouse.pos - mouse.last_pos;
        for (int i = player->cards.size() - 1; i >= 0; i--) {
            Card *card = player->cards[i];
            if (card->ondrag) {
                card->pos += movement;
                break;
            }
        }
        mouse.last_pos = QVector2D(mouseEvent->position().x(), mouseEvent->position().y());
    }
    return QMainWindow::event(event);
}

void Game::resizeEvent(QResizeEvent *event)
{
    dealer_pos = QVector2D(width() - 60, (height() - margin.y()) * .5);

    table->pos = QVector2D(width() * .5, (height() - margin.y()) * .5);
    table->initCardOrigins();

    player->pos = QVector2D(width() * .5, height() - 120);
    player->cardsCollectedPos = QVector2D(80, height() - (100 + margin.y()));
    player->initCardOrigins();

    ai->pos = QVector2D(width() * .5, 80);
    ai->cardsCollectedPos = QVector2D(80, 100);
    ai->initCardOrigins();
}

