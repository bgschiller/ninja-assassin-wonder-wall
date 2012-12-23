#!/usr/local/bin/python
from implication import find_ordering
import random
from StringIO import StringIO

def random_game(players):
    relationships = []
    for i,p in enumerate(players):
        (ww, na) = random.sample(
                players[:i] + players[i+1:], 2)
        relationships.append((p,ww,na))
    return relationships

def count_solvable(n_players, n_samples):
    n_solvable = 0
    for i in range(n_samples):
        outfile = StringIO()
        if find_ordering(random_game(range(n_players)), outfile) is not None:
            n_solvable += 1
        outfile.close()
    return n_solvable

if __name__ == '__main__':
    n_samples = 1000
    max_size = 10
    print '{0:4<} {1:8<}'.format('n', 'P(solvable)')
    for game_size in range(4,max_size):
        print '{0:4<} {1:8<1.6f}'.format(game_size, 
                float(count_solvable(game_size, n_samples))/ n_samples)
