"""Compare chosen last walls to initial fixed-door choice, with no future input."""
import csv,sys,os,tempfile,subprocess
from pathlib import Path
pilot,casedir,table=map(Path,sys.argv[1:])
w=csv.writer(sys.stdout,lineterminator='\n')
w.writerow(['case','initial_gate','actual_final_walls','initial_gate_was_final'])
for r in csv.DictReader(table.open()):
    if r['model']!='deferred-square' or int(r['seal'])<0:continue
    payload=(casedir/r['case']).read_bytes().splitlines()
    with tempfile.TemporaryDirectory() as d:
        path=Path(d)/'meta'
        env=os.environ|{'AHC008_QUAD_LOG':str(path)}
        subprocess.run([pilot.resolve(),'2','27','0'],input=b'\n'.join(payload[:-1])+b'\n',
                       stdout=subprocess.DEVNULL,env=env,timeout=2,check=True)
        gate=path.read_text().splitlines()[0].split()[11]
    actual=r['actual_last_walls']
    w.writerow([r['case'],gate,actual,int(gate in actual.split('+'))])
