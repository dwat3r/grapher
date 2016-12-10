#include "graphics.h"
#include <QTime>
#include <QCoreApplication>
#include <QApplication>
#include <QEventLoop>
#include <ctime>
//for matching
#include <deque>
#include <set>
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
                  for (neighbor n : node->getNeighbors())
                    {
                      if((n.second->getFrom() == selectedEdge->getFrom() &&
                          n.second->getTo() == node) ||
                         (n.second->getFrom() == node &&
                          n.second->getTo() == selectedEdge->getFrom()))
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
          selectedEdge = NULL;
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
          std::set<neighbor> n((*i)->getNeighbors());
          for (neighbor j : n)
            {
              removeEdge(j.second);

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
           << node->isInM()
           << node->pos().x()
           << node->pos().y();
      for (neighbor n : node->getNeighbors())
        {
          Node* first  = n.first;
          Edge* second = n.second;
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
      int inM;
      data >> id >> label >> bi >> inM >> x >> y;
      data.readLine();

      Node *node = new Node(id,static_cast<bipartition>(bi),inM,label,QPointF(x,y));
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
void graphics::matching()
{
  //reverse edge weights
  Edge* pewmax = *std::max_element(edges.begin(),edges.end(),
                [](Edge* a,Edge* b){return a->getWeight() < b->getWeight();});
  int ewmax = pewmax->getWeight();

  for (Edge* e : edges)
    e->setWeight(ewmax - e->getWeight());
  for (Edge* e : edges)
    e->setDirected(true);
  // create s and t
  Node *s = NULL;
  Node *t = NULL;
  drawST(s,t);
  // create empty M
  // M included in edge
  // create pi for nodes
  std::map<Node*,int> pi;
  // init pi
  for (Node* node : nodes)
    {
      if (node->getBi() == V1)
        pi[node] = 0;
      else if (node->getBi() == V2)
        {
          pi[node] = INT32_MAX; //positive infinity
          for (neighbor n : node->getNeighbors())
            if (n.second->getWeight() < pi[node])
              pi[node] = n.second->getWeight();
        }
      else
        //for s and t
        pi[node] = 0;
      node->setLabel(QString("%1").arg(pi[node]));
      node->update();
    }
  // create w for edges
  std::map<Edge*,int> w;
  // init w
  for (Edge* e : edges)
    {
      w[e] = pi[e->getFrom()] + e->getWeight() - pi[e->getTo()];
      e->setLabel(QString("%1").arg(w[e]));
    }
  //main loop
  while(true)
    {
      for (Node* node : nodes)
        {
          // create directed edges from s to nonM vertices of V1
          if(node->getBi() == V1 && !node->isInM())
            {
              Edge* e = new Edge(edgeId++,0,s,node);
              w[e] = 0;
              s->addNeighbor({node,e});
              edges.push_back(e);
              addItem(e);
            }
          // create directed edges from nonM vertices of V2 to t
          else if(node->getBi() == V2 && !node->isInM())
            {
              Edge* e = new Edge(edgeId++,0,node,t);
              w[e] = 0;
              node->addNeighbor({t,e});
              edges.push_back(e);
              addItem(e);
            }
        }

      // direct edges between V1 & V2 according to M
      // and reverse value if in M
      directEdges();

      // perform Dijkstra between s & t
      std::map<Node*,std::pair<int,Node*> > dp = dijkstra(s,t,pi,w);
      // fill P
      std::set<Edge*> P;
      for(auto& i : dp)
        {
          for(Edge* e : edges)
            {
              if(i.first == e->getFrom() && i.second.second == e->getTo())
                P.insert(e);
            }
        }
      // adjust M
      for(Edge* e : P)
        {
          // insert or delete edge on shortest path
          // according to M
          if(e->isInM())
            e->setInM(false);
          else
            e->setInM(true);
        }
      // adjust pi
      for(Node* n : nodes)
        {
          pi[n] += dp[n].first;
          if(n->getBi() != Neither)
            n->setLabel(QString("%1").arg(pi[n]));
        }
      // adjust w
      for(Edge* e : edges)
      {
          w[e] += pi[e->getFrom()] - pi[e->getTo()];
          e->setLabel(QString("%1").arg(w[e]));
      }
      //cleanup
      for (neighbor n : s->getNeighbors())
          removeEdge(n.second);
      for (neighbor n : t->getNeighbors())
          removeEdge(n.second);

      // if all nodes are in M, break;
      bool cond = true;
      for (Node *node : nodes)
        {
          if(!node->isInM() && node->getBi() != Neither)
            cond = false;
        }
      if (cond)
        break;
      else
        {
          QTime dieTime= QTime::currentTime().addMSecs(300);
          while (QTime::currentTime() < dieTime)
          QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
    }
}
//dijkstra
// returns list of edges of the shortest path
std::map<Node*,std::pair<int,Node*> > graphics::dijkstra(Node *source,Node *dest,
                                                         std::map<Node*,int>& pi,
                                                         std::map<Edge*,int>& w)
{
  // dist-prev structure holds result
  std::map<Node*,std::pair<int,Node*> > dp;
  std::deque<Node*> Q;
  //init
  for (Node* n : nodes)
    {
      dp[n].first = INT32_MAX; //ez a vegtelen most
      dp[n].second = NULL;
      Q.push_back(n);
    }
  dp[source].first = 0;

  while(!Q.empty())
    {
      //get minimal
      Node *u = Q[0];
      for (Node *n : Q)
        {
          if(dp[n].first < dp[u].first)
            u = n;
        }
      //terminate search if we reached dest
      if(u == dest)
        break;

      Q.erase(std::remove_if(Q.begin(),Q.end(),
                     [=](Node *n){return u == n;}));

      for (neighbor n : u->getNeighbors())
        {
          int alt = dp[u].first + w[n.second];
          if (alt < dp[n.first].first)
            {
              dp[n.first].first = alt;
              dp[n.first].second = u;
            }
        }
    }
  return dp;
}
// draws s and t nodes, based on the position of the existing ones
void graphics::drawST(Node *&s,Node *&t)
{

  // assume we're drawing bipartites the traditional way
  // create nodes beside the graph
  QPointF spos,tpos;
  qreal stop = 0,sbot = 0;
  qreal ttop = 0,tbot = 0;
  for (Node* node : nodes)
    {
      if (node->getBi() == V1)
        {
          if (node->x() < spos.x())
            spos.setX(node->x());
          if (node->y() > stop)
            stop = node->y();
          else if (node->y() < sbot)
            sbot = node->y();
        }
      else if (node->getBi() == V2)
        {
          if (node->x() > tpos.x())
            tpos.setX(node->x());
          if (node->y() > ttop)
            ttop = node->y();
          else if (node->y() < tbot)
            tbot = node->y();
        }
    }
  // doing pitagoras
  spos.setX(spos.x() - std::sqrt(std::pow(stop - sbot,2) -
                                 std::pow((stop - sbot) / 2,2)));
  spos.setY((stop - sbot) / 2);

  tpos.setX(tpos.x() + std::sqrt(std::pow(ttop - tbot,2) -
                                 std::pow((ttop - tbot) / 2,2)));
  tpos.setY((ttop - tbot) / 2);
  s = new Node(spos,nodeId++,Neither);
  t = new Node(tpos,nodeId++,Neither);
  s->setLabel("s");
  t->setLabel("t");
  nodes.push_back(s);
  nodes.push_back(t);
  addItem(s);
  addItem(t);
}
// direct edges between V1 & V2 according to M
// and reverse value if in M
void graphics::directEdges()
{
  for(Edge* e : edges)
    {
      // set neighborship
      if (e->getFrom()->isInM() && e->getTo()->isInM() &&
          e->getFrom()->getBi() == V1)
        {
          // swap!
          Node* n = e->getFrom();
          e->setFrom(e->getTo());
          e->setTo(n);
        }
      else if (e->getFrom()->getBi() == V2)
        {
          //swap!
          Node* n = e->getFrom();
          e->setFrom(e->getTo());
          e->setTo(n);
        }
      if (!(e->getFrom()->getBi() == Neither) &&
          !(e->getTo()->getBi() == Neither))
        e->getTo()->removeNeighbor(e->getFrom());
    }
  update();
}
