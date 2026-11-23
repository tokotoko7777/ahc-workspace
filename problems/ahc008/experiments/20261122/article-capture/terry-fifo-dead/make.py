from pathlib import Path
import hashlib
D=Path(__file__).resolve().parent
p=D.parent/'ahc008_terry_virtual_chase_absorb/candidate.private.cpp'
b=p.read_bytes()
assert hashlib.sha256(b).hexdigest()=='a237f7986e03ba94e98cfb6f1d2059f50d87305f6c8a82b1fa6d4ccd5f013f22'
s=b.decode()
i=s.index('namespace exact_build {')
s=s[:i]+s[i:].replace('namespace exact_build {','''namespace exact_build {
inline int dead_t=-1;
inline std::vector<std::string> dead_rows;
''',1)
a=s.index('Turn act_literal')
b=s.index('    const auto adj=nb(p);',a)
part=s[a:b]
needle='     z.moves[g]=occupied[g]=1;z.action[i]=Move[direction(p,g)];z.booked[i]=1;'
assert part.count(needle)==1
part=part.replace(needle,needle+'''
     if(dead_t>=0){
      auto unbuilt=[&](const auto& job){
       unsigned bits=0;
       auto xy=nb(job.at);
       for(int k=0;k<4;++k)if((job.mask>>k&1)&&xy[k]>=0&&!wall[xy[k]])bits|=1u<<k;
       return bits;
      };
      if(!unbuilt(a)){
       std::string line=std::to_string(dead_t)+" "+std::to_string(i)+" "+
          std::to_string(a.at)+" "+std::to_string(a.mask)+" "+z.action[i];
       int next=-1,bits=0;
       for(auto it=std::next(q[i].begin());it!=q[i].end();++it)if((bits=unbuilt(*it))){
          next=it->at;break;
       }
       line+=" "+std::to_string(next)+" "+std::to_string(bits);
       dead_rows.push_back(std::move(line));
      }
     }
''')
s=s[:a]+part+s[b:]
start=s.index('int main(int argc')
x=s[start:]
n='  if(turn==275)d=decide(s,policy,cap,stop);'
assert x.count(n)==1
x=x.replace(n,'''
  auto path=std::getenv("AHC008_FIFO_DEAD_LOG");
  if(path&&turn==275){
   std::ofstream f(path);
   for(const auto& row:exact_build::dead_rows)f<<row<<'\\n';
   return f?0:3;
  }
  exact_build::dead_t=path?turn:-1;
'''+n)
(D/'observer.private.cpp').write_text(s[:start]+x)
