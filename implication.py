#! /usr/local/bin/python
from collections import Counter, deque
import networkx as nx
import sys

def is_edge(g, edge):
    """edge should be a 2-tuple. returns True if edge is in the graph, False if it is not"""
    (a,b) = edge
    try:
        g[a][b]
        return True
    except KeyError:
        return False

def add_preds(g, x, queue=None, preds=[]):
    """ONLY call this on directed acyclic graphs. On a graph with a cycle, 
    it is an infinite recusion.
    This function depth-first traverses the graph, keeping track of all 
    predecessors and directing the appropriate edges.
    For example if we call the function on a graph:
        a --> b --> c
              |
              -> d --> e
    will add edges a -> c, a -> d, a -> e, and b -> e (the direct edges are 
    already present)
    These edges will also be put on the queue to be processed, if one is given"""
    for pred in preds:
        if not is_edge(g, (pred,x)):
            g.add_edge(pred,x)
            if queue is not None:
                queue.append((pred,x))
    succs = g[x].keys()
    for succ in succs:
        add_preds(g,succ, queue, preds + [x])
    
def make_assumption(pair, pairs_count, given_graph, relationships, outfile,
        reverse_checked=False, depth=1):
    """Given an edge to start with, make_assumption fixes that edge in the given orientation.
    Then it fixes all the edges implied by this orientation. If this produces a DAG, recurse 
    on another edge. Otherwise, try the opposite orientation.
    pair -- a 2-tuple (x,y) indicating that we should start by assuming x -> y
    pairs_count -- an instance of Counter, representing the number of times each 2-tuple is 
    represented in the relationship table, minus the number of times it is represented in 
    the graph.
    given_graph -- the edges that are already directed. make_assumption will honor these 
    directions.
    relationships -- a list of 3-tuples (x,y,z) where y is x's WW and z is x's NA
    reverse_checked -- if true, we have already concluded that the opposite orientation 
    leads to a cyclic graph, so if this orientation doesn't work out, there is no solution.
    depth -- should not be set at top level call. depth determines the indentation level, 
    so that the output visually represents the recursion depth. It is also used as a metric to evaluate the algorithm's performance"""

    outfile.write( '{spaces}Assume {u} -> {v}:\n'.format(spaces=' '*(2*(depth-1)), u=pair[0], v=pair[1]))
    working_graph = given_graph.copy()
    to_process = deque([pair])
    working_count = pairs_count.copy()
    while len(to_process) > 0:
        (a,b) = to_process.popleft()
        working_count[tuple(sorted((a,b)))] = 0
        working_graph.add_edge(a,b)
        for (x,y,z) in relationships: #make all the implications we can from that edge
            to_add = None
            if (x,y) == (a,b):
                to_add = (y,z)
            elif (y,x) == (a,b):
                to_add = (z,y)
            elif (y,z) == (a,b):
                to_add = (x,y)
            elif (z,y) == (a,b):
                to_add = (y,x)
            elif (x,z) == (a,b):
                to_add = (y,z) # we should also add (x,y), but add_preds will take care of it.
            elif (z,x) == (a,b):
                to_add = (z,y)
            if to_add is not None and (to_add not in working_graph.edges_iter()):
                outfile.write( '{spaces}{a} -> {b} is given, so relationship {xyz} => {w} -> {u}\n'.format(
                        spaces=' '*(2*(depth-1)), a=a, b=b, xyz=(x,y,z), w=to_add[0], u=to_add[1]))
                working_graph.add_edge(*to_add)
                to_process.append(to_add)
        if nx.is_directed_acyclic_graph(working_graph):
            # If we have an edge a->b, and an edge b->c, we need to add the edge a->c so that 
            #we can make assumptions based on that.
            nodes = working_graph.nodes()
            for node in nodes:
                add_preds(working_graph, node, to_process)
        else:
            break #we already have a cycle. we must have assumed wrong

    if nx.is_directed_acyclic_graph(working_graph):
        if len(working_graph.nodes()) == len(relationships):
            return True, working_graph, depth #success!
        next_pair = sorted(working_count.most_common(1)[0][0])#no contradictions, but not done yet
        return make_assumption(next_pair, working_count, working_graph, relationships,
                outfile, depth=depth+1)
    elif not reverse_checked: #that didn't work out... check the other side!
        outfile.write( '{spaces}Cycle in graph! try the reverse...\n'.format(spaces=' '*(2*depth)))
        return make_assumption(list(reversed(pair)), pairs_count, given_graph, 
                relationships, outfile, reverse_checked=True, depth=depth)
    else:
        outfile.write('{spaces}Cycle in graph, and we\'ve already checked the other side! Returning false.\n'.format(
                spaces=' '*2*depth))
        return False, working_graph, depth

def count_pairs(relationships):
    """relationships should be a list of 3-tuples (x,y,z). return a Counter
    object with a count for every (x,y), (y,z), and (x,z)"""
    pair_count = Counter()
    for (x,y,z) in relationships:
        pair_count[tuple(sorted([x,y]))] += 1
        pair_count[tuple(sorted([y,z]))] += 1
        pair_count[tuple(sorted([x,z]))] += 1
    return pair_count

def find_ordering(relationships, outfile=sys.stdout):
    """main function. Given a table of relationships, either print a solution or 
    print 'no solution' and list the simple cycles.
    relationships should be a list of 3-tuples, (x,y,z) where y is x's WW and z is x's NA"""
    pair_count = count_pairs(relationships)
    starting_pair = pair_count.most_common(1)[0][0]
    order_graph = nx.DiGraph()
    solvable, order_graph, recursion_depth = make_assumption(starting_pair, pair_count, 
            order_graph, relationships, outfile)
    if solvable:
        order = nx.topological_sort(order_graph)
        outfile.write('solution: {0}\n'.format(order))
        return order, recursion_depth
    else:
        outfile.write('no solution.\n')
        for cycle in nx.simple_cycles(order_graph):
            outfile.write('cycle: {0}\n'.format(' -> '.join(map(str, cycle))))
        return None, recursion_depth

if __name__ == '__main__':
    relationships = [('a','b','c'),('b','c','d'),('c','b','a'),('d','b','a'),('e','a','d'),
            ('f','c','e')]
    print 'is there a solution to {0}?'.format(relationships)
    find_ordering(relationships)
    print ''
    relationships = [(1,2,3),(2,4,6),(3,4,7),(4,1,5),(5,6,3),(6,1,7),(7,2,5)]
    print 'is there a solution to {0}?'.format(relationships)
    find_ordering(relationships)
