#include "graphics.h"
//#include <QDebug>
#include <cmath>

//Node
Node::Node(QPointF pos,int id) : QGraphicsEllipseItem()
, id(id)
, adlist()
{
  setPos(pos);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(0);
  setRect(boundingRect());
  setVisible(true);
  label = QString("%1").arg(id);
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
  painter->setPen(Qt::black);
  painter->setBrush(Qt::white);
  painter->drawEllipse(boundingRect());
  painter->drawText(boundingRect(),Qt::AlignCenter,label);

}

//Edge
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

QRectF Edge::boundingRect() const
{
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
  : QGraphicsScene()
  , nodes()
  , edges()
  , selectedEdge(NULL)
  , selectedNode(NULL)
  ,  drawmode(NodeDraw){}

void graphics::setNodeDrawMode(){drawmode = NodeDraw;}
void graphics::setEdgeDrawMode(){drawmode = EdgeDraw;}

void graphics::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{


  if(drawmode == NodeDraw)
    {
      //don't add colliding nodes
      for(Node* node : nodes)
        {
          if(node->contains(event->scenePos()))
            return;
        }
      Node *node = new Node(event->scenePos(),static_cast<int>(nodes.size()) + 1);
      nodes.push_back(node);
      addItem(node);
    }
}

void graphics::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  for(Node* node : nodes)
    {
      if(node->contains(event->scenePos()))
        {
          if(drawmode == NodeDraw)
            {
              //select node to move
              selectedNode = node;
            }
          else
            {
              //start of edge draw

              Edge *edge = new Edge(node,static_cast<int>(edges.size()) + 1);
              edges.push_back(edge);
              addItem(edge);
              selectedEdge = edge;
            }
        }
    }
}

void graphics::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{

  if(drawmode == NodeDraw && selectedNode != NULL)
    {
      // node movement
      selectedNode->setPos(event->scenePos());
      // move connected edges
      for (Edge* edge : edges)
        {
          if(edge->getFrom() == selectedNode)
            edge->setStart(selectedNode->scenePos());
          if(edge->getTo() == selectedNode)
            edge->setEnd(selectedNode->scenePos());
        }
      update();
    }
  else if (selectedEdge != NULL)
    {
      // edge draw
      selectedEdge->setEnd(event->scenePos());
      update();
    }
}

void graphics::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if(drawmode == NodeDraw && selectedNode != NULL)
    {
      // stop moving node
      selectedNode = NULL;
    }
  else if (selectedEdge != NULL)
    {
      // end of edge draw
      for(Node* node : nodes)
        {
          if(node->contains(event->scenePos()))
            {
              //disallow parallel edges
              //if() TODO
              selectedEdge->setTo(node);
              // add neighbors
              node->addNeighbor({selectedEdge->getFrom(),selectedEdge});
              selectedEdge->getFrom()->addNeighbor({node,selectedEdge});
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


QTextStream& operator << (QTextStream &data,graphics &g)
{

  // these contain the mappings between objects and their serialized ids
  data.setFieldWidth(5);
  QMap<Node*,int> nodepmap;
  QMap<Edge*,int> edgepmap;
  /*data looks like this:
  nodesize edgesize
  (first nodes):
  id label x y [neighborNodeId neigborEdgeId]*
  ###
  (then edges):
  id label startx starty endx endy fromId toId
  */
  data << g.nodes.size() << g.edges.size() << '\n';

  for (Node* node : g.nodes)
    {
      data << node->getId()
           << node->getLabel()
           << node->pos().x()
           << node->pos().y();
      for (neighbor n : node->getAdlist())
        {
          Node* first  = std::get<0>(n);
          Edge* second = std::get<1>(n);
          if(nodepmap[first] == 0)
            nodepmap[first] = first->getId();
          if(edgepmap[second] == 0)
            edgepmap[second] = second->getId();

          data << nodepmap[first]
                  << edgepmap[second];
        }
      data << '\n';
    }
  data << "###\n";
  for (Edge* edge : g.edges)
    {
      data << edge->getId()
           << edge->getLabel()
           << edge->getStart().x()
           << edge->getStart().y()
           << edge->getEnd().x()
           << edge->getEnd().y();

      if(nodepmap[edge->getFrom()] == 0)
        nodepmap[edge->getFrom()] = edge->getFrom()->getId();
      if(nodepmap[edge->getTo()] == 0)
        nodepmap[edge->getTo()] = edge->getTo()->getId();

      data << nodepmap[edge->getFrom()]
           << nodepmap[edge->getTo()] << "\n";
    }

  return data;
}

QTextStream& operator >> (QTextStream &data,graphics &g)
{
  //id map for storing the ids of objects
  QMap<int,Node*> nodepmap;
  QMap<int,Edge*> edgepmap;
  //reconstruct objects from serialized objects
  size_t nodesize,edgesize;

  data >> nodesize >> edgesize;

  // first run:
  // ignore pointers since we dont have the objects yet

  for(size_t i = nodesize;i > 0; --i)
    {
      int id;
      QString label;
      qreal x,y;
      data >> id >> label >> x >> y;
      data.readLine();

      Node *node = new Node(id,label,QPointF(x,y));
      g.nodes.push_back(node);
      g.addItem(node);
      nodepmap[id] = node;
    }
  data.readLine();//ignore ###
  for(size_t i = edgesize;i > 0; --i)
    {
      int id;
      QString label;
      qreal x1,y1,x2,y2;
      data >> id >> label >> x1 >> y1 >> x2 >> y2;
      data.readLine();

      Edge *edge = new Edge(id,label,QPointF(x1,y1),QPointF(x2,y2));
      g.edges.push_back(edge);
      g.addItem(edge);
      edgepmap[id] = edge;
    }
  data.seek(0);
  data.readLine();
  // second run:
  // we fill up the pointers
  for(size_t i = nodesize;i > 0; --i)
    {
      int id,n;QString s;
      // skip unneeded first run
      data >> id >> s >> n >> n;
      QStringList sl = data.readLine().split(" ",QString::SkipEmptyParts);
      for(int i =0;i < sl.length(); i += 2)
        {
          nodepmap[id]->addNeighbor({nodepmap[sl[i].toInt()],edgepmap[sl[i+1].toInt()]});
        }
    }
  data.readLine();
  for(size_t i = edgesize;i > 0; --i)
    {
      int id,n;QString s;
      //skip unneeded
      data >> id >> s >> n >> n >> n >> n;
      int fid,tid;
      data >> fid >> tid;
      edgepmap[id]->setFrom(nodepmap[fid]);
      edgepmap[id]->setTo(nodepmap[tid]);
    }
  return data;
}

void graphics::cleanup()
{
  clear();
  nodes.clear();
  edges.clear();
}
