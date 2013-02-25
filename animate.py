import math as m
import time
class Vector(object):
    def __init__(self,x,y):
        self.x=x
        self.y=y

    def __str__(self):
        return 'Vector({},{})'.format(self.x,self.y)

    __repr__ = __str__

    def normalize(self):
        mag = self.mag()
        self.x /= mag
        self.y /= mag
        return self

    def mag(self):
        return m.sqrt(self.x*self.x + self.y*self.y)

    def dot(self,b):
        return self.x * b.x + self.y * b.y

    def line(self, b):
        slope = Vector(self.x - b.x, self.y - b.y)
        slope.normalize()
        offset = Vector(b.x, b.y)
        return slope, offset

    def dir_to_line(self, slope, offset):
        '''gives a vector representing the direction from self to the given line
        slope and offset should be vectors as returned by Vector.line()-- in
        particular, slope should be normalized.
        the vector v returned is such that self + v is a point on the line.
        as described at 
        http://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line'''
        return (offset - self) - slope * ((offset - self).dot(slope)) 

    def __add__(self,b):
        return Vector(self.x + b.x, self.y + b.y)
    def __neg__(self):
        return Vector(-self.x, -self.y)
    def __sub__(self, b):
        return self + -b
    def __iadd__(self,b):
        self = self + b
    def __isub__(self,b):
        self = self - b
    def __mul__(self,b):
        if type(b) == type(self):
            #perform element-wise
            return Vector(self.x * b.x, self.y * b.y)
        return Vector(self.x*b, self.y*b)
    def __imul__(self,k):
        self = self * k
    def __div__(self,b):
        if type(b) == type(self):
            # element-wise division
            return Vector(self.x/b.x, self.y/b.y)
        return Vector(self.x/b, self.y/b)
    def __idiv__(self, b):
        self = self / b
class Player(object):
    def __init__(self,name,pos,ww,na):
        self.name=name
        self.pos=pos
        self.na=na
        self.ww=ww
    def __str__(self):
        return 'Player(name={},pos={},ww={},na={})'.format(
                self.name,
                self.pos,
                self.na,
                self.ww)
    __repr__=__str__

class NawwGame(object):
    def __init__(self,relationships, step_size=0.001):
        self.step_size = step_size
        n_players = len(relationships)
        t_vals = map(lambda x: x*2*m.pi/n_players, range(n_players))
        self.players = [ Player(r[0],Vector(m.sin(t), m.cos(t)),r[1],r[2]) for (r,t) in zip(relationships, t_vals) ]
        for player in self.players:
            player.na = filter(lambda p: p.name == player.na, self.players)[0]
            player.ww = filter(lambda p: p.name == player.ww, self.players)[0]

        self.init_show()
    
    def step(self):
        '''move each player closer to safety.
        If the projection of a player's position onto the line between 
        that player's na and ww puts them in safety, take a step there. Otherwise,
        move toward that player's ww.'''
        t_positions = []
        for player in self.players:
            ww_pos, na_pos = player.ww.pos, player.na.pos
            slope, offset = ww_pos.line(na_pos)
            #direction to line between 
            direction = player.pos.dir_to_line(slope,offset)
            proj = player.pos + direction
            # these are the t values such that, eg, na_pos == slope * t_na + offset
            t_na = (na_pos - offset) / slope 
            t_ww = (ww_pos - offset) / slope
            t_p = (proj - offset) / slope
            # proj is between na and ww iff (t_p - t_ww) * (t_na - t_ww) > 0
            # (i.e. when t_na and t_p are on the same side of t_ww )
            if (t_p - t_ww) * (t_na - t_ww) > 0:
                #we should move towards ww instead of towards the line
                direction = ww_pos - player.pos
            new_pos = player.pos + (direction.normalize() * self.step_size)
            t_positions.append(new_pos)

        for player, new_pos in zip(self.players,t_positions):
            player.pos = new_pos
    

    def init_show(self):
        print '''set terminal gif animate delay 4
set output "anim.gif"
set xrange [-2:2]
set yrange [-2:2]
set nokey
unset xtics
unset ytics
unset border'''

    def show(self):
        print "plot '-' with points"
        for player in self.players:
            print '{} {}'.format(player.pos.x, player.pos.y)
        print 'e'


if __name__=='__main__':
    relationships = [('a','b','c'),('b','c','d'),('c','b','a'),('d','b','a'),('e','a','d'), ('f','c','e')]
    game = NawwGame(relationships)
    for i in range(10000):
        game.show()
        game.step()

        


