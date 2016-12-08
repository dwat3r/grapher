#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "node.h"
#include "edge.h"
#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>


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
  //
  void editValueEvent(QPointF pos,int value);
  std::vector<Edge*> getEdges() const {return edges;}
  //(de)serialization
  friend QTextStream& operator << (QTextStream& data,graphics &g);
  friend QTextStream& operator >> (QTextStream& data,graphics &g);
  //matching related functions
  void matching();
  std::map<Node*,std::pair<int,Node*> > dijkstra(Node *source,Node *dest);
  void drawST(Node* s,Node* t);
  void directEdges();
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
  //TODO: prevent the doubleclick + other events messing
  QTimer timer;
  bool doubleClicked;
protected slots:
  void timeout(){}
};


#endif // GRAPHICS_H
