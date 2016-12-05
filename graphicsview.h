#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include "graphics.h"
#include <QGraphicsView>

class graphicsview : public QGraphicsView
{
public:
  graphicsview(QWidget *parent);
public slots:
  //handle graph value editing
  //right clicks here
  void mousePressEvent(QMouseEvent *event);

  graphics* getScene() const {return scene;}

private:
  graphics *scene;
};

#endif // GRAPHICSVIEW_H
