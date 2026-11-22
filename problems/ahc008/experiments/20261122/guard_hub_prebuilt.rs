// Counterfactual test using unmodified official v3 Sim transitions.
// Fixed minimum-cost hub selected lexicographically before inspecting outcomes.
use std::{env,fs,path::Path};
use tools::{parse_input,Sim};
type Pos=(usize,usize);
const Q:Pos=(3,6);
const GATES:[Pos;3]=[(2,6),(3,5),(4,6)];
const ACTIONS:[u8;3]=[b'u',b'l',b'd'];
const INNER:[[Pos;3];3]=[[(1,6),(0,5),(0,6)],[(4,4),(5,4),(3,4)],[(6,6),(5,6),(6,5)]];
const WALL:[Pos;15]=[(0,7),(1,7),(0,4),(1,5),(2,4),(5,5),(4,5),(3,3),(4,3),(5,3),(6,4),(6,7),(7,6),(7,5),(5,7)];
fn md(a:Pos,b:Pos)->usize {a.0.abs_diff(b.0)+a.1.abs_diff(b.1)}
fn main() {
    let path=env::args().nth(1).expect("case directory");
    let mut files:Vec<_>=fs::read_dir(Path::new(&path)).unwrap().map(|x|x.unwrap().path()).filter(|x|x.extension().map_or(false,|e|e=="txt")).collect();
    files.sort();
    println!("case,feasible_prebuilt,guard_optimistic_walk,arrival40_none,arrival40_guard,caught10,caught20,caught40");
    for f in files {
        let input=parse_input(&fs::read_to_string(&f).unwrap());
        let mut sim=Sim::new(&input);
        let name=f.file_name().unwrap().to_string_lossy();
        if sim.ps.iter().any(|&(r,c,_)|WALL.contains(&(r,c))) || sim.hs.iter().any(|p|WALL.contains(p)|| GATES.contains(p)||INNER.iter().flatten().any(|x|x==p)) {
            println!("{},0,-1,0,0,0,0,0",name);continue;
        }
        for (r,c) in WALL {sim.blocked[r][c]=true;}
        let distances=tools::bfs(&sim.blocked,Q.0,Q.1);
        let (guard,walk)=sim.hs.iter().enumerate().map(|(i,&(r,c))|(i,distances[r][c])).min_by_key(|x|x.1).unwrap();
        let mut summary=vec![0_usize;5];
        for active in [false,true] {
            let mut sim=Sim::new(&input);
            for (r,c) in WALL {sim.blocked[r][c]=true;}
            sim.hs[guard]=Q;
            let mut arrived=0_u32;let mut captured=0_u32;let mut used=[false;3];
            for t in 1..=40 {
                let mut letters=vec![b'.';sim.hs.len()];
                let mut best=(0,0);
                for a in 0..3 {
                    let mask=sim.ps.iter().enumerate().filter(|(_,p)|INNER[a].contains(&(p.0,p.1))).fold(0_u32,|m,(i,_)|m|(1<<i));
                    arrived|=mask;
                    if !active || used[a] || mask==0 || sim.ps.iter().any(|&(r,c,_)|md((r,c),GATES[a])<=1) {continue;}
                    if (mask&!captured).count_ones()>(best.0 as u32&!captured).count_ones(){best=(mask,a);}
                }
                if best.0!=0 {letters[guard]=ACTIONS[best.1];captured|=best.0;used[best.1]=true;}
                let s=std::str::from_utf8(&letters).unwrap();
                sim.human_move(s).unwrap();
                sim.pet_move();
                if active {for (k,j) in [10,20,40].iter().enumerate() {if t==*j {summary[2+k]=captured.count_ones() as usize;}}}
            }
            summary[active as usize]=arrived.count_ones() as usize;
        }
        println!("{},1,{},{},{},{},{},{}",name,walk,summary[0],summary[1],summary[2],summary[3],summary[4]);
    }
}
