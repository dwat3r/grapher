#include "graphics.h"
#include <QTime>
#include <QCoreApplication>
#include <QApplication>
#include <QEventLoop>
#include <ctime>
//for matching
#include <deque>
#include <algorithm>

//graphics
graphics::graphics()
  : QGraphicsScene()
  , nodes()
  , edges()
  , selectedEdge(NULL)
  , selectedNode(NULL)
  , drawmode(NodeDraw)
  , nodeId(0)
  , edgeId(0)
{
  srand (static_cast <unsigned> (time(0)));
  // timer for avoid messing with click events TODO
  timer.setInterval(QApplication::doubleClickInterval() + 100);
  timer.setSingleShot(true);
  connect(&timer,SIGNAL(timeout()), this, SLOT(timeout()));
  doubleClicked = false;
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
              removeNode(node);
              return;
            }
        }
      bipartition bi = V1;
      if(QGuiApplication::keyboardModifiers() == Qt::ShiftModifier){
          bi = V2;
        }
      Node* node = new Node(event->scenePos(),nodeId++,bi);
      nodes.push_back(node);
      addItem(node);
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
  //select node to move
  if(drawmode == NodeDraw)
    {
      for(Node* node : nodes)
        {
          if(node->contains(event->scenePos()))
            {
              selectedNode = node;
              break;
            }
        }
    }
  //start of edge draw
  else
    {
      for(Node* node : nodes)
        {
          if(node->contains(event->scenePos()))
            {
              selectedEdge = new Edge(node,edgeId++);
              addItem(selectedEdge);
              break;
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
              // and also disallow edges between same colour
              bool cond = false;
              //loop
              if (selectedEdge->getFrom() == node)
                cond = true;
              //same colour
              else if (selectedEdge->getFrom()->getBi() == node->getBi())
                cond = true;
              //parallel
              else
                {
                  for (neighbor n : node->getAdlist())
                    {
                      if((std::get<1>(n)->getFrom() == selectedEdge->getFrom() &&
                          std::get<1>(n)->getTo() == node) ||
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
                  edges.push_back(selectedEdge);
                  // add neighbors
                  node->addNeighbor({selectedEdge->getFrom(),selectedEdge});
                  selectedEdge->getFrom()->addNeighbor({node,selectedEdge});
                  update();
                  selectedEdge = NULL;
                }
              break;
            }
        }
      if(selectedEdge != NULL)
        {
          removeItem(selectedEdge);
          delete selectedEdge;
        }
    }
}
void graphics::editValueEvent(QPointF pos,int value)
{
  for (Edge *edge : edges)
    {
      if(edge->contains(pos))
        {
          edge->setWeight(value);
          edge->update();
          return;
        }
    }
}


void graphics::removeNode(Node *node)
{
  for(auto i = nodes.begin();i!= nodes.end();++i)
    {
      if(*i == node)
        {
          removeItem(*i);
          std::vector<neighbor> n(std::move((*i)->getAdlist()));
          for (neighbor j : n)
            {
              removeEdge(std::get<1>(j));

            }
          nodes.erase(i);
          node->removeFromNeighbors();
          delete node;
          return;
        }
    }
}
void graphics::removeEdge(Edge *edge)
{
  for(auto i = edges.begin();i!= edges.end();++i)
    {
      if((*i)->getId() == edge->getId())
        {
          removeItem(*i);
          edges.erase(i);
          edge->removeFromNeighbors();
          delete edge;
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
//serialization
QTextStream& operator << (QTextStream &data,graphics &g)
{

  // these contain the mappings between objects and their serialized ids
  data.setFieldWidth(10);
  QMap<Node*,int> nodepmap;
  QMap<Edge*,int> edgepmap;
  /*data looks like this:
  nodesize edgesize
  (first nodes):
  id label bi x y [neighborNodeId neigborEdgeId]*
  ###
  (then edges):
  id label startx starty endx endy fromId toId
  */
  data << g.nodes.size() << g.edges.size() << '\n';

  for (Node* node : g.nodes)
    {
      data << node->getId()
           << node->getLabel()
           << node->getBi()
           << node->pos().x()
           << node->pos().y();
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
           << edge->getWeight()
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
  QMap<int,Node*> nodepmap;
  QMap<int,Edge*> edgepmap;
  //reconstruct objects from serialized objects
  size_t nodesize,edgesize;

  data >> nodesize >> edgesize;

  // first run:
  // ignore pointers since we dont have the objects yet

  for(size_t i = nodesize;i > 0; --i)
    {
      int id,bi;
      QString label;
      qreal x,y;
      data >> id >> label >> bi >> x >> y;
      data.readLine();

      Node *node = new Node(id,static_cast<bipartition>(bi),label,QPointF(x,y));
      g.nodes.push_back(node);
      g.addItem(node);
      nodepmap[id] = node;
    }
  data.readLine();//ignore ###
  for(size_t i = edgesize;i > 0; --i)
    {
      int id,weight;
      QString label;
      qreal x1,y1,x2,y2;
      data >> id >> weight >> label >> x1 >> y1 >> x2 >> y2;
      data.readLine();

      Edge *edge = new Edge(id,weight,label,QPointF(x1,y1),QPointF(x2,y2));
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
//matching algorithm
//first dijkstra
std::map<Node*,int> graphics::dijkstra(Node* source)
{
  std::map<Node*,int> dist;
  std::map<Node*,Node*> prev;
  std::deque<std::pair<Node*,int> > Q;
  //init
  for (Node* n : nodes)
    {
      dist[n] = 100000; //ez a vegtelen most
      prev[n] = NULL;
      Q.push_back({n,dist[n]});
    }
  dist[source] = 0;

  while(!Q.empty())
    {
      //get minimal
      auto pu = std::min_element(Q.begin(),Q.end(),
                  [](std::pair<Node*,int> a,std::pair<Node*,int> b){
                      return std::get<1>(a) < std::get<1>(b);});
      Node *u = std::get<0>(*pu);
      Q.erase(pu);

      for (neighbor n : u->getAdlist())
        {
          int alt = dist[u] + std::get<1>(n)->getWeight();
          if (alt < dist[std::get<0>(n)])
            {
              dist[std::get<0>(n)] = alt;
              prev[std::get<0>(n)] = u;
            }
        }
    }
  return dist;
}
// then matching
void graphics::matching()
{
  //reverse edge weights
  Edge* pewmax = *std::max_element(edges.begin(),edges.end(),
                [](Edge* a,Edge* b){return a->getWeight() < b->getWeight();});
  int ewmax = pewmax->getWeight();

  for (Edge* e : edges)
      e->setWeight(ewmax - e->getWeight());

  // create s and t
  // and create directed edges

  //
}
