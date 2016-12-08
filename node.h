#ifndef NODE_H
#define NODE_H

#include <QtGui>
#include <QGraphicsEllipseItem>
#include <set>

#define RADIUS 15
class Node;
class Edge;
typedef std::pair<Node*,Edge*> neighbor;
typedef enum {V1,V2,Neither} bipartition;

class Node : public QGraphicsEllipseItem
{
public:
  Node(QPointF pos,int id,bipartition bi);
  // for deserialization
  Node(int id, bipartition bi,bool inM,QString label, QPointF pos);
  //destructor notifies neighbors
  // and destructs connected edges too

  QRectF boundingRect() const;
  bool contains(const QPointF &pos) const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
  void addNeighbor(std::pair<Node*,Edge*> neigh){adlist.insert(neigh);}

  //getters
  int getId() const {return id;}
  std::set<neighbor>& getNeighbors() {return adlist;}
  QString getLabel() const {return label;}
  bipartition getBi() const {return bi;}
  bool isInM() const {return inM;}
  //debug
  void getNodeInfo() const;
  //setters
  void setLabel(QString label) {label = label;}
  void setInM(bool b) {inM = b;}
  // removes neighbor
  void removeNeighbor(Node *node);
  void removeFromNeighbors();
private:
  int id;
  std::set<neighbor> adlist;
  QString label;
  //which set is in it
  bipartition bi;
  // is it in M
  bool inM;
};
#endif // NODE_H
