#!/usr/local/bin/python
from implication import find_ordering, count_pairs
import random
from StringIO import StringIO

def random_game(players):
    relationships = []
    for i,p in enumerate(players):
        (ww, na) = random.sample(
                players[:i] + players[i+1:], 2)
        relationships.append((p,ww,na))
    return relationships

def count_shared_pairs(relationships):
    '''Count the number of pairs in a relationship table that appear
    in more than one row. This tests a result found analytically that
    the average number of shared pairs approaches 11 as n->infty'''
    shared_pairs = 0
    for pair, count in count_pairs(relationships).most_common():
        if count == 1:
            break
        shared_pairs += count
    return shared_pairs

def combinations(lst, so_far=[]):
    if len(lst) == 0:
        return [so_far]
    head = lst[0]
    tail = lst[1:]
    combo_list = []
    for elem in head:
        combo_list.extend(combinations(tail, so_far + [elem]))
    return combo_list

def all_games(players):
    for p in players:
        choices = []
        for ww, na in ((w, n) for w in players for n in players 
                if (not w == p and not n == p and not n == w)):
            choices.append((p,ww,na))
        yield choices

def count_solvable(game_list):
    n_solvable = 0
    solve_depths = []
    unsolve_depths = []
    for relationships in game_list:
        outfile = StringIO()
        ordering, recursion_depth = find_ordering(relationships, outfile)
        if ordering is not None:
            n_solvable += 1
            solve_depths.append(recursion_depth)
        else:
            unsolve_depths.append(recursion_depth)
        outfile.close()
    return n_solvable, solve_depths, unsolve_depths

if __name__ == '__main__':
    n_samples = 1000
    max_size = 20
    print 'd_s: recursion depth for solvable problems'
    print 'd_u: recursion depth for unsolvable problems'
    print '{0:>4} {1:>12} {2:>12} {3:>12} {4:>12} {5:>12}'.format('n', 'P(solvable)', 'avg(d_s)', 'max(d_s)', 'avg(d_u)', 'max(d_u)')
    for game_size in range(4,max_size):
        n_solveable, solve_depths, unsolve_depths = count_solvable(
            (random_game(range(game_size)) for x in xrange(n_samples)))
        print '{0:>4} {1:>1.10f} {2:>1.10f} {3:>12} {4:>1.10f} {5:>12}'.format(game_size, 
                float(n_solveable)/ n_samples, 
                sum(solve_depths)/float(len(solve_depths)),
                max(solve_depths),
                sum(unsolve_depths)/float(len(unsolve_depths)),
                max(unsolve_depths))
