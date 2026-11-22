from pathlib import Path
s=Path('tactical.cpp').read_text()
b=Path('bank.cpp').read_text()
pre,s=s.split('namespace {',1)
preb,b=b.split('namespace {',1)
assert pre==preb
parts=[]
for name,txt in [('original',s),('plain',b)]:
 txt='namespace { '+txt
 txt=txt.replace('int main(int argc,char**argv){\n ios::sync_with_stdio(false);cin.tie(nullptr);','int run(int argc,char**argv,std::istringstream& pref){')
 assert 'int run(' in txt
 txt=txt.replace(' Game g;g.init(', ' auto* oldbuf=cin.rdbuf(pref.rdbuf());\n Game g;g.init(',1)
 end=txt.index('\n',txt.index(' Game g;g.init('))
 txt=txt[:end]+'\n cin.rdbuf(oldbuf);'+txt[end:]
 if name=='plain':
  txt=txt.replace('g.bankMode=argc>1?atoi(argv[1]):0;', 'g.bankMode=5;').replace('g.bankDirection=argc>2?atoi(argv[2]):0;', 'g.bankDirection=0;')
 txt=txt.rstrip()[:-1]+' return 0;\n}\n'
 parts.append('namespace '+name+' {\n'+txt+'}\n')
main=r'''
int main(int argc,char** argv){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,m,r,c,k;
 if(!(cin>>n))return 0;
 ostringstream s;s<<n<<'\n';
 for(int i=0;i<n;i++){cin>>r>>c>>k;s<<r<<' '<<c<<' '<<k<<'\n';}
 cin>>m;s<<m<<'\n';
 for(int i=0;i<m;i++){cin>>r>>c;s<<r<<' '<<c<<'\n';}
 istringstream pref(s.str());
 if(m>=8&&n<=12)return plain::run(1,argv,pref);
 return original::run(argc,argv,pref);
}
'''
Path('portfolio.cpp').write_text(pre+''.join(parts)+main)
