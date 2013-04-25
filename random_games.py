#!/home/schillb/Research/ninja-assassin-wonderwall/mypy/bin/python
from implication import find_ordering, count_pairs
import random
from StringIO import StringIO
import itertools

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

def all_choices(players):
    for p in players:
        choices = []
        for ww, na in ((w, n) for w in players for n in players 
                if (not w == p and not n == p and not n == w)):
            choices.append((p,ww,na))
        yield choices

def all_games(players):
    return itertools.product(*all_choices(players))

def display_game(game):
    return '{} {}'.format(len(game),
            ' '.join(map(lambda tup: ' '.join(map(str,tup)), game)))

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
    import redis
    r = redis.Redis('newman.cs.wwu.edu')
    num_games = 10000
    highest = 80
    while highest < 100:
        print 'generating {} random games of size {}'.format(num_games, highest + 1)
        cnt = 0
        for game in (random_game(range(highest + 1)) for i in xrange(num_games)):
            if cnt == num_games/4:
                print '1/4 of the way'
            elif cnt == num_games/2:
                print '1/2 of the way'
            elif cnt == num_games*3/4:
                print '3/4 of the way'
            cnt += 1
            r.rpush('jobs',display_game(game))
        highest = highest + 1
