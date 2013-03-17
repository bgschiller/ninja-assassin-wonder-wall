#include<iostream>
#include<stdlib.h>
#include<queue>
#include<algorithm>
#include <boost/utility.hpp>


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/lookup_edge.hpp>
using namespace boost;
using namespace std;

#define DEBUG

#ifdef DEBUG
#define SPACES(x) for(int ii=0; ii<2*x; ii++) printf(" ");
#define OUTPUT(x) printf x
#else
#define SPACES(x) 
#define OUTPUT(x) 
#endif

#define max(x,y) x > y ? x : y

//typedef so that we can change this later
typedef adjacency_list<> graph_t;
typedef struct {
    int p;
    int w;
    int n;
} player_t;
typedef pair<int,int> Edge;

/* function prototypes */
vector<Edge> ImpliedEdges(Edge guess, const vector<player_t>& players);
void AddImpliedEdges(graph_t& F, Edge guess, const vector<player_t>& players, unsigned short depth);
pair<bool,graph_t> MakeAssumption(graph_t& G, vector<player_t> players, unsigned short depth=0);
void sort_on_pair_count(vector<player_t>& players);

struct cycle_detector : public dfs_visitor<> 
{//code from boost/graph/example/file_dependencies.cpp
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
    return ! has_cycle;
}



vector<int> FindOrdering(vector<player_t> players){
    graph_t order_g;
    bool acyclic;
    OUTPUT(("inside FindOrdering\n"));
    add_edge(1,2,order_g);      //BGL causes a segfault if we ask edge() on an empty group
    remove_edge(1,2,order_g);   //(2nd part of hack)
    sort_on_pair_count(players);
    boost::tie(acyclic,order_g) =  MakeAssumption(order_g, players);
    vector<int> order;
    if (acyclic){
        topological_sort(order_g, back_inserter(order));
    }
    return order;
}

vector<Edge> ImpliedEdges(Edge guess, const vector<player_t>& players){
    vector<Edge> impl_edges;
    int rr = guess.first;
    int ss = guess.second;
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
            impl_edges.push_back(make_pair(tt,ww));
            impl_edges.push_back(make_pair(tt,nn));
        } else if (rr == nn && ss == ww){
            impl_edges.push_back(make_pair(ww,tt));
            impl_edges.push_back(make_pair(nn,tt));
        }
    }
    return impl_edges;
}

void inline swap(int& x, int& y){
    int temp = x;
    x = y;
    y = temp;
}

void order_triplet(int& a, int& b, int& c){
    //Given three unique integers, re-assign them so that
    // a < b < c 
    // where the set {a,b,c} remains unchanged
    // (we move the values around, but they're all still there)
    if (a < b && b < c){ //abc
        return;
    } else if (b < a && a < c){ //bac
        swap(a,b);
    } else if ( c < a && a < b){ //cab
        swap(c,a);
        swap(c,b);
    } else if (a < c && c < b){ //acb
        swap(c,b);
    } else if (c < b && b < a){ //cba
        swap(a,c);
    } else if (b < c && c < a){//bca
        swap(b,c);
        swap(a,c);
    }
#ifdef DEBUG
    if (! ( a<b && b < c)){
        OUTPUT(("Uh oh, order triplet failed.\n"));
        exit(127);
    }
#endif
}

template <typename T>
vector<size_t> sort_indices(const vector<T> &v) {

  // initialize original index locations
  vector<size_t> idx(v.size());
  for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;

  // sort indices based on comparing values in v
  sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

  return idx;
}

