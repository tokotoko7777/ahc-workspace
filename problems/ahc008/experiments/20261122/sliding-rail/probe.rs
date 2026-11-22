use std::{env,fs};
use tools::{parse_input,Input,Sim};
const S:usize=30; const V:usize=900;
fn id(p:(usize,usize))->usize{p.0*S+p.1}
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
const L:usize=10;
const P:usize=L+2;
const H:usize=6;
type Dist=[u16;V];
#[derive(Copy,Clone)]
struct Plan{w:[usize;L],v:[usize;P]}
fn choose(i:&Input)->Plan{
 let pets:Vec<_>=i.ps.iter().map(|&(x,y,t)|(id((x,y)),t)).collect();
 let hs:Vec<_>=i.hs.iter().copied().map(id).collect();
 let mut best=(-1e10,None);
 for side in 0..4{for c in 1..=S-L-1{
  let cell=|d,t|match side{0=>d*S+t,1=>(S-1-d)*S+t,2=>t*S+d,_=>t*S+S-1-d};
  let w=std::array::from_fn(|j|cell(1,c+j));
  let v=std::array::from_fn(|j|cell(0,c+j-1));
  if pets.iter().any(|&(p,_)|w.contains(&p))||hs.iter().any(|p|w.contains(p)){continue}
  let cost=w.iter().map(|&p|hs.iter().map(|&q|md(p,q)).min().unwrap()as f64).sum::<f64>();
  let station=hs.iter().enumerate().flat_map(|(a,&ha)|hs.iter().enumerate().filter(move |&(b,_)|b!=a).map(move |(_, &hb)| md(ha,v[0]).max(md(hb,v[P-1])))).min().unwrap();
  let near=pets.iter().filter(|p|(1..=3).contains(&p.1)).map(|&(p,t)|{
    let d=v[1..=L].iter().map(|&q|md(p,q)).min().unwrap();
    (1.0+0.13*(t-1)as f64)/(3.0+d as f64)
  }).sum::<f64>();
  let score=near*32.0-0.18*cost-0.30*station as f64;
  if score>best.0{best=(score,Some(Plan{w,v}));}
 }}
 best.1.unwrap()
}
fn walls(sim:&Sim)->[bool;V]{std::array::from_fn(|p|sim.blocked[p/S][p%S])}
fn emit(sim:&mut Sim,act:&[u8]){
 sim.human_move(std::str::from_utf8(act).unwrap()).unwrap();
 sim.pet_move();
}
struct Forecast {
 t: [[[f64;P];P];3],
 ad:Vec<f64>,
 st:Vec<f64>,
}
fn to(a:usize,b:usize)->usize{if a<b{a+1}else if a>b{a-1}else{a}}
fn legal(a:usize,b:usize,p:usize)->bool{a<=L-6&&b==a+7&&(p==a+3||p==a+4)}
fn actions(a:usize,b:usize)->impl Iterator<Item=(usize,usize)>{
 let lo=a.saturating_sub(1);let hi=(a+1).min(P-1);
 let l=b.saturating_sub(1);let h=(b+1).min(P-1);
 (lo..=hi).flat_map(move|x|(l.max(x+1)..=h).map(move|y|(x,y)))
}
impl Forecast {
 fn ia(t:usize,k:usize,a:usize,b:usize,p:usize)->usize{(((t*3+k)*P+a)*P+b)*P+p}
 fn is(t:usize,k:usize,a:usize,b:usize,p:usize,g:usize)->usize{Self::ia(t,k,a,b,p)*(L-5)+g}
 fn new()->Self{
  let mut out=Self{t:[[[0.;P];P];3],ad:vec![0.;(H+1)*3*P*P*P],st:vec![0.;(H+1)*3*P*P*P*(L-5)]};
  for p in 1..=L{
   let mut s=[0.;P];s[p]=1.;
   for k in 0..3{
    let mut r=[0.;P];
    for x in 1..=L{for v in [x-1,x+1]{if v>0&&v<=L{r[v]+=s[x]*0.5;}}}
    out.t[k][p]=r;s=r;
   }
  }
  for h in 0..=H{for k in 0..3{for a in 0..P{for b in a+1..P{for p in 1..=L{
   let near=legal(a,b,p);
   let ia=Self::ia(h,k,a,b,p);
   out.ad[ia]=if near{1.}else if h==0{0.}else{
    actions(a,b).map(|(x,y)|(1..=L).map(|q|out.t[k][p][q]*out.ad[Self::ia(h-1,k,x,y,q)]).sum::<f64>()).fold(0.,f64::max)
   };
   for g in 0..L-5{
    out.st[Self::is(h,k,a,b,p,g)]=if near&&a==g{1.}else if h==0{0.}else{
     let (x,y)=(to(a,g),to(b,g+7));
     (1..=L).map(|q|out.t[k][p][q]*out.st[Self::is(h-1,k,x,y,q,g)]).sum()
    };
   }
  }}}}}
  out
 }
}
#[derive(Default,Copy,Clone)]
struct ResultRow{prep:usize,stage:usize,staff_wait:usize,anchor:usize,
 ready:usize,staff:usize,prep_arrivals:u32,staff_arrivals:u32,joined:u32,
 gfix:usize,move_count:usize,wait_count:usize,build_count:usize,
 closed:usize,cow:usize,pig:usize,rab:usize,other:usize,first_legal:usize,arrival_comparison:u64}
