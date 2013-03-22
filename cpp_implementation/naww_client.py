#!/home/schillb/Research/ninja-assassin-wonder-wall/mypy/bin/python
'''designed to be called over ssh by naww_master.py
should pass along as command line parameters the location of a redis instance
and the number of children to maintain at a time.
The redis instance will include a list 'jobs', each entry of which will be of the form
n p1 w1 a1 p2 w2 a2 ...
where n is the number of players,
pi is the ith player
wi is the ith player's wonderwall
ai is the ith player's ninja assassin.
These are to be fed as input to the c++ implementation.
If there is a solution, it should be set as the value for key 
n p1 w1 a1 p2 w2 a2 ...
and the key should added to 'solved:n' (for appropriate n).
If there is no solution, add the key to 'unsolvable:n' (for appropriate n).'''


import multiprocessing
import subprocess
import redis

r = redis.Redis('newman.cs.wwu.edu')

def find_solution(problem):
    '''solve a single problem, presented as a string:
    n p1 w1 a1 p2 w2 a2 ...
    and push the results to redis'''
    solution = subprocess.check_output("echo {} | ./implication".format(problem), shell=True).translate(None, '\n')
    r.lrem('inprogress', 1, problem)
    size = int(problem.split()[0]) 
    if solution:
        r.sadd('solved:{}'.format(size), problem)
        r.hset('solutions:{}'.format(size), problem, solution)
    else:
        r.sadd('unsolvable:{}'.format(size),problem)

def problems():
    '''This generator will continue to produce values for the pool until 
    there are no more in the redis list'''
    new_prob = r.rpoplpush('jobs','inprogress')
    while new_prob is not None:
        yield new_prob
        new_prob = r.rpoplpush('jobs','inprogress')

if __name__=='__main__':
    num_cpus = multiprocessing.cpu_count()
    pool = multiprocessing.Pool(num_cpus * 2)
    pool.map_async(find_solution,problems()).wait()

