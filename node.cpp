#include "node.h"
#include <ctime>
#include <QDebug>

Node::Node(QPointF pos,int id)
  : QGraphicsEllipseItem()
  , adlist()
  , id(id)
{
  setPos(pos);
  label = QString::number(id);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(0);
  setRect(boundingRect());
  setVisible(true);
}
Node::Node(int id,QString label,QPointF pos)
  : id(id)
  , label(label)
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
  qDebug() << "Node -> id=" << id << ", neighbors=" << adlist;
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
  painter->setPen(pen());
  painter->setBrush(Qt::white);
  painter->drawEllipse(boundingRect());
  painter->setPen(Qt::black);
  painter->drawText(boundingRect(),Qt::AlignCenter,label);

}

void Node::removeNeighbor(Node *node)
{
  for (auto n = adlist.begin();n != adlist.end();)
    {
      if(std::get<0>(*n) == node)
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
    std::get<0>(n)->removeNeighbor(this);
}
