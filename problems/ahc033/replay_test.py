from benchmark import score

inp=("5\n"+" ".join(map(str,range(25)))+"\n").encode()
assert score(inp,b"D\nD\nB\nB\nB\n")["score"]==25000001 # move into a vacated cell
for out in (b"D\nU\nB\nB\nB\n", # swaps
            b"D\n.\nB\nB\nB\n", # same destination
            b"B\nPD\nB\nB\nB\n", # small crane carrying into occupied entrance
            b"B\n.PB\nB\nB\nB\n"): # destroy while holding
    try:
        score(inp,out)
    except ValueError:
        continue
    raise AssertionError("accepted invalid synchronous output")
# Ship one correct box, and a small crane uses the next input after leaving.
good=b"PRRRRQ\nPRQ\nB\nB\nB\n"
assert score(inp,good)=={"score":24000006,"turns":6,"inversions":0,"wrong":0,"missing":24}
