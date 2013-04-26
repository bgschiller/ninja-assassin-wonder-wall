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


#define max(x,y) x > y ? x : y

//typedef so that we can change this later
typedef adjacency_list<> graph_t;
typedef struct {
    int p;
    int w;
    int n;
} player_t;
bool operator== (const player_t& p1, const player_t& p2){
    return p1.p == p2.p;
}

typedef pair<int,int> Edge;

bool verbose = false;

/* function prototypes */
pair< vector<Edge>, vector<player_t> > ImpliedEdges(
        Edge guess, 
        const vector<player_t>& players, 
        unsigned short depth);

void AddImpliedEdges(
        graph_t& F, 
        Edge guess, 
        vector<player_t>& players, 
        unsigned short depth);

pair<bool,graph_t> MakeAssumption(
        graph_t& G, 
        vector<player_t> players, 
        size_t player_count, 
        unsigned short depth=0);

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
    add_edge(1,2,order_g);  //hack required because 
    //BGL causes a segfault if we ask edge() on an empty group
    remove_edge(1,2,order_g);   //(2nd part of hack)
    sort_on_pair_count(players);
    boost::tie(acyclic,order_g) =  MakeAssumption(
            order_g, players, players.size());
    vector<int> order;
    if (acyclic){
        topological_sort(order_g, back_inserter(order));
    }
    return order;
}

pair< vector<Edge>, vector<player_t> >
ImpliedEdges(
        Edge guess, 
        const vector<player_t>& players, 
        unsigned short depth){
//Assuming 'guess' is an edge, what edges must also exist in order
//to respect every player's constraints?
//returns a pair, .first is the edges to add
//.second is the players from which those edges are drawn
// (this is necessary because we need to evict those players
// from the set we're guessing with)
    vector<Edge> impl_edges;
    vector<player_t> used_players;
    int rr = guess.first;
    int ss = guess.second;
    for (size_t ii = 0; ii < players.size(); ++ii){
        int tt = players[ii].p;
        int ww = players[ii].w;
        int nn = players[ii].n;
        bool used_this_player = false;
        if (rr == tt && ss == ww){
            impl_edges.push_back(make_pair(ww,nn));
            impl_edges.push_back(make_pair(tt,nn));
            used_this_player = true;
        } else if (rr == ww && ss == tt){
            impl_edges.push_back(make_pair(nn,ww));
            impl_edges.push_back(make_pair(nn,tt));
            used_this_player = true;
        } else if (rr == tt && ss == nn){
            impl_edges.push_back(make_pair(tt,ww));
            impl_edges.push_back(make_pair(ww,nn));
            used_this_player = true;
        } else if (rr == nn && ss == tt){
            impl_edges.push_back(make_pair(ww,tt));
            impl_edges.push_back(make_pair(nn,ww));
            used_this_player = true;
        } else if (rr == ww && ss == nn){
            impl_edges.push_back(make_pair(tt,ww));
            impl_edges.push_back(make_pair(tt,nn));
            used_this_player = true;
        } else if (rr == nn && ss == ww){
            impl_edges.push_back(make_pair(ww,tt));
            impl_edges.push_back(make_pair(nn,tt));
            used_this_player = true;
        }
        if (used_this_player){
            used_players.push_back(players[ii]);
            if (verbose){
                auto new_edge = impl_edges.end();
                new_edge--;
                printf("%*s%d -> %d is given, "
                        "so player %d's choices give %d -> %d\n",
                        (depth+1)*2, " ", guess.first, guess.second,
                        players[ii].p, 
                        new_edge->first, new_edge->second);
                new_edge--;
                printf("%*s%d -> %d is given, "
                        "so player %d's choices give %d -> %d\n",
                        (depth+1)*2, " ", guess.first, guess.second,
                        players[ii].p, 
                        new_edge->first, new_edge->second);
            }
        }
    }
    return make_pair( impl_edges, used_players);
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
    memset(counts, 0, 
            sizeof(counts[0][0]) * players.size() *players.size());
    //counts will always be indexed on [a][b] such that a < b.
    for(size_t i=0; i < players.size(); i++){ // count up all pairs
        player_t pp = players[i];
        //we need to make a copy because order_triplet
        //screws with the values
        order_triplet(pp.p,pp.w,pp.n);
        //now pp.p < pp.w < pp.n
        counts[pp.p][pp.w] += 1;
        counts[pp.w][pp.n] += 1;
        counts[pp.p][pp.n] += 1;
    }
    vector<int> player_scores(players.size());
    for (size_t i=0; i < player_scores.size(); i++){
        //each player's score is the count of their most 
        //valuable pair
        player_t pp = players[i];
        order_triplet(pp.p,pp.w,pp.n);
        player_scores[i] = max(
                counts[pp.p][pp.w], 
                counts[pp.w][pp.n]);
        player_scores[i] = max(
                player_scores[i], 
                counts[pp.p][pp.n]);
    }
    //we want to sort players based on the values in player_scores,
    //so we sort the indices of player_score and reorder players 
    //according to that permutation on the indices.
    vector<size_t> perm = sort_indices(player_scores);
    vector<player_t> temp_players(players.size());
    for (size_t i = 0; i < perm.size(); i++){
        temp_players[i] = players[perm[i]];
    }
    players = temp_players;
}

