from pathlib import Path
import hashlib
D=Path(__file__).resolve().parent
S=D.parent/'ahc008_terry_virtual_chase_absorb/candidate.private.cpp'
b=S.read_bytes()
assert hashlib.sha256(b).hexdigest()=='a237f7986e03ba94e98cfb6f1d2059f50d87305f6c8a82b1fa6d4ccd5f013f22'
s=b.decode()
assert s.count('\nint main(')==1
(D/'observer.private.cpp').write_text(s.split('\nint main(',1)[0]+'\n#include "keyed.hpp"\n'+(D/'observer.inc').read_text())
