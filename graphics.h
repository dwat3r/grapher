#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QtEvents>
#include <QPainter>
#include <string>
#include <vector>
#include <utility>

//radius of the nodes
#define RADIUS 15
class Edge;
class Node : public QGraphicsEllipseItem
{
public:
  Node(QPointF pos,int id)
    : adlist()
    , id(id)
    , pos(pos)
  {
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    setRect(boundingRect());
    setVisible(true);
    label = QString("%1").arg(id);
  }
  QRectF boundingRect() const Q_DECL_OVERRIDE;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;
  QPointF getPos() const {return pos;}
private:
  int id;
  QPointF pos;
  std::vector<std::pair<Node*,Edge*> > adlist;
  QString label;
};

class Edge : public QGraphicsLineItem
{
public:
  Edge(Node* from)
    : label("1")
    , from(from)
    , to(NULL)
    , start(from->getPos())
    , end(from->getPos())
  {
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setVisible(true);
  }
  void setEnd(QPointF pos){end = pos;}
  void setTo(Node* node){end = node->getPos();to = node;}
  QRectF boundingRect() const Q_DECL_OVERRIDE;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;
private:
  QString label;
  Node* from;
  Node* to;
  QPointF start;
  QPointF end;
};
//we handle events here.
class graphics : public QGraphicsScene
{
  Q_OBJECT
public:
  graphics();
  //select actions based on the buttons' state
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
  //
public slots:
  void setNodeDrawMode();
  void setEdgeDrawMode();
private:
  std::vector<Node*> nodes;
  std::vector<Edge*> edges;
  Edge *selectedEdge;
  Node *selectedNode;
  enum drawmode{NodeDraw,EdgeDraw} drawmode;
};

#endif // GRAPHICS_H
