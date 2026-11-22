use std::{env,fs};
use tools::{parse_input,Input,Sim};
const S:usize=30; const V:usize=900;
fn id(p:(usize,usize))->usize{p.0*S+p.1}
fn xy(p:usize)->(usize,usize){(p/S,p%S)}
fn md(p:usize,q:usize)->usize{(p/S).abs_diff(q/S)+(p%S).abs_diff(q%S)}
fn nbs(p:usize)->[usize;4]{[if p>=S{p-S}else{V},if p+S<V{p+S}else{V},if p%S>0{p-1}else{V},if p%S+1<S{p+1}else{V}]}
fn dchar(p:usize,q:usize)->u8{b"UDLR"[nbs(p).iter().position(|&x|x==q).unwrap()]}
fn bfs(block:&[bool;V],p:usize)->[u16;V]{
    let mut d=[u16::MAX;V];d[p]=0;
    let mut q=[0;V];q[0]=p;let mut head=0;let mut tail=1;
    while head<tail{
        let u=q[head];head+=1;
        for z in nbs(u){if z<V&&!block[z]&&d[z]==u16::MAX{
            d[z]=d[u]+1;q[tail]=z;tail+=1;
        }}
    }
    d
}
fn mv(p:usize,d:&[u16;V],b:&[bool;V])->Option<usize>{
 nbs(p).into_iter().find(|&q|q<V&&!b[q]&&d[q]<d[p])
}
#[derive(Copy,Clone)]
struct Plan{wall:[usize;6],lane:[usize;6],a:usize,b:usize}
fn choose(i:&Input)->Plan{
 let pets:Vec<_>=i.ps.iter().map(|&(x,y,t)|(id((x,y)),t)).collect();
 let humans:Vec<_>=i.hs.iter().copied().map(id).collect();
 let mut best=(-1e30,None);
 for side in 0..4{for c in 2..=22{for rev in [false,true]{
    let cell=|depth:usize,t:usize| -> usize{match side{0=>depth*S+t,1=>(S-1-depth)*S+t,2=>t*S+depth,_=>t*S+S-1-depth}};
    let wall=std::array::from_fn(|j|cell(1,c+j));
    let mut lane=std::array::from_fn(|j|cell(0,c+j));
    let mut a=cell(0,c-1);let mut b=cell(0,c+6);
    if rev{std::mem::swap(&mut a,&mut b);lane.reverse();}
    if pets.iter().any(|&(p,_)|wall.contains(&p))||humans.iter().any(|p|wall.contains(p)){continue}
    let build=wall.iter().map(|&p|humans.iter().map(|&h|md(h,p)).min().unwrap() as f64).sum::<f64>();
    let far=humans.iter().map(|&p|md(p,b) as f64).sum::<f64>()/humans.len() as f64;
    let a_stance=humans.iter().map(|&p|md(p,a)).min().unwrap() as f64;
    let mut dogs=0.;
    for &(p,t) in &pets{if t!=4{continue}
      let dp=md(p,a) as f64;
      let side=if md(p,a)+2 < md(p,b){1.0}else if lane.contains(&p){0.6}else{0.15};
      dogs+=side/(6.+dp);
    }
    let val=dogs*280.0-build*0.14-far*0.30-a_stance*0.22;
    if val>best.0{best=(val,Some(Plan{wall,lane,a,b}));}
 }}}
 best.1.unwrap()
}
fn walls(sim:&Sim)->[bool;V]{std::array::from_fn(|i|sim.blocked[i/S][i%S])}
#[derive(Default)]
struct Counts { ready:usize, guard:usize, staffed:usize, near:usize, far_known:usize, route_near:usize, enter:usize, opportunity:usize,dogcatch:usize, othercatch:usize, residual:usize, largest_dogs:usize,legal_fail:usize, preliminary_wall_turns:usize,empty_attempts:usize,two_core_turns:usize,two_legal_turns:usize,first_single_legal:usize }
fn planned(input:&Input, plan:Plan,mode:usize,turns:usize)->Counts{
 let mut sim=Sim::new(input); let mut result=Counts::default();
 let dogs:Vec<_>=input.ps.iter().enumerate().filter(|(_,p)| p.2==4).map(|(j,_)|j).collect();
 assert_eq!(dogs.len(),2);
 let nh=sim.hs.len();let nd=dogs.len();
 let mut filter=vec![vec![0.;nh+1];nd];for f in &mut filter{f[nh]=1.}
 let mut a=plan.a;let mut b=plan.b;let mut lane=plan.lane;
 let mut near_guard=0;
 let mut role_set=false;
 let inward=&plan.lane[1..5];
 let core=&plan.lane[2..4];
 let mut first_ready=0;
 if mode==1||mode==2{for p in plan.wall {sim.blocked[p/S][p%S]=true;}}
 if mode==2{
  let orig:Vec<_>=sim.hs.iter().copied().map(id).collect();
  near_guard=(0..nh).min_by_key(|&j| md(orig[j],a) as i32-md(orig[j],b) as i32).unwrap();
  for j in 0..nh{sim.hs[j]=xy(if j==near_guard{a}else{b});}
  role_set=true;
 }
 let mut cap=false;let mut seen=[false;20];let mut seen_route=[false;20];
 let mut held_far=0;
 for t in 0..turns{
  let blocked=walls(&sim);let hs:Vec<_>=sim.hs.iter().copied().map(id).collect();
  let ps:Vec<_>=sim.ps.iter().map(|&(x,y,_)|id((x,y))).collect();
  let made=plan.wall.iter().filter(|&&p|blocked[p]).count();
  let ready=made==6;
  let mut action=vec![b'.';nh];
  let mut will=[false;V];
  let mut forbid=[false;V];
  for &p in &ps{forbid[p]=true;for q in nbs(p){if q<V{forbid[q]=true;}}}
  for &p in &hs{forbid[p]=true;}
  if ready{
   result.ready+=1;
   if first_ready==0{first_ready=t+1}
   if !role_set{
    let bd=bfs(&blocked,b);let ad=bfs(&blocked,a);
    near_guard=(0..nh).min_by_key(|&j|ad[hs[j]] as i32-bd[hs[j]] as i32).unwrap();
    role_set=true;
   }
   let mut far_count=hs.iter().enumerate().filter(|&(j,&p)|j!=near_guard&&p==b).count();
   let guard=hs[near_guard]==a&&far_count>0;
   if guard{result.guard+=1;}
   let staffed=guard&&far_count==nh-1;
   if staffed{result.staffed+=1;held_far+=1;}
   if guard{
    let db=bfs(&blocked,b);
    for (kk,&j) in dogs.iter().enumerate(){
      if inward.contains(&ps[j]){seen[j]=true;}
      if !inward.contains(&ps[j])&&db[ps[j]]>db[a]&&md(ps[j],a)<=3 {
          result.near+=1;
          let de=bfs(&blocked,lane[0]);
          if db[ps[j]]==de[ps[j]].saturating_add(db[lane[0]]) {
              result.route_near+=1;
              seen_route[j]=true;
          }
          let far_mass=(0..nh).filter(|&k|k!=near_guard&&hs[k]==b).map(|k|filter[kk][k]).sum::<f64>();
          if far_mass>0.7{result.far_known+=1;}
      }
    }
   }
   let can=guard&&!forbid[lane[0]]&&!forbid[lane[5]]&&hs.iter().all(|p|!inward.contains(p));
   let inside_dogs=dogs.iter().filter(|&&j|core.contains(&ps[j])).count();
   result.two_core_turns+=usize::from(inside_dogs==2);
   result.two_legal_turns+=usize::from(can&&inside_dogs==2);
   if can&&inside_dogs>=1&&result.first_single_legal==0 {result.first_single_legal=t+1;}
   if can&&inside_dogs>=if mode==4{2}else{1} {
     result.opportunity+=1;
     action[near_guard]=dchar(a,lane[0]).to_ascii_lowercase();
     let fg=(0..nh).find(|&j|j!=near_guard&&hs[j]==b).unwrap();
     action[fg]=dchar(b,lane[5]).to_ascii_lowercase();
     result.dogcatch=inside_dogs;
     result.othercatch=ps.iter().filter(|p|inward.contains(p)).count()-inside_dogs;
     cap=true;
   } else {
    // Mode 3 re-baits the opposite side after a fixed long, unproductive dwell.
    // This still observes only player-visible state. Roles swap via outside walk
    // if the near-side guard would otherwise block an early gate.
    if mode==3&&staffed&&held_far>=24 {
      std::mem::swap(&mut a,&mut b);lane.reverse();held_far=0;
      let dd=bfs(&blocked,a);near_guard=(0..nh).min_by_key(|&j|dd[hs[j]]).unwrap();far_count=0;
      result.empty_attempts+=1;
    }
    let da=bfs(&blocked,a);let db=bfs(&blocked,b);
    for j in 0..nh {
     let (dst,d)=if j==near_guard{(a,&da)}else{(b,&db)};
     if hs[j]==dst {continue}
     if let Some(q)=mv(hs[j],d,&blocked){action[j]=dchar(hs[j],q);}
    }
    let _=far_count;
   }
  } else{
   result.preliminary_wall_turns+=1;
   let mut assigned=[false;V];let mut candidates=Vec::new();
   let mut dist=Vec::new();
   let mut travel=blocked;for &w in &plan.wall{travel[w]=true;}
   for &h in &hs{dist.push(bfs(&travel,h));}
   for &p in &plan.wall{if blocked[p]{continue}
    for q in nbs(p){if q>=V||travel[q]{continue}
     let shade=if inward.contains(&q){2}else{0};
     for j in 0..nh{let d=dist[j][q];if d==u16::MAX||forbid[p]&&d==0{continue}
      candidates.push((i32::from(d)*8+shade+if forbid[p]{4}else{0}, j,p,q));
     }
    }
   }
   candidates.sort_unstable();
   let mut targets=vec![None;nh];
   for (_,j,p,q) in candidates {if targets[j].is_none()&&!assigned[p]{targets[j]=Some((p,q));assigned[p]=true;}}
   for (j,&h) in hs.iter().enumerate(){if let Some((p,q))=targets[j]{if h==q{action[j]=dchar(h,p).to_ascii_lowercase();will[p]=true;}}}
   for (j,&h) in hs.iter().enumerate(){if action[j]!=b'.'{continue}
    let q=targets[j].map_or(b,|(_,q)|q);
    let d=bfs(&travel,q);
    if let Some(v)=mv(h,&d,&travel){if !will[v]{action[j]=dchar(h,v);}}
   }
  }
  if sim.human_move(std::str::from_utf8(&action).unwrap()).is_err(){result.legal_fail+=1;break}
  let replies=sim.pet_move();
  if cap {
   let bw=walls(&sim);
   let d=bfs(&bw,inward[0]);
   assert_eq!(d.iter().filter(|&&x|x!=u16::MAX).count(),4);
   assert!(sim.hs.iter().all(|&p|d[id(p)]==u16::MAX));
   let alive=dogs.iter().filter(|&&j|{let p=sim.ps[j];d[id((p.0,p.1))]!=u16::MAX}).count();
   assert_eq!(alive,result.dogcatch);
   break
  }
  let blocked=walls(&sim);
  let hs:Vec<_>=sim.hs.iter().copied().map(id).collect();
  let dists:Vec<_>=hs.iter().map(|&p|bfs(&blocked,p)).collect();
  let replies:Vec<_>=replies.split_whitespace().collect();
  for (kk,&j) in dogs.iter().enumerate(){
   let observed=replies[j].as_bytes();
   let s=ps[j];
   let d=b"UDLR".iter().position(|&x|x==observed[0]).unwrap();
   let begin=nbs(s)[d]; let end=id((sim.ps[j].0,sim.ps[j].1));
   let f=&mut filter[kk];
   if observed.len()!=2{f.fill(0.);f[nh]=1.;continue}
   let mut renew=f[nh];let mut eligible=0;
   for h in 0..nh {if dists[h][s]==u16::MAX||dists[h][s]==0{renew+=f[h];f[h]=0.;}else{eligible+=1;}}
   f[nh]=0.;
   for h in 0..nh {let dist=&dists[h];let v=dist[s];
    if v!=u16::MAX&&v!=0&&dist[begin]+1==v{
      let count=nbs(s).into_iter().filter(|&p|p<V&&dist[p]==v-1).count();
      f[h]=(f[h]+renew/f64::from(eligible))/count as f64;
    } else {f[h]=0.;}
    if hs[h]==begin||hs[h]==end{f[nh]+=f[h];f[h]=0.;}
   }
   let total:f64=f.iter().sum();
   if total>0.{for x in f{*x/=total;}}else{f.fill(0.);f[nh]=1.;}
  }
 }
 result.enter=seen.into_iter().filter(|&x|x).count();
 let _=seen_route;
 let blocked=walls(&sim);
 let biggest=sim.hs.iter().map(|&h|bfs(&blocked,id(h))).max_by_key(|d|d.iter().filter(|&&x|x!=u16::MAX).count()).unwrap();
 result.residual=sim.ps.iter().filter(|p|biggest[id((p.0,p.1))]!=u16::MAX).count();
 result.largest_dogs=dogs.iter().filter(|&&j|{let p=sim.ps[j];biggest[id((p.0,p.1))]!=u16::MAX}).count();
 result
}
fn main(){
 let args:Vec<String>=env::args().collect();
 let turns=args[2].parse().unwrap();
 let mut paths:Vec<_>=fs::read_dir(&args[1]).unwrap().map(|p|p.unwrap().path()).filter(|p|p.extension().is_some_and(|x|x=="txt")).collect();
 paths.sort();
 println!("case,mode,dogs,ready,guard,staffed,near,far_known,route_near,entered,closure_turns,caught_dog,caught_other,residual,largest_dogs,wall_turns,illegal,rebaits,two_core_turns,two_legal_turns,first_single_legal");
 for path in paths{
  let input=parse_input(&fs::read_to_string(&path).unwrap());
  let nd=input.ps.iter().filter(|x|x.2==4).count();
  for mode in args[3..].iter().map(|s|s.parse().unwrap()){
   let c=planned(&input,choose(&input),mode,turns);
   println!("{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}",path.file_name().unwrap().to_string_lossy(),mode,nd,c.ready,c.guard,c.staffed,c.near,c.far_known,c.route_near,c.enter,c.opportunity,c.dogcatch,c.othercatch,c.residual,c.largest_dogs,c.preliminary_wall_turns,c.legal_fail,c.empty_attempts,c.two_core_turns,c.two_legal_turns,c.first_single_legal);
  }
 }
}
