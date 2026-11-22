"""Pinned v24 map override for a hindsight-only diagnostic."""
import argparse
import hashlib
from pathlib import Path

P=Path(__file__).resolve().parent
BASE=P.parent/'ahc008_terry_same_geometry/hybrid/source-release/frozen.cpp'
SHA='7ae8fc4c25a5b69c448682b62c428fb73f83b7a2d826003f07b2ea173be06639'
def replace(s,key,value):
    if s.count(key)!=1:raise ValueError(f"Pinned anchor missing or ambiguous: {key!r}")
    return s.replace(key,value,1)
def main():
    p=argparse.ArgumentParser()
    p.add_argument('--v24',type=Path,default=BASE)
    p.add_argument('--output',type=Path,default=P/'submission.cpp')
    a=p.parse_args()
    raw=a.v24.read_bytes()
    if hashlib.sha256(raw).hexdigest()!=SHA:raise ValueError("Frozen v24 source hash mismatch")
    t=replace(raw.decode(),' void makeBanks(){',' int m5LayoutOverride=-1;\n void makeBanks(){')
    t=replace(t,'  bankRot=bankStarted; bankStarted=0;','''  if(m5LayoutOverride>=0&&humans.size()==5&&bankN==2){
   bankStarted=m5LayoutOverride/3;
   bankShift=offsets[m5LayoutOverride%3];
  }
  bankRot=bankStarted; bankStarted=0;''')
    t=replace(t,'''  if(v.starts_with(p))forced=clamp(atoi(argv[i]+p.size()),0,4);
 }
 g.terryLayout(forced<0?5:forced);''','''  if(v.starts_with(p))forced=clamp(atoi(argv[i]+p.size()),0,4);
  constexpr string_view geometry="--m5-layout=";
  if(v.starts_with(geometry)){
   auto k=v.substr(geometry.size());
   if(k.size()!=1||k[0]<'0'||k[0]>'5'||g.m5LayoutOverride>=0){
    cerr<<"Expected exactly one --m5-layout=0..5\\n";return 2;
   }
   g.m5LayoutOverride=k[0]-'0';
  }
 }
 if(g.m5LayoutOverride>=0&&(forced>=0||g.humans.size()!=5)){
  cerr<<"--m5-layout requires M=5 and cannot be combined with --terry-force-map\\n";
  return 2;
 }
 g.terryLayout(forced<0?5:forced);
 if(const char* path=getenv("AHC008_M5_LAYOUT_LOG");path&&forced<0&&g.humans.size()==5){
  ofstream layoutLog(path);
  int id=3*g.bankRot+(g.bankShift-4)/4;
  if(layoutLog)layoutLog<<"layout,"<<id<<",rotation,"<<g.bankRot<<",offset,"<<g.bankShift
                         <<",forced,"<<g.m5LayoutOverride<<'\\n';
 }''')
    a.output.write_text(t)
    print(hashlib.sha256(a.output.read_bytes()).hexdigest(),a.output)
if __name__=='__main__':
    main()
