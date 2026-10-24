"""Create and independently hash-verify a clean, source-linked Windows ZIP."""
import argparse,hashlib,json,subprocess,zipfile
from pathlib import Path
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--version',required=True)
parser.add_argument('--validation',type=Path,required=True)
args=parser.parse_args()
if not all(c.isalnum() or c in '.-' for c in args.version):raise SystemExit('Invalid version')
root=Path(__file__).resolve().parents[1];package=root/'Dist/Apollo12/Windows'
out=root/'Dist/Releases'/('apollo12-v'+args.version);out.mkdir(parents=True,exist_ok=True)
bundle='LunarRescue-Apollo12-v'+args.version+'-Windows';archive=out/(bundle+'.zip')
def digest(stream):
 h=hashlib.sha256()
 while block:=stream.read(1024*1024):h.update(block)
 return h.hexdigest()
def file_digest(p):
 with p.open('rb') as f:return digest(f)
commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()
if subprocess.check_output(['git','status','--porcelain'],cwd=root,text=True).strip():raise SystemExit('Commit the reviewed source before making a source-linked release')
validation=json.loads(args.validation.read_text(encoding='utf-8-sig'))
if any(c['failed'] or not c['passed'] for c in validation['cases']):raise SystemExit('Validation has failed/empty cases')
selected=[]
for p in sorted(package.rglob('*')):
 if not p.is_file():continue
 rel=p.relative_to(package)
 if any(part.lower() in {'saved','reports'} for part in rel.parts):continue
 if p.suffix.lower() in {'.pdb','.log','.csv'} or p.name.startswith('Manifest_'):continue
 selected.append((p,rel.as_posix()))
names={n for p,n in selected}
required={'LunarRescue.exe','LunarRescue/Binaries/Win64/LunarRescue.exe','LunarRescue/Content/Paks/LunarRescue-Windows.pak','LunarRescue/Content/Paks/LunarRescue-Windows.utoc','LunarRescue/Content/Paks/LunarRescue-Windows.ucas','READ_ME.txt','Credits/Apollo12/POLISH_UPDATE.md','Credits/Apollo12/HISTORICAL_AUDIT.md','Credits/Apollo12/PLAYTEST_GUIDE.md','Engine/Extras/Redist/en-us/vc_redist.x64.exe'}
if not required<=names:raise SystemExit('Missing runtime/credits files: '+str(required-names))
if file_digest(root/'Binaries/Win64/LunarRescue.exe')!=file_digest(package/'LunarRescue/Binaries/Win64/LunarRescue.exe'):raise SystemExit('Staged executable differs from the built executable')
info={'version':args.version,'source_commit':commit,'source_repository':'https://github.com/Shahzod1602/NASA','platform':'Windows 64-bit','configuration':'Development','validation':validation,'files':[{'path':rel,'bytes':p.stat().st_size,'sha256':file_digest(p)} for p,rel in selected]}
manifest=json.dumps(info,indent=2,ensure_ascii=False)+'\n';(out/'BUILD_INFO.json').write_text(manifest,encoding='utf-8')
with zipfile.ZipFile(archive,'x',compression=zipfile.ZIP_DEFLATED,compresslevel=6,allowZip64=True) as z:
 for p,rel in selected:z.write(p,bundle+'/'+rel)
 z.writestr(bundle+'/BUILD_INFO.json',manifest)
with zipfile.ZipFile(archive) as z:
 if z.testzip() is not None:raise RuntimeError('ZIP CRC failed')
 if len(z.namelist())!=len(selected)+1:raise RuntimeError('ZIP entry count mismatch')
 for row in info['files']:
  with z.open(bundle+'/'+row['path']) as f:
   if digest(f)!=row['sha256']:raise RuntimeError('ZIP hash mismatch: '+row['path'])
checksum=file_digest(archive);(out/(archive.name+'.sha256')).write_text(checksum+'  '+archive.name+'\n',encoding='ascii')
print(json.dumps({'archive':str(archive),'bytes':archive.stat().st_size,'sha256':checksum,'source_commit':commit,'entries':len(selected)+1},indent=2))
