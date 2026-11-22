"""Conditional capture after observed entry into a short room corner.

This tests a local mechanism; it cannot estimate how often pets actually
approach entrances in the 30x30 match. The guard needs a specified number
of turns to reach the stance. Escape through the entry is absorbing.
"""
import csv
import sys
from collections import defaultdict

STEPS = [(1,0),(-1,0),(0,1),(0,-1)]

def case(width, depth, speed, remaining, guard_delay, horizon):
    room = {(x,y) for x in range(width) for y in range(1,depth+1)}
    gate=(0,0)
    escape=(0,-1)
    tiles=room|{gate,escape}
    edges={q:[(q[0]+i,q[1]+j) for i,j in STEPS if (q[0]+i,q[1]+j) in tiles]
           for q in tiles}
    d={(0,1):1.}
    wins, escaped = 0.,0.
    def advance(d,n):
        nonlocal escaped
        for _ in range(n):
            nd=defaultdict(float)
            for q, v in d.items():
                for x in edges[q]:
                    prob=v/len(edges[q])
                    if x==escape: escaped+=prob
                    else: nd[x]+=prob
            d=nd
        return d
    d=advance(d,remaining)
    for t in range(horizon+1):
        if t>=guard_delay:
            for q in list(d):
                if q in room and abs(q[0])+q[1]>=2:
                    wins+=d.pop(q)
        if t<horizon:d=advance(d,speed)
    assert abs(wins+escaped+sum(d.values())-1)<1e-8
    return wins,escaped

def main():
    writer=csv.writer(sys.stdout,lineterminator="\n")
    writer.writerow(["width","depth","model","steps_left_after_entry","guard_delay_turns","deadline_turns","capture","escape"])
    for w in (1,2,3):
        for depth in (2,3,4):
            for name,k in (("cow",1),("pig",2),("rabbit",3)):
                for remaining in range(k):
                    for guard in (0,1,2):
                        for h in (1,2,4,8):
                            if guard>h:continue
                            a,b=case(w,depth,k,remaining,guard,h)
                            writer.writerow([w,depth,name,remaining,guard,h,f"{a:.8f}",f"{b:.8f}"])
if __name__=="__main__":main()
