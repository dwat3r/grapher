#include "graphics.h"
#include <QDebug>
#include <cmath>

//Node
Node::Node(QPointF pos)
  : QGraphicsEllipseItem()
  , adlist()
  , inCStateRoundCount(0)
{
  setPos(pos);
  label = QString("%1").arg(id);
  id = static_cast <qreal> (rand()) / static_cast <qreal> (RAND_MAX);
  setPen(QPen(Qt::black,3));
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(0);
  setRect(boundingRect());
  setVisible(true);
}
Node::Node(qreal id,QString label,QPointF pos,status state,int inCStateRoundCount)
  : id(id)
  , label(label)
  , state(state)
  , inCStateRoundCount(inCStateRoundCount)
{
  setPos(pos);
  setPen(QPen(Qt::black,3));
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(0);
  setRect(boundingRect());
  setVisible(true);
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
void Node::setColor()
{
  QBrush brush;
  if(state == M)
    brush.setColor(Qt::green);
  if(state == nM)
    brush.setColor(Qt::black);
  if(state == C)
    brush.setColor(Qt::yellow);
  if(state == R)
    brush.setColor(Qt::red);
  setPen(QPen(brush,3));
  update();
}
void Node::setState(Node *neigh)
{
  if(state == M && neigh->getId() < id && neigh->getState() == C)
    {
      state = C;
      inCStateRoundCount = 1;
    }
  else if(state == nM && neigh->getId() < id && neigh->getState() == C)
    {
      bool cond = true;
      for (neighbor n : I_pi())
        {
          Node *node = std::get<0>(n);
          if(node != neigh && node->getState() == M)
            cond = false;
        }
      if(cond)
        state = C;
      inCStateRoundCount = 1;
    }
  else if (state == C)
    {
      bool cond = true;
      for (neighbor n : nI_pi())
        {
          Node *node = std::get<0>(n);
          if(node->getState() == C)
            cond = false;
        }
      if(cond && inCStateRoundCount < 2)
        {
          state = R;
        }
      else
        inCStateRoundCount++;
    }
  else if (state == R)
    {
      bool cond = true;
      for (neighbor n : I_pi())
        {
          Node *node = std::get<0>(n);
          if(node->getState() != nM &&
             node->getState() != M)
            cond = false;
        }
      if(cond)
        {
          bool cond2 = true;
          for(neighbor n : I_pi())
            {
              if(std::get<0>(n)->getState() != nM)
                cond2 = false;
            }
          if(cond2)
            state = M;
          else
            {
              cond2 = true;
              for(neighbor n : I_pi())
                {
                  if(std::get<0>(n)->getState() != M)
                    cond2 = false;
                }
              if(cond2)
                state = nM;
            }
        }
    }
  setColor();
}
void Node::advertiseState()
{
  std::for_each(adlist.begin(),adlist.end(),
                [this](neighbor n){std::get<0>(n)->setState(this);});
}

bool Node::checkMIS()
{
  if(state == M)
    {
      for(neighbor n : I_pi())
        {
          if(std::get<0>(n)->getState() != nM)
            return false;
        }
      return true;
    }
  else if(state == nM)
    {
      for(neighbor n : I_pi())
        {
          if(!std::get<0>(n)->checkMIS())
            return false;
        }
      return true;
    }
  return false;
}
std::vector<neighbor> Node::I_pi()
{
  std::vector<neighbor> pi(adlist);
  std::remove_if(pi.begin(),pi.end(),
                 [this](neighbor n){return std::get<0>(n)->getId() > id;});
  return pi;
}
std::vector<neighbor> Node::nI_pi()
{
  std::vector<neighbor> pi(adlist);
  std::remove_if(pi.begin(),pi.end(),
                 [this](neighbor n){return std::get<0>(n)->getId() < id;});
  return pi;
}
void Node::removeNeighbor(Node *node)
{
  for (auto n = adlist.begin();n!=adlist.end();++n)
    {
      if(std::get<0>(*n) == node)
        {
          adlist.erase(n);
          return;
        }
    }
}
Node::~Node()
{
  for(neighbor n : adlist)
      std::get<0>(n)->removeNeighbor(this);
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
  //painter->drawText(boundingRect(),Qt::AlignCenter,label);
}
Edge::~Edge()
{
  from->removeNeighbor(to);
  to->removeNeighbor(from);
}

//graphics
graphics::graphics()
  : QGraphicsScene()
  , nodes()
  , edges()
  , selectedEdge(NULL)
  , selectedNode(NULL)
  , drawmode(NodeDraw)
{
  srand (static_cast <unsigned> (time(0)));
}

void graphics::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{


  if(drawmode == NodeDraw)
    {
      //if we doubleclicked on an item, delete it
      bool deleted = false;
      for(Node* node : nodes)
        {
          if(node->contains(event->scenePos()))
            {
              removeNode(node);
              deleted = true;
            }
        }
      if(!deleted)
        {
          Node *node = new Node(event->scenePos());
          nodes.push_back(node);
          addItem(node);
        }
    }
  else
    {
      for(Edge* edge : edges)
        {
          if(edge->contains(event->scenePos()))
            {
              removeEdge(edge);
              return;
            }
        }
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
              bool cond = false;
              for (neighbor n : node->getAdlist())
                {
                  if(std::get<1>(n)->getFrom() == selectedEdge->getFrom() &&
                     std::get<1>(n)->getTo() == node)
                    {
                      cond = true;
                      break;
                    }
                }
              if(!cond)
                {
                  selectedEdge->setTo(node);
                  // add neighbors
                  node->addNeighbor({selectedEdge->getFrom(),selectedEdge});
                  selectedEdge->getFrom()->addNeighbor({node,selectedEdge});
                  update();
                  selectedEdge = NULL;
                }
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
  QMap<Node*,qreal> nodepmap;
  QMap<Edge*,qreal> edgepmap;
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
           << node->getState()
           << node->getInCStateRoundCount()
           << node->pos().x()
           << node->pos().y();
      for (neighbor n : node->getAdlist())
        {
          Node* first  = std::get<0>(n);
          Edge* second = std::get<1>(n);
          if(!(nodepmap[first] > 0))
            nodepmap[first] = first->getId();
          if(!(edgepmap[second] > 0))
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

      if(!(nodepmap[edge->getFrom()] > 0))
        nodepmap[edge->getFrom()] = edge->getFrom()->getId();
      if(!(nodepmap[edge->getTo()] > 0))
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
      int id,inCStateRoundCount,state;
      QString label;
      qreal x,y;
      data >> id >> label >> x >> y >> state >> inCStateRoundCount;
      data.readLine();

      Node *node = new Node(id,label,QPointF(x,y),static_cast<status>(state),inCStateRoundCount);
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

void graphics::removeNode(Node *node)
{
  for(auto i = nodes.begin();i!= nodes.end();++i)
    {
      if(*i == node)
        {
          removeItem(*i);
          for (neighbor n : (*i)->getAdlist())
            {
              removeEdge(std::get<1>(n));
            }
          nodes.erase(i);
          node->~Node();
          return;
        }
    }
}
void graphics::removeEdge(Edge *edge)
{
  for(auto i = edges.begin();i!= edges.end();++i)
    {
      if(*i == edge)
        {
          removeItem(*i);
          edges.erase(i);
          edge->~Edge();
          return;
        }
    }
}
void graphics::cleanup()
{
  clear();
  nodes.clear();
  edges.clear();
}
