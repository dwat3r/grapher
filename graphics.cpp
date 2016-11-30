#include "graphics.h"
#include "node.h"
#include "edge.h"
#include <QDebug>
#include <ctime>
#include <QTime>
#include <QCoreApplication>
#include <QEventLoop>


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
              removeNode(node);
              return;
            }
        }
      Node *node = new Node(event->scenePos(),nodeId++);
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
              qDebug() << std::get<1>(j) << std::get<1>(j)->getId();
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
      qDebug() << (*i)->getId() << edge->getId();
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
