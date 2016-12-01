#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "node.h"
#include "edge.h"
#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>


//we handle events here.
class graphics : public QGraphicsScene
{
  Q_OBJECT
public:
  graphics();
  //select actions based on the buttons' state
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

  //(de)serialization
  friend QTextStream& operator << (QTextStream& data,graphics &g);
  friend QTextStream& operator >> (QTextStream& data,graphics &g);

  //reset scene
  void cleanup();
  //deletions
  void removeNode(Node *node);
  void removeEdge(Edge *edge);
public slots:
  void setNodeDrawMode(){drawmode = NodeDraw;}
  void setEdgeDrawMode(){drawmode = EdgeDraw;}
private:
  std::vector<Node*> nodes;
  std::vector<Edge*> edges;
  Edge *selectedEdge;
  Node *selectedNode;
  enum drawmode{NodeDraw,EdgeDraw} drawmode;
  int nodeId;
  int edgeId;
};


#endif // GRAPHICS_H
