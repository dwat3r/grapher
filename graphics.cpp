#include "graphics.h"
#include <QDebug>
#include <cmath>
#include <QTime>
#include <QCoreApplication>
#include <QEventLoop>

//Node
Node::Node(QPointF pos)
  : QGraphicsEllipseItem()
  , adlist()
  , state(nM)
  , inCStateRoundCount(0)
{
  setPos(pos);
  id = static_cast <qreal> (rand()) / static_cast <qreal> (RAND_MAX);
  label = QString::number(id, 'd', 3);
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

void Node::getNodeInfo() const
{
  qDebug() << "Node -> id=" << this->id << ", neighbors=" << this->adlist.size() << ", state=" << this->state << " rounds=" << this->inCStateRoundCount+1;
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
void Node::updateColors()
{
  QColor color;
  if(state == M)
    color = Qt::green;
  if(state == nM)
    color = Qt::black;
  if(state == C)
    color = Qt::yellow;
  if(state == R)
    color = Qt::red;
  setPen(QPen(color,3));
  update();
}
// Recurse on (every?) state change -> advertise
void Node::updateState(/*Node *neigh*/)
{
  qDebug() << "Entering updateState at...";
  getNodeInfo();

  QTime dieTime= QTime::currentTime().addMSecs(300);
  while (QTime::currentTime() < dieTime)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  if(state == M)
    {
      for (neighbor n : I_pi())
        {
          Node *node = std::get<0>(n);
          if(node->getState() == C) {
              state = C;
              inCStateRoundCount++;
              updateColors();
              return this->advertiseState();
            }
        }
    }
  else if(state == nM)
    {
      bool potentialMisEntrant = true;
      for (neighbor n : I_pi())
        {
          Node *node = std::get<0>(n);
          if(!(node->getState() == C || node->getState() != M)) {
              potentialMisEntrant = false;
              break;
            }
        }
      if(potentialMisEntrant) {
          state = C;
          inCStateRoundCount++;
          updateColors();
          return this->advertiseState();
        }
    }
  else if (state == C)
    {
      bool isBiggestC = true;
      for (neighbor n : nI_pi())
        {
          Node *node = std::get<0>(n);
          if(node->getState() == C)
            isBiggestC = false;
        }
      if(isBiggestC && inCStateRoundCount > 1)
        {
          state = R;
          updateColors();
        }
      else
        {
          inCStateRoundCount++;
        }
      return this->advertiseState();
    }
  else if (state == R)
    {
      if( I_pi().size() > 0 ) {
          for (neighbor n : I_pi())
            {
              Node *node = std::get<0>(n);
              if(node->getState() != nM &&
                 node->getState() != M)
                return ;
            }
        }

      bool misCandidate = true;
      for(neighbor n : I_pi())
        {
          if(std::get<0>(n)->getState() == M)
            {
              misCandidate = false;
              break;
            }
        }
      if(misCandidate)
        {
          state = M;
        }
      else
        {
          state = nM;
        }
      inCStateRoundCount = 0;
      updateColors();
      //return this->advertiseState();
    }
}
void Node::advertiseState()
{
  std::for_each(adlist.begin(),adlist.end(),
                [this](neighbor n){std::get<0>(n)->updateState();});
  this->updateState();
}
// Call on stable graph only => Nodes are either in state M or nM
bool Node::checkMIS()
{
  if(state == M)
    {
      for(neighbor n : I_pi())
        {
          if(std::get<0>(n)->getState() != nM)
            {
              state = C;
              updateColors();
              return false;
            }
        }
      return true;
    }
  else if(state == nM)
    {
      bool misInvariantHolds = false;
      for(neighbor n : I_pi())
        {
          //if(!std::get<0>(n)->checkMIS())
          if(std::get<0>(n)->getState() == M ) {
              misInvariantHolds = true;
              break;
            }
        }
      if (!misInvariantHolds)
        {
          state = C;
          updateColors();
          return false;
        }
      return true;
    }
  return false; // This code must be unreachable
}

std::vector<neighbor> Node::I_pi()
{
  std::vector<neighbor> pi(adlist);
  std::vector<neighbor>::iterator it = std::remove_if(pi.begin(),pi.end(), [this](neighbor n){return std::get<0>(n)->getId() > id;});
  pi.erase( it, pi.end() );

  return pi;
}
std::vector<neighbor> Node::nI_pi()
{
  std::vector<neighbor> pi(adlist);
  std::vector<neighbor>::iterator it = std::remove_if(pi.begin(),pi.end(), [this](neighbor n){return std::get<0>(n)->getId() < id;});
  pi.erase( it, pi.end() );

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
  //painter->drawText(boundingRect(),Qt::AlignCenter,QString("%1").arg(id));
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
  , edgeId(0)
{
  srand (static_cast <unsigned> (time(0)));
}

void graphics::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{


  if(drawmode == NodeDraw)
    {
      //if we doubleclicked on an item, delete it
      for(Node* node : nodes)
        {
          if(node->contains(event->scenePos()))
            {
              //graphModificationListener(node);
              std::vector<neighbor> notifiableNeighbors = node->getAdlist();
              removeNode(node);
              massCheckMIS(notifiableNeighbors);
              return;
            }
        }
      Node *node = new Node(event->scenePos());
      nodes.push_back(node);
      addItem(node);
      graphModificationListener(node);
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

              Edge *edge = new Edge(node,edgeId++);
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
              //disallow parallel and loop edges
              bool cond = false;
              //loop
              if (selectedEdge->getFrom() == node)
                cond = true;
              //parallel
              else
                {
                  for (neighbor n : node->getAdlist())
                    {
                      if((std::get<1>(n)->getFrom() == selectedEdge->getFrom() &&
                          std::get<1>(n)->getTo() == node)||
                         (std::get<1>(n)->getFrom() == node &&
                          std::get<1>(n)->getTo() == selectedEdge->getFrom()))
                        {
                          cond = true;
                          break;
                        }
                    }
                }
              if(!cond)
                {
                  selectedEdge->setTo(node);
                  // add neighbors
                  node->addNeighbor({selectedEdge->getFrom(),selectedEdge});
                  selectedEdge->getFrom()->addNeighbor({node,selectedEdge});
                  update();
                  graphModificationListener(selectedEdge->getFrom());
                  graphModificationListener(selectedEdge->getTo());
                  selectedEdge = NULL;
                }
              break;
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

void graphics::graphModificationListener(Node* changedNode) const
{
  qDebug() << "-----------------------------------------------";
  if (changedNode->checkMIS() == false) {
      qDebug() << "MIS invariant violated -> advertising state";
      changedNode->advertiseState();
    } else {
      qDebug() << "MIS invariant holds";
    }
}

void graphics::massCheckMIS(const std::vector<neighbor>& toNotify) {
  for (auto n = toNotify.begin();n!=toNotify.end();++n)
    {
      //std::get<0>(*n)->checkMIS();
      graphModificationListener(std::get<0>(*n));
    }
}

QTextStream& operator << (QTextStream &data,graphics &g)
{

  // these contain the mappings between objects and their serialized ids
  data.setFieldWidth(10);
  QMap<Node*,qreal> nodepmap;
  QMap<Edge*,int> edgepmap;
  /*data looks like this:
  nodesize edgesize
  (first nodes):
  id label state incround.. x y [neighborNodeId neigborEdgeId]*
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
      qDebug() << node->getId();
      for (neighbor n : node->getAdlist())
        {
          Node* first  = std::get<0>(n);
          Edge* second = std::get<1>(n);
          if(!(nodepmap.contains(first)))
            nodepmap[first] = first->getId();
          if(!(edgepmap.contains(second)))
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

      if(!(nodepmap.contains(edge->getFrom())))
        nodepmap[edge->getFrom()] = edge->getFrom()->getId();
      if(!(nodepmap.contains(edge->getTo())))
        nodepmap[edge->getTo()] = edge->getTo()->getId();

      data << nodepmap[edge->getFrom()]
          << nodepmap[edge->getTo()] << "\n";
    }

  return data;
}

QTextStream& operator >> (QTextStream &data,graphics &g)
{
  //id map for storing the ids of objects
  QMap<qreal,Node*> nodepmap;
  QMap<int,Edge*> edgepmap;
  //reconstruct objects from serialized objects
  size_t nodesize,edgesize;

  data >> nodesize >> edgesize;

  // first run:
  // ignore pointers since we dont have the objects yet

  for(size_t i = nodesize;i > 0; --i)
    {
      int inCStateRoundCount,state;
      QString label;
      qreal x,y,id;
      data >> id >> label >> state >> inCStateRoundCount >> x >> y;
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
      qreal id;int n;QString s;
      // skip unneeded first run
      data >> id >> s >> n >> n >> n >> n;
      QStringList sl = data.readLine().split(" ",QString::SkipEmptyParts);
      for(int i =0;i < sl.length(); i += 2)
        {
          nodepmap[id]->addNeighbor({nodepmap[sl[i].toDouble()],edgepmap[sl[i+1].toInt()]});
        }
    }
  data.readLine();
  for(size_t i = edgesize;i > 0; --i)
    {
      int id,n;QString s;
      //skip unneeded
      data >> id >> s >> n >> n >> n >> n;
      qreal fid,tid;
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
          for (auto j = (*i)->getAdlist().begin();j!=(*i)->getAdlist().end();)
            {
              qDebug() << std::get<1>(*j) << std::get<1>(*j)->getId();
              removeEdge(std::get<1>(*j));

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
      qDebug() << (*i)->getId() << edge->getId();
      if((*i)->getId() == edge->getId())
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
