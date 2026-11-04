from pathlib import Path
import json, hashlib, datetime, sys
root=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else Path(__file__).resolve().parents[1]
def digest(p):
 try:return hashlib.sha256(p.read_bytes()).hexdigest()
 except FileNotFoundError:return None
def compare(entries):
 return [{'path':e['path'],'expected':e['sha256'],'actual':digest(root/e['path'])} for e in entries if digest(root/e['path'])!=e['sha256']]
inputs=json.loads((root/'Reports/Gameplay20261103/build-inputs.json').read_text(encoding='utf-8-sig'))
preserved=json.loads((root/'Reports/Gameplay20261103/preservation-before.json').read_text(encoding='utf-8-sig'))
staged=root/'Dist/Apollo12/Windows/LunarRescue'
exe=digest(staged/'Binaries/Win64/LunarRescue.exe')
manifest=json.loads((root/'Content/Science/LunarWater/manifest.json').read_text(encoding='utf-8-sig'))
data=root/'Content/Science/LunarWater/study.json'
info=json.loads((root/'Dist/Apollo12/Windows/BUILD_INFO.json').read_text(encoding='utf-8-sig'))
checks={
 'built_and_staged_executable_match':exe is not None and exe==digest(root/'Binaries/Win64/LunarRescue.exe'),
 'build_info_identity_matches':info.get('executable_sha256')==exe and info.get('published_release') is False,
 'source_data_checksum_matches_manifest':digest(data)==manifest['sha256'] and data.stat().st_size==manifest['bytes'],
 'bundled_data_matches_source':digest(staged/'Content/Science/LunarWater/study.json')==digest(data),
 'bundled_manifest_matches_source':digest(staged/'Content/Science/LunarWater/manifest.json')==digest(root/'Content/Science/LunarWater/manifest.json'),
 'bundled_credits_match_source':digest(root/'Dist/Apollo12/Windows/Credits/LunarWater/CREDITS.md')==digest(root/'SourceArt/LunarWater/CREDITS.md'),
 'editor_module_matches_captured_build':digest(root/'Binaries/Win64/UnrealEditor-LunarRescue.dll')==inputs['editor_module_sha256']}
source_bad=compare(inputs['files']);map_bad=compare(inputs['maps']);player_bad=compare(preserved)
report={'checked_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'executable_sha256':exe,'checks':checks,
 'source_inputs':{'total':len(inputs['files']),'matched':len(inputs['files'])-len(source_bad),'mismatches':source_bad},
 'maps':{'total':len(inputs['maps']),'matched':len(inputs['maps'])-len(map_bad),'mismatches':map_bad},
 'original_player_files':{'total':len(preserved),'matched':len(preserved)-len(player_bad),'mismatches':player_bad},
 'data_id':manifest['dataset_id'],'data_sha256':digest(data),'human_playtest':False,'network_isolation_test':False}
report['passed']=all(checks.values()) and not(source_bad or map_bad or player_bad)
(root/'Reports/Gameplay20261103/bundle-verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
print(json.dumps({'passed':report['passed'],'checks':checks,'source_matched':report['source_inputs']['matched'],'maps_matched':report['maps']['matched'],'player_matched':report['original_player_files']['matched'],'sha256':exe},indent=2))
sys.exit(0 if report['passed'] else 1)
