import csv,hashlib,json
from pathlib import Path
D=Path(__file__).resolve().parent;W=D.parent
A=W/'ahc008_terry_virtual_chase_absorb'; O=W/'ahc008_terry_oversized_chase'
def sh(b):return hashlib.sha256(b).hexdigest()
panels={
    'practice':('0bd17793a6742076fc9a8aecb755162e87560cea1a83daa152e602751a1615e0','ahc008_focus_assets/tools/in'),
    'fresh':('f76ed91d684707dcfd4d4f34052e6fa748933630f2c8d8279391bec9ccfe5911','ahc008_terry_first_chase_reuse/default-fresh100/in')}
def main():
    out={};pin={}
    for panel,(expected,idir) in panels.items():
        p=A/f'{panel}.native.csv'; raw=p.read_bytes();assert sh(raw)==expected
        pin[str(p.relative_to(W))]=expected
        official={r['case']:r for r in csv.DictReader(raw.decode().splitlines()) if r['arm']=='terry_independent'}
        assert len(official)==100
        for stratum in ('map1','late'):
            p=O/f'{panel}-{stratum}'/'recordings.json';raw=p.read_bytes()
            pin[str(p.relative_to(W))]=sh(raw)
            for r in json.loads(raw):
                if int(r['pairs'])<=0:continue
                c=r['case']; src=official[c];key=panel+'/'+c
                mark=list(map(int,src['telemetry'].split()))
                assert len(mark)==15 and int(r['pairs'])==mark[10] and r['mode']==mark[13]
                assert r['map']==mark[0] and r['score']==int(src['score'])
                assert r['input_sha']==src['input_sha'] and r['commands_sha']==src['commands_sha']
                inp=W/idir/c;action=(p.parent/c).with_suffix('.actions');t=action.with_suffix('.trace')
                a=action.read_bytes(); b=t.read_bytes()
                assert sh(inp.read_bytes())==r['input_sha'] and sh(a)==r['commands_sha'] and sh(b)==r['trace_sha']
                assert len(a.splitlines())==300 and len(b.splitlines())==600 and b.splitlines()[::2]==a.splitlines()
                q=dict(panel=panel,case=c,input=str(inp.relative_to(W)),action=str(action.relative_to(W)),
                       trace=str(t.relative_to(W)),input_sha=r['input_sha'],action_sha=r['commands_sha'],trace_sha=r['trace_sha'],
                       cap=mark[10],normal=mark[11],chase=mark[12],map=mark[0],native_mode=mark[13],strata=[stratum])
                if key in out:
                    assert q['trace_sha']==out[key]['trace_sha']
                    out[key]['strata'].append(stratum)
                else:out[key]=q
    assert len(out)>=20
    (D/'FIXTURES.json').write_text(json.dumps(dict(pins=pin,rows=list(out.values())),indent=2)+'\n')
if __name__=='__main__':main()