fn mask_kind(i:&Input,ps:&[usize],p:&Plan)->u32{
 ps.iter().enumerate().filter(|&(j,x)|(1..=3).contains(&i.ps[j].2)&&p.v[1..=L].contains(x)).fold(0,|m,(j,_)|m|(1<<j))
}
fn run(i:&Input,p:Plan,f:&Forecast,adaptive:bool,limit:usize,prebuilt:bool)->ResultRow{
 let mut out=ResultRow::default();
 let mut s=Sim::new(i);let m=s.hs.len();
 if prebuilt{for x in p.w {s.blocked[x/S][x%S]=true;}}
 let mut tr=None;let mut assistants=None;
 for t in 0..limit+H+1{
  let b=walls(&s);let hs:Vec<_>=s.hs.iter().copied().map(id).collect();
  let ps:Vec<_>=s.ps.iter().map(|&(x,y,_)|id((x,y))).collect();
  let mut ban=[false;V];for &x in &ps{ban[x]=true;for z in nbs(x){if z<V{ban[z]=true;}}}for &x in &hs{ban[x]=true;}
  let mut action=vec![b'.';m];
  let arrivals=mask_kind(i,&ps,&p);
  if !p.w.iter().all(|&x|b[x]){
   if t==limit{break}
   out.prep+=1;out.prep_arrivals|=arrivals;
   let mut route=b;for x in p.w{route[x]=true;}
   // Avoid staging builders inside the future rail unless already on the rail.
   let ds:Vec<Dist>=hs.iter().map(|&x|bfs(&route,x)).collect();
   let mut jobs=Vec::new();
   for k in 0..L{let w=p.w[k];if b[w]{continue}
    for q in nbs(w){if q>=V||route[q]{continue}
     let inside=p.v[1..=L].contains(&q);
     for x in 0..m{let d=ds[x][q];if d==u16::MAX||ban[w]&&d==0{continue}
      jobs.push((d as i32*8+if inside{15}else{0}+if ban[w]{5}else{0},x,k,q));
     }
    }
   }
   jobs.sort_unstable();
   let mut used=[false;L];let mut work=vec![None;m];
   for (_,x,k,q) in jobs{if work[x].is_none()&&!used[k]{work[x]=Some((k,q));used[k]=true;}}
   for x in 0..m{if let Some((k,q))=work[x]{
    if hs[x]==q{action[x]=dchar(q,p.w[k]).to_ascii_lowercase();}
    else if let Some(q2)=mv(hs[x],&bfs(&route,q),&route){action[x]=dchar(hs[x],q2);}
   }}
  } else {
   out.ready=1;
   let la=bfs(&b,p.v[0]);let lb=bfs(&b,p.v[P-1]);
   let (ga,gb)=*assistants.get_or_insert_with(||{
    let mut best=(u16::MAX, u32::MAX,0,1);
    for a in 0..m{for c in 0..m{if a!=c{
     let (da,db)=(la[hs[a]],lb[hs[c]]);
     best=best.min((da.max(db),da as u32+db as u32,a,c));
    }}}
    (best.2,best.3)
   });
   let extra=(0..m).any(|k|k!=ga&&k!=gb&&p.v[1..=L].contains(&hs[k]));
   if tr.is_none()&&(hs[ga]!=p.v[0]||hs[gb]!=p.v[P-1]||extra){
    if t==limit{break}
    out.stage+=1;out.prep_arrivals|=arrivals;
    for x in 0..m{
     let d=if x==ga{Some(&la)}else if x==gb{Some(&lb)}
           else if p.v[1..=L].contains(&hs[x]){
             if la[hs[x]]<=lb[hs[x]]{Some(&la)}else{Some(&lb)}
           }else{None};
     if let Some(d)=d{if let Some(z)=mv(hs[x],d,&b){action[x]=dchar(hs[x],z);}}
    }
   } else {
    out.staff=1;out.staff_arrivals|=arrivals;
    let observed:Vec<_>=ps.iter().enumerate().filter_map(|(j,&q)|{
     let typ=i.ps[j].2; if !(1..=3).contains(&typ){return None}
     p.v[1..=L].iter().position(|&z|z==q).map(|k|(j,typ-1,k+1))
    }).collect();
    if tr.is_none(){
     if t>=limit{break}
     if observed.is_empty(){out.staff_wait+=1;}
     else {
      let mut best=(-1.,0);
      for g in 0..L-5 {
       let v=observed.iter().map(|&(_,k,q)|f.st[Forecast::is(H,k,0,P-1,q,g)]).sum::<f64>();
       if v>best.0{best=(v,g);}
      }
      out.anchor=t+1;out.gfix=best.1+1;out.joined=arrivals;
      let mut check=0_u64;for q in hs.iter().chain(ps.iter()){check=check.wrapping_mul(131).wrapping_add(*q as u64+1);}
      out.arrival_comparison=check;
      tr=Some((t,best.1));
     }
    }
    if let Some((started,g))=tr {
     let pa=p.v.iter().position(|x|*x==hs[ga]).unwrap();let pb=p.v.iter().position(|x|*x==hs[gb]).unwrap();
     assert!(pa<pb);
     if (adaptive||pa==g)&&pb==pa+7&&pa<=L-6&&!ban[p.v[pa+1]]&&!ban[p.v[pb-1]]&&!extra&&observed.iter().any(|&(_,_,q)|legal(pa,pb,q)){
      out.first_legal=t+1;
      action[ga]=dchar(hs[ga],p.v[pa+1]).to_ascii_lowercase();
      action[gb]=dchar(hs[gb],p.v[pb-1]).to_ascii_lowercase();
      out.closed=1;
      for (j,&q) in ps.iter().enumerate(){if p.v[pa+2..pb-1].contains(&q){
       match i.ps[j].2{1=>out.cow+=1,2=>out.pig+=1,3=>out.rab+=1,_=>out.other+=1}
      }}
     }else if t>=started+H{break}
     else {
      let remaining=H-(t-started);
      let (na,nb)=if adaptive{
       let mut best=(-1.,pa,pb);
       for (a1,b1) in actions(pa,pb){
        let v=observed.iter().map(|&(_,k,q)|{
         (1..=L).map(|r|f.t[k][q][r]*f.ad[Forecast::ia(remaining-1,k,a1,b1,r)]).sum::<f64>()
        }).sum::<f64>();
        if v>best.0||v==best.0&&a1==pa&&b1==pb{best=(v,a1,b1);}
       }
       (best.1,best.2)
      }else{(to(pa,g),to(pb,g+7))};
      if na!=pa{action[ga]=dchar(hs[ga],p.v[na]);}
      if nb!=pb{action[gb]=dchar(hs[gb],p.v[nb]);}
     }
    }
   }
  }
  for &z in &action{if z==b'.'{out.wait_count+=1}else if z.is_ascii_lowercase(){out.build_count+=1}else{out.move_count+=1}}
  emit(&mut s,&action);
  if out.closed==1{
    let grid=walls(&s);
    let a=p.v.iter().position(|x|*x==hs[assistants.unwrap().0]).unwrap();
    let inside=p.v[a+2]; let area=bfs(&grid,inside);
    assert_eq!(area.iter().filter(|&&d|d!=u16::MAX).count(),4);
    assert!(s.hs.iter().all(|&h|area[id(h)]==u16::MAX));
    let survivors=s.ps.iter().enumerate().filter(|&(_,&(x,y,_))|area[id((x,y))]!=u16::MAX).count();
    assert_eq!(survivors,out.cow+out.pig+out.rab+out.other);
    break
  }
 }
 out
}
fn main(){
 let args:Vec<_>=env::args().collect();
 let limit=args[2].parse().unwrap();
 let mut paths:Vec<_>=fs::read_dir(&args[1]).unwrap().map(|p|p.unwrap().path()).filter(|p|p.extension().is_some_and(|x|x=="txt")).collect();
 paths.sort();
 let f=Forecast::new();
 println!("case,prebuilt,adaptive,prep,stage,staff_wait,anchor,ready,staff,prep_arrivals,staff_arrivals,at_anchor,fixed_window,moves,waits,builds,closed,cow,pig,rabbit,other,first_legal,anchor_hash");
 for x in paths{
  let i=parse_input(&fs::read_to_string(&x).unwrap());let p=choose(&i);
  for prebuilt in [false,true] {if prebuilt&&args.len()<4{continue}
   let a=run(&i,p,&f,false,limit,prebuilt);let b=run(&i,p,&f,true,limit,prebuilt);
   assert_eq!((a.anchor,a.arrival_comparison,a.gfix,a.prep,a.stage,a.staff_wait),(b.anchor,b.arrival_comparison,b.gfix,b.prep,b.stage,b.staff_wait));
   for (ad,row) in [(0,a),(1,b)]{
    println!("{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}",x.file_name().unwrap().to_string_lossy(),prebuilt as u8,ad,row.prep,row.stage,row.staff_wait,row.anchor,row.ready,row.staff,row.prep_arrivals.count_ones(),row.staff_arrivals.count_ones(),row.joined.count_ones(),row.gfix,row.move_count,row.wait_count,row.build_count,row.closed,row.cow,row.pig,row.rab,row.other,row.first_legal,row.arrival_comparison);
   }
  }
 }
}
