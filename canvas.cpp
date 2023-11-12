#include "canvas.h"

#include <QPainter>

Canvas::Canvas(QWidget *parent)
    : QWidget{parent}{
    parent->set;
}

void Canvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.drawRect(0, 0, 100, 100);
}
