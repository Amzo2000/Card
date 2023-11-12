#ifndef GAME_H
#define GAME_H

#include <QMainWindow>
#include "mouse.h"
#include "card.h"
#include "player.h"
#include "table.h"
#include <QImage>
#include "scoreAnimation.h"
#include "stat.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Game; }
QT_END_NAMESPACE

class Game : public QMainWindow
{
    Q_OBJECT

public:
    Game(QWidget *parent = nullptr);
    ~Game();

    const static int TABLE{0};
    const static int PLAYER{1};
    const static int AI{2};

    const qreal FPS{120.0};

    qreal frame_to_show{FPS};

    Mouse mouse;

    QList<Card*> cards;

    void background(QColor color);

    Stat *stat;

    Table *table;
    Player *player;
    Player *ai;

    int winner;

    QVector2D dealer_pos;

    QList<Animation*> animations;
    QList<ScoreAnimation*> scoreAnimations;

    bool onRonda{false};
    QList<int> rondaBonusCount;
    QList<int> rondaScores;
    QList<Player*> winnersRonda;
    qreal onRondaProgress{0};

    QVector2D margin;

    bool canPlay{false};
    bool firstToPlay{true};
    int remainCards{0};

    qreal time{0};

    bool paused{false};

    bool isCardsConfigured{false};
    bool isGameFinish{false};
    bool isReloadedCards{false};
    bool finished{false};

    bool isMissa{false};
    qreal missaProgress{0};

    QImage light;

    qint64 now;
    qint64 lastTime;
    int frame{0};

private:
    Ui::Game *ui;

    // QWidget interface
public:
    void reposition();
    void paintEvent(QPaintEvent *event);
    void initCards();
    bool distributionCards(int mode, QList<Card*> &parent_cards, int cardLength, bool show);
    void rondaBonus();

    void restart();

    bool event(QEvent *event);
    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
};
#endif // GAME_H
