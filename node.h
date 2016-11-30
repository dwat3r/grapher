#ifndef NODE_H
#define NODE_H

#include <QtGui>
#include <QGraphicsEllipseItem>

#define RADIUS 23
class Node;
class Edge;
typedef std::pair<Node*,Edge*> neighbor;

class Node : public QGraphicsEllipseItem
{
public:
  Node(QPointF pos,int id);
  // for deserialization
  Node(int id, QString label, QPointF pos);
  //destructor notifies neighbors
  // and destructs connected edges too
  ~Node();
  QRectF boundingRect() const;
  bool contains(const QPointF &pos) const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
  void addNeighbor(std::pair<Node*,Edge*> neigh){adlist.push_back(neigh);}

  //getters
  int getId() const {return id;}
  std::vector<neighbor>& getAdlist() {return adlist;}
  QString getLabel() const {return label;}
  void getNodeInfo() const;
  //setters
  void setLabel(QString label) {label = label;}
  // removes neighbor
  void removeNeighbor(Node *node);
private:
  int id;
  std::vector<neighbor> adlist;
  QString label;
};
#endif // NODE_H
