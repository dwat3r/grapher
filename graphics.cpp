#include "graphics.h"
#include <QDebug>
#include <cmath>
//Node
QRectF Node::boundingRect() const
{
  return QRectF(pos.x() - RADIUS,pos.y() - RADIUS,
                2 * RADIUS,2 * RADIUS);
}

void Node::paint(QPainter *painter,const QStyleOptionGraphicsItem *,QWidget *)
{
  painter->setPen(Qt::black);
  painter->setBrush(Qt::white);
  painter->drawEllipse(boundingRect());
  painter->drawText(boundingRect(),Qt::AlignCenter,label);

}

QRectF Edge::boundingRect() const
{
  //TODO: set center of line
  //now returns shit
  //min(x0, x1), min(y0, y1), abs(x1-x0), abs(y1-y0)
  return QRectF(std::min(start.x(),end.x()),std::min(start.y(),end.y()),
                std::abs(end.x()-start.x()),std::abs(end.y()-start.y()));
}

void Edge::paint(QPainter *painter,const QStyleOptionGraphicsItem *,QWidget *)
{
  painter->setPen(Qt::black);
  painter->drawLine(start,end);
  painter->drawText(boundingRect(),Qt::AlignCenter,label);
}

//graphics
graphics::graphics()
  : drawmode(NodeDraw)
  , nodes()
  , edges()
  , selectedEdge(NULL)
  , selectedNode(NULL)
  , QGraphicsScene(){}

void graphics::setNodeDrawMode(){drawmode = NodeDraw;}
void graphics::setEdgeDrawMode(){drawmode = EdgeDraw;}

void graphics::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{

  //qDebug() << event->scenePos();
  if(drawmode == NodeDraw)
    {
      //don't add colliding nodes
      for(Node* node : nodes)
        {
          if(node->contains(event->scenePos()))
            return;
        }
      Node *node = new Node(event->scenePos(),nodes.size() + 1);
      nodes.push_back(node);
      addItem(node);
    }
}
void graphics::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

  if(drawmode == NodeDraw && selectedNode != NULL)
    {
      // node movement
    }
  else if (selectedEdge != NULL)
    {
      // edge draw
      selectedEdge->setEnd(event->scenePos());
      update();
    }
}

void graphics::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if(drawmode == NodeDraw)
    {
      //select node to move
    }
  else
    {
      //start of edge draw
      for(Node* node : nodes)
        {
          if(node->contains(event->scenePos()))
            {
              Edge *edge = new Edge(node);
              edges.push_back(edge);
              addItem(edge);
              selectedEdge = edge;
            }
        }
    }
}

void graphics::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if(drawmode == NodeDraw)
    {
      // stop moving node
    }
  else
    {
      // end of edge draw
      for(Node* node : nodes)
        {
          if(node->contains(event->scenePos()))
            {
              selectedEdge->setTo(node);
              update();
              selectedEdge = NULL;
            }
        }
      if(selectedEdge != NULL)
        {
          removeItem(selectedEdge);
          selectedEdge = NULL;
          edges.pop_back();
        }
    }
}
