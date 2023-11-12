#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>

class Canvas : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);

signals:


    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
};

#endif // CANVAS_H
