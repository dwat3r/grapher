#include "edge.h"
#include "node.h"
#include <cmath>


Edge::Edge(Node* from,int id)
  : label("1")
  , from(from)
  , to(NULL)
  , start(from->pos())
  , end(from->pos())
  , id(id)
{
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
  setVisible(true);
}
Edge::Edge(int id,QString label,QPointF start,QPointF end)
  : label(label)
  ,from(NULL)
  ,to(NULL)
  ,start(start)
  ,end(end)
  ,id(id)
{
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
  setVisible(true);
}
QRectF Edge::boundingRect() const
{
  //min(x0, x1), min(y0, y1), abs(x1-x0), abs(y1-y0)
  return QRectF(std::min(start.x(),end.x()),std::min(start.y(),end.y()),
                std::abs(end.x()-start.x()),std::abs(end.y()-start.y()));
}
//from stackoverflow:
//http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
qreal dist2(QPointF v, QPointF w) { return std::pow(v.x() - w.x(),2) + std::pow(v.y() - w.y(),2); }
qreal distToSegment(QPointF p, QPointF v, QPointF w)
{
  qreal t = ((p.x() - v.x()) * (w.x() - v.x()) + (p.y() - v.y()) * (w.y() - v.y())) / dist2(v, w);
  t = std::max(0.0, std::min(1.0, t));
  return std::sqrt(dist2(p, QPointF(v.x() + t * (w.x() - v.x()),v.y() + t * (w.y() - v.y()))));
}

bool Edge::contains(const QPointF &pos) const
{
  return distToSegment(pos,start,end) < 10;
}

void Edge::paint(QPainter *painter,const QStyleOptionGraphicsItem *,QWidget *)
{
  painter->setPen(Qt::black);
  painter->drawLine(start,end);
  //painter->drawText(boundingRect(),Qt::AlignCenter,QString("%1").arg(id));
}
void Edge::removeFromNeighbors()
{
  from->removeNeighbor(to);
  to->removeNeighbor(from);
}
