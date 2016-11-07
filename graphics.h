#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QtEvents>
#include <QPainter>
#include <vector>
#include <utility>
#include <QMap>
#include <QTextStream>

//radius of the nodes
#define RADIUS 15
class Edge;
class Node;
typedef std::pair<Node*,Edge*> neighbor;

class Node : public QGraphicsEllipseItem
{
public:
  Node(QPointF pos,int id);
  Node(int id,QString label,QPointF pos)
    : id(id),adlist(),label(label){setPos(pos);}

  QRectF boundingRect() const;
  bool contains(const QPointF &pos) const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
  void addNeighbor(std::pair<Node*,Edge*> neigh){adlist.push_back(neigh);}

  //getters
  int getId() const {return id;}
  std::vector<neighbor>& getAdlist() {return adlist;}
  QString getLabel() const {return label;}
  //setters
  void setLabel(QString label) {label = label;}
  void mapNeighbor(std::function<void(neighbor)> lambda)
  {std::for_each(adlist.begin(),adlist.end(),lambda);}
private:
  int id;
  std::vector<neighbor> adlist;
  QString label;
};

//this class represents undirected edges.
class Edge : public QGraphicsLineItem
{
public:
  Edge(Node* from,int id);
  Edge(int id,QString label,QPointF start,QPointF end)
    : label(label),from(NULL),to(NULL),start(start),end(end),id(id){}

  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
  //setters
  void setEnd(QPointF pos){end = pos;}
  void setStart(QPointF pos){start = pos;}
  void setTo(Node* node){end = node->pos();to = node;}
  void setFrom(Node* node){start = node->pos();from = node;}
  //getters
  Node* getFrom() const {return from;}
  Node* getTo() const {return to;}
  QString getLabel() const {return label;}
  QPointF getStart() const {return start;}
  QPointF getEnd() const {return end;}
  int getId() const {return id;}
private:
  QString label;
  Node* from;
  Node* to;
  QPointF start;
  QPointF end;
  int id;
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

  //TODO: rewrite to QTextStream
  //(de)serialization
  friend QTextStream& operator << (QTextStream& data,graphics &g);
  friend QTextStream& operator >> (QTextStream& data,graphics &g);

  //reset scene
  void cleanup();

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