void AddImpliedEdges(
        graph_t& F, 
        Edge guess, 
        vector<player_t>& players, 
        unsigned short depth){
    queue<Edge> Q;
    Q.push(guess);
    int rr, ss;
    while (! Q.empty()){
        Edge next = Q.front(); Q.pop();
        rr = next.first;
        ss = next.second;
        add_edge(rr,ss,F);
        vector<Edge> impl_edges;
        vector<player_t> used_players;
        tie(impl_edges, used_players) = ImpliedEdges(
                next, 
                players, 
                depth);
        for (Edge new_edge : impl_edges){
            if (add_edge(new_edge.first, new_edge.second, F).second){
                //add_edge.second is True if the edges 
                //did not already exist
                Q.push(new_edge);
            }
            else {
            }
        }
        for (size_t ii = 0; ii < players.size(); ++ii){
            if (std::find(used_players.begin(), used_players.end(),
                        players[ii]) != used_players.end()){
                players.erase(players.begin() + ii);
            }
        }
    }
}
/* MakeAssumption(G, players)
 * returns when either
 *      players is empty -- all edges are in G (success case)
 *      G contains a cycle -- this says we made a mistake and 
 *          must backtrack (failure case)
 */
pair<bool,graph_t> MakeAssumption(
        graph_t& G, 
        vector<player_t> players, 
        size_t player_count, 
        unsigned short depth){
    if (! is_acyclic(G)){ 
        //we don't have to go any farther -- already a contradiction
        if (verbose){
            printf("%*sG is cyclic! we must have assumed wrong...\n",
                    depth*2, " ");
        }
        return make_pair(false, G);
    }
    //G must be acyclic at this point
    if (players.size() == 0 || num_vertices(G) == player_count){
        if (verbose){
            printf("%*sNo more players to try, and G is acyclic:"
                   " Success!\n",depth*2, " ");
        }
        return make_pair(true,G); //base case!
    }
    
    vector<player_t> players_backup = players; 
    //make a copy in case this guess is wrong
    player_t player = players.back(); 
    //(we don't want to pop_back because AddImpliedEdges needs 
    //to add edge (player.w, player.n)
    //AddImpliedEdges will remove the players it uses 
    //from the players vector
    if (verbose){
        printf("%*sAssume %d -> %d\n", depth*2, " ", player.p, player.w);
    }
    graph_t F = G; //make a copy so as to not mess up the 
    //original if this guess is wrong
    AddImpliedEdges(
            F, 
            make_pair(player.p, player.w), 
            players, 
            depth);
    pair<bool, graph_t> result = MakeAssumption(
            F, 
            players,
            player_count, 
            depth+1);
    if (result.first){ //if F is complete and acyclic!
        return result;
    }
    F = G; //that didn't work... try the other way
    if (verbose){
        printf("%*sAssume %d -> %d\n", 
                depth*2, " ", 
                player.w, player.p);
    }
    AddImpliedEdges(
            F, 
            make_pair(player.w, player.p), 
            players_backup, 
            depth);
    return MakeAssumption(F, players_backup, player_count, depth+1); //Success or failure, this is our last shot.
}

int main(int argc, char* argv[])  
{
    if (argc > 1 && 
            (!strcmp(argv[1],"-v") || 
             !strcmp(argv[1],"--verbose"))){
        verbose = true;
    } else if (argc > 1 && 
            (!strcmp(argv[1], "-h") || 
             !strcmp(argv[1], "--help"))){
        printf("%s [-v] [-h]\n",argv[0]);
        printf("-v : verbose flag. Gives insight into assumptions"
                " and choices\n");
        printf("  (can also use '--verbose')\n");
        printf("-h : help flag. Display this message and quit\n");
        printf("  (can also use '--help').\n");
        printf("Note: only the first flag supplied will be used. "
                "It does not make sense to get verbose help.\n");
        printf("Give the Ninja Assassin Wonderwall problem as "
                "standard in formatted in the following way"
                "(newlines are ignored by the program but included"
                "here for readability):\n");
        printf("  number_players\n");
        printf("  0 w_0 n_0\n");
        printf("  1 w_1 n_1\n");
        printf("  .\n");
        printf("  .\n");
        printf("  .\n");
        printf("  number_players-1 w_(number_players-1) "
                "n_(number_players-1)\n");
        printf("where w_i, n_i are player i's wonderwall and "
                "ninja assassin, respectively\n");
        printf("If a solution to the Ninja Assassin Wonderwall "
                "problem exists, the output will give an ordering "
                "that is a solution. Otherwise, there will be no "
                "output.\n");
       exit(0); 
    }
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
Example input:
6 0 1 2 1 2 3 2 1 0 3 1 0 4 0 3 5 2 4 

Since newlines are ignored, this is equivalent to:
6 
0 1 2 
1 2 3 
2 1 0 
3 1 0 
4 0 3 
5 2 4 
*/
