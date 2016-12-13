#include "edge.h"
#include <cmath>

const qreal Pi = 3.14;

Edge::Edge(Node* from,int id)
  : label("1")
  , from(from)
  , to(nullptr)
  , start(from->pos())
  , end(from->pos())
  , id(id)
  , weight(1)
  , directed(0)
  , inM(false)
{
  init();
}
Edge::Edge(int id,int weight,QString label,QPointF start,QPointF end)
  : label(label)
  ,from(nullptr)
  ,to(nullptr)
  ,start(start)
  ,end(end)
  ,id(id)
  ,weight(weight)
  ,directed(0)
  ,inM(false)
{
  init();
}
Edge::Edge(int id,int weight,Node* from,Node* to)
  : from(from)
  , to(to)
  , start(from->pos())
  , end(to->pos())
  , id(id)
  , weight(weight)
  , directed(1)
{
  init();
}
void Edge::init()
{
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
  setVisible(true);
}

QRectF Edge::boundingRect() const
{
  //min(x0, x1), min(y0, y1), abs(x1-x0), abs(y1-y0)
  qreal extra = 10;
  return QRectF(std::min(start.x(),end.x()),std::min(start.y(),end.y()),
                std::abs(end.x()-start.x()),std::abs(end.y()-start.y()))
      .normalized()
      .adjusted(-extra,-extra,extra,extra);
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
  if (inM)
    painter->setPen(Qt::red);
  else
    painter->setPen(Qt::black);

  painter->drawLine(start,end);
  if (directed)
    {
      QLineF _line(start,end);
      //adjust
      QPointF edgeOffset((_line.dx() * RADIUS) / _line.length(),
                         (_line.dy() * RADIUS) / _line.length());
      _line.setP1(_line.p1() + edgeOffset);
      _line.setP2(_line.p2() - edgeOffset);

      setLine(_line);
      qreal arrowSize = 10;
      double angle = ::acos(line().dx() / (line().length()));
      if (line().dy() >= 0)
          angle = (Pi * 2) - angle;

      QPointF arrowP1 = line().p1() + QPointF(sin(angle + Pi / 3) * arrowSize,
                                      cos(angle + Pi / 3) * arrowSize);
      QPointF arrowP2 = line().p1() + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                      cos(angle + Pi - Pi / 3) * arrowSize);
      QPolygonF arrowHead;
      arrowHead << line().p1() << arrowP1 << arrowP2;
      painter->drawPolygon(arrowHead);
    }
  painter->drawText(boundingRect(),Qt::AlignCenter,QString("%1").arg(weight));
}
void Edge::removeFromNeighbors()
{
  from->removeNeighbor(to);
  to->removeNeighbor(from);
}
