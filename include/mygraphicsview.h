#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyGraphicsView(QWidget *parent = 0);

protected:
    virtual void wheelEvent(QWheelEvent* event);

signals:

public slots:

};

#endif // MYGRAPHICSVIEW_H