void sort_on_pair_count(vector<player_t>& players){
    int counts[players.size()][players.size()] ;
    memset(counts, 0, sizeof(counts[0][0]) * players.size() *players.size());
    //counts will always be indexed on [a][b] such that a < b.
#ifdef DEBUG
    for (size_t i=0; i < players.size(); i++){
        for (size_t j=0; j < players.size(); j++){
            if (counts[i][j] != 0) OUTPUT(("uninitialized counts!!\n"));
        }
    }
#endif
    for(int i=0; i < players.size(); i++){ // count up all pairs
        player_t pp = players[i];//we need to make a copy because order_triplet screws with the values
        order_triplet(pp.p,pp.w,pp.n);
        //now pp.p < pp.w < pp.n
        counts[pp.p][pp.w] += 1;
        counts[pp.w][pp.n] += 1;
        counts[pp.p][pp.n] += 1;
    }
    vector<int> player_scores(players.size());
    for (size_t i=0; i < player_scores.size(); i++){
        //each player's score is the count of their most valuable pair
        player_t pp = players[i];
        order_triplet(pp.p,pp.w,pp.n);
        player_scores[i] = max(counts[pp.p][pp.w], counts[pp.w][pp.n]);
        player_scores[i] = max(player_scores[i], counts[pp.p][pp.n]);
    }
    //we want to sort players based on the values in player_scores,
    //so we sort the indices of player_score and reorder players according
    //to that permutation on the indices.
#ifdef DEBUG
    OUTPUT(("scores are \n"));
    for(int s: player_scores){
        OUTPUT(("%d ",s));
    }
    OUTPUT(("\n"));
#endif
    vector<size_t> perm = sort_indices(player_scores);
#ifdef DEBUG
    OUTPUT(("perms is \n"));
    for (size_t p: perm){
        OUTPUT(("%u ",p));
    }
    OUTPUT(("\n"));
#endif
    vector<player_t> temp_players(players.size());
    for (size_t i = 0; i < perm.size(); i++){
        temp_players[i] = players[perm[i]];
    }
    players = temp_players;
#ifdef DEBUG
    OUTPUT(("sorted order:\n"));
    for (player_t p: players){
        OUTPUT(("%d ",p.p));
    }
    OUTPUT(("\n"));
#endif
}

     

void AddImpliedEdges(graph_t& F, Edge guess, const vector<player_t>& players, unsigned short depth){
    queue<Edge> Q;
    Q.push(guess);
    int rr, ss;
    SPACES(depth);
    OUTPUT(("guessing %d -> %d\n",guess.first, guess.second));
    while (! Q.empty()){
        Edge next = Q.front(); Q.pop();
        rr = next.first;
        ss = next.second;
        SPACES(depth);
        OUTPUT(("%d -> %d is an edge\n",rr,ss));
        auto edge_exists = boost::lookup_edge(rr,ss,F);
        if (! edge_exists.second){ //second gives true if edge exists
            add_edge(rr,ss,F);
            vector<Edge> impl_edges = ImpliedEdges(next, players);
            for (Edge new_edge : impl_edges){
                if (! edge(new_edge.first, new_edge.second, F).second){
                    add_edge(new_edge.first, new_edge.second, F);
                    Q.push(new_edge);
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
pair<bool,graph_t> MakeAssumption(graph_t& G, vector<player_t> players, unsigned short depth){
    SPACES(depth);
    OUTPUT(("top of MakeAssumption\n"));
    SPACES(depth);
    OUTPUT(("still %d players left in list", (int) players.size()));
    if (players.size() == 0){
        auto ret_val = make_pair(is_acyclic(G), G);
        SPACES(depth);
        OUTPUT(("G is %scyclic\n",ret_val.first ? "a" : ""));
        return ret_val; //base case!
    }
    if (! is_acyclic(G)){ //we don't have to go any farther -- already a contradiction
        SPACES(depth);
        OUTPUT(("G is cyclic!\n"));
        return make_pair(false, G);
    }
    player_t player = players.back(); //don't forget to pop_back before recursing.
    //(we don't want to pop_back yet because AddImpliedEdges needs to add edge (player.w, player.n)
    SPACES(depth);
    OUTPUT(("Assuming %d -> %d\n",player.p,player.w));
    graph_t F = G; //make a copy so as to not mess up the original if this guess is wrong

    AddImpliedEdges(F, make_pair(player.p, player.w), players, depth);
    players.pop_back();
    pair<bool, graph_t> result = MakeAssumption(F, players, depth+1);
    if (result.first){ //if F is complete and acyclic!
        return result;
    }
    F = G; //that didn't work... try the other way
    players.push_back(player); //give AddImpliedEdges the context to add edge (player.n,player.w)
    SPACES(depth);
    OUTPUT(("Assuming %d -> %d\n",player.w,player.p));
    AddImpliedEdges(F, make_pair(player.w, player.p), players, depth);
    players.pop_back();
    return MakeAssumption(F, players, depth+1); //Success or failure, this is our last shot.
}

int main()  
{
    int n;
    cin >> n;
    vector<player_t> players(n);
    for(int i=0; i < n; i++){
        cin >> players[i].p >> players[i].w >> players[i].n;
    }
 
    vector<int> ordering = FindOrdering(players);
    for(int i = ordering.size() - 1; i >= 0 ; i-- )
            std::cout << ordering[i] << ' ';
    cout << endl;
    return 0;
}

   /*
     *
6 0 1 2 1 2 3 2 1 0 3 1 0 4 0 3 5 2 4 

6 
0 1 2 
1 2 3 
2 1 0 
3 1 0 
4 0 3 
5 2 4 
*/
