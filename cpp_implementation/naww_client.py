#!/home/schillb/Research/ninja-assassin-wonder-wall/mypy/bin/python
'''
The redis instance will include a list 'jobs', each entry of which will be of the form
n p1 w1 a1 p2 w2 a2 ...
where n is the number of players,
pi is the ith player
wi is the ith player's wonderwall
ai is the ith player's ninja assassin.
These are to be fed as input to the c++ implementation.
If there is a solution, increase 'solveable_count:n' (for appropriate n).
If there is no solution, increase 'unsolvable_count:n' (for appropriate n).
In either case, increase 'n_total:n' (for appropriate n).'''

import multiprocessing
import itertools
import subprocess
import redis

def find_solution(problem):
    '''solve a single problem, presented as a string:
    n p1 w1 a1 p2 w2 a2 ...
    and push the results to redis'''
    size = int(problem.split()[0]) 
    r.incr('n_total:{}'.format(size))
    solution = subprocess.check_output("echo {} | /home/schillb/Research/ninja-assassin-wonder-wall/cpp_implementation/implication".format(problem), shell=True).translate(None, '\n')
    if solution:
        r.incr('solvable_count:{}'.format(size))
    else:
        r.incr('unsolvable_count:{}'.format(size))

def problems():
    '''This generator will continue to produce values for the pool until 
    there are no more in the redis list'''
    new_prob = r.lpop('jobs')
    while new_prob is not None:
        yield new_prob
        new_prob = r.lpop('jobs')
    exit(0)

if __name__=='__main__':
    r = redis.Redis('newman.cs.wwu.edu')
    probs = problems()
    num_cpus = multiprocessing.cpu_count()
    pool = multiprocessing.Pool(num_cpus * 2)
    NN = num_cpus * 4
    while True:
        pool.map_async(find_solution,itertools.islice(probs, NN)).wait()

