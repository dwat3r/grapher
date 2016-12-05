#include "graphicsview.h"
#include <QtGui>
#include <QInputDialog>
#include <QGraphicsItem>
#include <QDebug>

graphicsview::graphicsview(QWidget *parent) :
  QGraphicsView (parent)
{
  scene = new graphics;
  setScene(scene);
  show();
}
void graphicsview::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::RightButton)
    {
      //ugly shitty hack, fix if you have time
      for(Edge* edge : scene->getEdges())
        {
          if(edge->contains(mapToScene(event->pos())))
            {
              int value = QInputDialog::getInt(this, tr("Value"),
                                               tr("Please enter new value"));

              this->scene->editValueEvent(mapToScene(event->pos()),value);
              return;
            }
        }
    }
  else
    {
      QGraphicsView::mousePressEvent(event);
    }
}
