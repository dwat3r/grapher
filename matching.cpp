#include "graphics.h"
#include <QCoreApplication>
#include <QEventLoop>
#include <deque>
#include <set>
#include <algorithm>

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
              else if (i.first == e->getTo() && i.second.second == e->getFrom())
                P.insert(e);
            }
        }
      // adjust M
      for(Edge* e : P)
        {
          // insert or delete edge on shortest path
          // according to M
          if(e->isInM())
            {
              e->setInM(false);
              e->getFrom()->setInM(false);
              e->getTo()->setInM(false);
            }
              else
            {
              e->setInM(true);
              e->getFrom()->setInM(true);
              e->getTo()->setInM(true);
            }
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
      for (auto e = edges.begin();e!=edges.end();)
      {
        if((*e)->getTo() == t)
            removeEdgeIt(e);
        else
          ++e;
      }
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
  if (nodes.size() > 2)
    {
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
    }
  else
    //case for only 1 pair
    {
      for (Node* node : nodes)
        {
          if (node->getBi() == V1)
            {
              spos = node->pos();
              spos.setX(spos.x() - 50);
            }
          else if (node->getBi() == V2)
            {
              tpos = node->pos();
              tpos.setX(tpos.x() + 50);
            }
        }
    }
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
