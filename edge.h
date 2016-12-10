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
  Edge(int id,int weight,Node* from,Node* to);
  void init();
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
  bool isDirected() const {return directed;}
  bool isInM() const {return inM;}
  //setters
  void setEnd(QPointF pos){end = pos;}
  void setStart(QPointF pos){start = pos;}
  void setTo(Node* node){end = node->pos();to = node;}
  void setFrom(Node* node){start = node->pos();from = node;}
  void setLabel(QString l){label = l;}
  void setWeight(int nw){weight = nw;}
  void setDirected(bool b){directed = b;update();}
  void setInM(bool b){inM = b;}
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
  //directedness control if we should
  //draw tiny arrows on the edges pointy end
  //or not
  bool directed;
  //inM
  bool inM;
};
#endif // EDGE_H
