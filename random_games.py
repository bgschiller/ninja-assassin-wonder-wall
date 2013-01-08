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
    choice_lists = []
    for p in players:
        choices = []
        for ww, na in ((w, n) for w in players for n in players 
                if (not w == p and not n == p and not n == w)):
            choices.append((p,ww,na))
        choice_lists.append(choices)
    return combinations(choice_lists)

def count_solvable(game_list):
    n_solvable = 0
    for relationships in game_list:
        outfile = StringIO()
        if find_ordering(relationships, outfile) is not None:
            n_solvable += 1
        outfile.close()
    return n_solvable

if __name__ == '__main__':
    n_samples = 1000
    max_size = 10
    print '{0:4<} {1:8<}'.format('n', 'P(solvable)')
    for game_size in range(4,max_size):
        print '{0:4<} {1:8<1.6f}'.format(game_size, 
                float(count_solvable((random_game(range(game_size)) for x in xrange(n_samples))))/ n_samples)
