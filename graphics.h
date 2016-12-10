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
#include <functional>
#include <QMap>
#include <QTextStream>
#include <random>

//radius of the nodes
#define RADIUS 23
class Edge;
class Node;
typedef std::pair<Node*,Edge*> neighbor;
typedef enum {nM,C,R,M} status;

class Node : public QGraphicsEllipseItem
{
public:
  Node(QPointF pos);
  // for deserialization
  Node(qreal id,QString label,QPointF pos,status state,int inCStateRoundCount);
  //destructor notifies neighbors
  // and destructs connected edges too
  ~Node();
  QRectF boundingRect() const;
  bool contains(const QPointF &pos) const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
  void addNeighbor(std::pair<Node*,Edge*> neigh){adlist.push_back(neigh);}

  //getters
  qreal getId() const {return id;}
  std::vector<neighbor>& getAdlist() {return adlist;}
  QString getLabel() const {return label;}
  status getState() const {return state;}
  int getInCStateRoundCount() const {return inCStateRoundCount;}
  void getNodeInfo() const;
  //setters
  void setLabel(QString label) {label = label;}
  // set color of node according to state
  void updateColors();
  //message passing between nodes
  // updateState implements Algo 2
  void updateState(/*Node* neigh*/);
  // advertiseState calls neighbor node's setState
  void advertiseState();
  // check mis invariant
  bool checkMIS();
  // the I_pi(v) function
  std::vector<neighbor> I_pi();
  std::vector<neighbor> nI_pi();
  // removes neighbor
  void removeNeighbor(Node *node);
  // trigger this function on graph change (Edge change -> trigger both nodes?)
private:
  qreal id;
  std::vector<neighbor> adlist;
  QString label;
  status state;
  int inCStateRoundCount;
};

//this class represents undirected edges.
class Edge : public QGraphicsLineItem
{
public:
  Edge(Node* from,int id);
  Edge(int id,QString label,QPointF start,QPointF end);
  // destructor calls notify
  void removeFromNeighbors();
  QRectF boundingRect() const;
  bool contains(const QPointF &pos) const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
  //getters
  Node* getFrom() const {return from;}
  Node* getTo() const {return to;}
  QString getLabel() const {return label;}
  QPointF getStart() const {return start;}
  QPointF getEnd() const {return end;}
  int getId() const {return id;}
  //setters
  void setEnd(QPointF pos){end = pos;}
  void setStart(QPointF pos){start = pos;}
  void setTo(Node* node){end = node->pos();to = node;}
  void setFrom(Node* node){start = node->pos();from = node;}
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

  // Graph triggered changes
  void graphModificationListener(Node* changedNode) const;
  void massCheckMIS(const std::vector<neighbor>& toNotify);

  //TODO: rewrite to QTextStream
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
  int edgeId;
};


#endif // GRAPHICS_H
