#include "node.h"
#include <QDebug>
#include <ctime>
#include <functional>

Node::Node(QPointF pos,int id,bipartition bi)
  : QGraphicsEllipseItem()
  , id(id)
  , adlist()
  , bi(bi)
  , inM()
{
  setPos(pos);
  label = QString::number(id);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(0);
  setRect(boundingRect());
  setVisible(true);
}
Node::Node(int id,bipartition bi,bool inM,QString label,QPointF pos)
  : id(id)
  , label(label)
  , bi(bi)
  , inM(inM)
{
  setPos(pos);
  setPen(pen());
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(0);
  setRect(boundingRect());
  setVisible(true);
}

void Node::getNodeInfo() const
{
  qDebug() << "Node -> id=" << id << ", bi=" << bi;
}

QRectF Node::boundingRect() const
{
  return QRectF(-RADIUS,-RADIUS,
                2 * RADIUS,2 * RADIUS);
}
bool Node::contains(const QPointF &p) const
{
  //  return (centerx-10<=mx) && (centerx+10>= mx) &&
  //(centery-10<=my) && (centery+10>= my);
  return (x() - RADIUS <= p.x()) && (x() + RADIUS >= p.x()) &&
      (y() - RADIUS <= p.y()) && (y() + RADIUS >= p.y());
}
void Node::paint(QPainter *painter,const QStyleOptionGraphicsItem *,QWidget *)
{
  if(bi == V1)
    painter->setPen(Qt::red);
  else
    painter->setPen(Qt::blue);

  painter->setBrush(Qt::white);
  painter->drawEllipse(boundingRect());
  painter->setPen(Qt::black);
  painter->drawText(boundingRect(),Qt::AlignCenter,label);

}

void Node::removeNeighbor(Node *node)
{
  for (auto n = adlist.begin();n != adlist.end();)
    {
      if(n->first == node)
        {
          n = adlist.erase(n);
          return;
        }
      else
        ++n;
    }
}
void Node::removeFromNeighbors()
{
  for(neighbor n : adlist)
    n.first->removeNeighbor(this);
}
