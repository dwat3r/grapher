#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QtEvents>
#include <QPainter>
#include <string>
#include <vector>
#include <utility>

//radius of the nodes
#define RADIUS 15
class Edge;
class Node;
typedef std::pair<Node*,Edge*> neighbor;
class Node : public QGraphicsEllipseItem
{
public:
  Node(QPointF pos,int id)
    : QGraphicsEllipseItem()
    , id(id)
    , adlist()
  {
    setPos(pos);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    setRect(boundingRect());
    setVisible(true);
    label = QString("%1").arg(id);
   }
  QRectF boundingRect() const;
  bool contains(const QPointF &pos) const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
  void addNeighbor(std::pair<Node*,Edge*> neigh){adlist.push_back(neigh);}

  // map function over all neighbors of node
//  void mapNeighbors(std::function<void(neighbor)> lambda)
//  {
//    std::transform(adlist.begin(),adlist.end(),adlist.begin(),lambda);
//  }
private:
  int id;
  std::vector<neighbor> adlist;
  QString label;
};

//this class represents undirected edges.
class Edge : public QGraphicsLineItem
{
public:
  Edge(Node* from)
    : label("1")
    , from(from)
    , to(NULL)
    , start(from->pos())
    , end(from->pos())
  {
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(0);
    setVisible(true);
  }
  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
  //setters
  void setEnd(QPointF pos){end = pos;}
  void setTo(Node* node){end = node->pos();to = node;}
  void setStart(QPointF pos){start = pos;}
  void setFrom(Node* node){start = node->pos();from = node;}
  //getters
  Node* getFrom(){return from;}
  Node* getTo(){return to;}
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
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
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
