#ifndef EDGE_H
#define EDGE_H

#include "node.h"
#include <QtGui>
#include <QGraphicsLineItem>

//this class represents undirected edges.
class Edge : public QGraphicsLineItem
{
public:
  Edge(Node* from,int id);
  Edge(int id,int weight,QString label,QPointF start,QPointF end);

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
  int getWeight() const {return weight;}
  //setters
  void setEnd(QPointF pos){end = pos;}
  void setStart(QPointF pos){start = pos;}
  void setTo(Node* node){end = node->pos();to = node;}
  void setFrom(Node* node){start = node->pos();from = node;}
  void setWeight(int nw){weight = nw;}
  //remove
  void removeFromNeighbors();
private:
  QString label;
  Node* from;
  Node* to;
  QPointF start;
  QPointF end;
  int id;
  //weight
  int weight;
};
#endif // EDGE_H
