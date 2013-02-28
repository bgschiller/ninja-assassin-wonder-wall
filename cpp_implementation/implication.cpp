#include<iostream>
#include<stdlib.h>
#include <boost/utility.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/topological_sort.hpp>
using namespace boost;
using namespace std;

//so that we can change the defaults
typedef adjacency_list<> graph_t;
typedef struct {
    int p;
    int w;
    int n;
} player_t;
typedef pair<int,int> Edge;
typedef graph_traits<Graph>::vertex_descriptor Vertex;

/* function prototypes */
vector<Edge> ImpliedEdges(Edge guess, const vector<player_t>& players);
void AddImpliedEdges(graph_t& F, Edge guess, const vector<player_t>& players);
pair<bool,graph_t> MakeAssumption(graph_t& G, vector<player_t> players);

struct cycle_detector : public dfs_visitor<> //code from boost/graph/example/file_dependencies.cpp
{
    cycle_detector(bool& has_cycle) : m_has_cycle(has_cycle) { }
      template <class Edge, class Graph>
           void back_edge(Edge, Graph&) { m_has_cycle = true; }
    protected:
       bool& m_has_cycle;
};

bool is_acyclic(const graph_t& G)
{
    bool has_cycle = false;
    cycle_detector vis(has_cycle);
    depth_first_search(G, visitor(vis));
    return has_cycle;
}



vector<Vertex> FindOrdering(vector<player_t> players){
    graph_t order_g();
    pair<bool,graph_t> result = MakeAssumption(order_g, players);
    vector<Vertex> order;
    if (result.first){
        topological_sort(result.second, front_inserter(order));
    }
    return order;
}

vector<Edge> ImpliedEdges(Edge guess, const vector<player_t>& players){
    vector<Edge> impl_edges();
    Vertex rr = guess.first;
    Vertex ss = guess.second;
    for (size_t ii = 0; ii < players.size(); ++ii){
        int tt = players[ii].p;
        int ww = players[ii].w;
        int nn = players[ii].n;
        if (rr == tt && ss == ww){
            impl_edges.push_back(make_pair(ww,nn));
            impl_edges.push_back(make_pair(tt,nn));
        } else if (rr == ww && ss == tt){
            impl_edges.push_back(make_pair(nn,ww));
            impl_edges.push_back(make_pair(nn,tt));
        } else if (rr == tt && ss == nn){
            impl_edges.push_back(make_pair(tt,ww));
            impl_edges.push_back(make_pair(ww,nn));
        } else if (rr == nn && ss == tt){
            impl_edges.push_back(make_pair(ww,tt));
            impl_edges.push_back(make_pair(nn,ww));
        } else if (rr == ww && ss == nn){
            impl_edges.push_back(make_pair(tt,ww))
            impl_edges.push_back(make_pair(tt,nn));
        } else if (rr == nn && ss == ww){
            impl_edges.push_back(make_pair(ww,tt));
            impl_edges.push_back(make_pair(nn,tt));
        }
    }
    return impl_edges;
}

void AddImpliedEdges(graph_t& F, Edge guess, const vector<player_t>& players){
    queue<Edge> Q;
    Q.push_back(guess);
    while (! Q.empty()){
        Edge next = Q.pop_front();
        rr = next.first();
        ss = next.second();
        if (add_edge(rr,ss,F).second){ //second gives true if edge is new
            vector<Edge> impl_edges = ImpliedEdges(next, players);
            for (Edge new_edge : impl_edges){
                if (add_edge(new_edge.first, new_edge.second, F).second){
                    Q.push_back(new_edge);
                }
            }
        }
    }
}
/* MakeAssumption(G, players)
 * returns when either
 *      players is empty -- all edges are in G (success case)
 *      G contains a cycle -- this says we made a mistake and must backtrack (failure case)
 */
pair<bool,graph_t> MakeAssumption(graph_t& G, vector<player_t> players){
    if (players.size() == 0){
        return make_pair(is_acyclic(G), G); //success base case!
    }
    if (! is_acyclic(G)){ //we don't have to go any farther -- already a contradiction
        return make_pair(false, G);
    }
    player_t player = players.back(); //don't forget to pop_back before recursing.
    //(we don't want to pop_back yet because AddImpliedEdges needs to add edge (player.w, player.n)
    graph_t F(G); //make a copy so as to not mess up the original if this guess is wrong
    AddImpliedEdges(F, make_pair(player.p, player.w), players);
    players.pop_back();
    pair<bool, graph_t> result = MakeAssumption(F, players);
    if (result.first){ //if F is complete and acyclic!
        return make_pair(true,F);
    }
    F = G; //that didn't work... try the other way
    players.push_back(player) //give AddImpliedEdges the context to add edge (player.n,player.w)
    AddImpliedEdges(F, make_pair(player.p, player.w), players);
    players.pop_back();
    return MakeAssumption(F, players); //Success or failure, this is our last shot.
}

int main()  
{
    int p[] = {1,2,3,4,5,6};
    int w[] = {2,3,2,2,1,3};
    int n[] = {3,4,1,1,4,5};
    vector<player_t> players();
    for (int i = 0; i < 6; ++i){
        player_t this_player = {p[i], w[i], n[i] };
        players.push_back(this_player);
    }
    vector<int> ordering = FindOrdering(players);
    for( vector<int>::const_iterator i = ordering.begin(); i != ordering.end(); ++i)
            std::cout << *i << ' ';
    cout << endl;
    return 0;
}


