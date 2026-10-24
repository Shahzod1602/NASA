"""Apply the v0.5 tool instructions to stable English String Table keys."""
from pathlib import Path
import json,re
p=Path(__file__).resolve().parents[1]/'Source/LunarRescue/LunarApolloStrings.inl'
changes={
'MenuHelp':'F1 Settings. Controls are introduced as you play.',
'Footer':'F1 Settings     I Evidence album     Esc Cancel tool / Pause',
'InstructionCheckout0':'Approach the field kit. Open its work panel and check suit support.',
'InstructionCheckout1':'Open the field-kit panel and secure the camera and sample tools.',
'InstructionCheckout2':'Confirm Intrepid as your return point. The journal contains your map.',
'InstructionSeismometer':'Open the seismometer tool. Center the bubble with the mouse, then confirm.',
'InstructionAntenna':'Open antenna alignment and find the strongest signal. Current bearing: {Angle} degrees (simplified).',
'InstructionSamples':'Photograph each site, then open its sampling tool. Head: rock. Bench: soil. Sharp: drive-tube core.',
'InstructionTraverse':'Choose your route to Surveyor 3. The journal contains the historical route and your destination.',
'InstructionSurveyorPhoto':'Approach Surveyor 3 and open the context camera. Center the spacecraft, then release the shutter.',
'InstructionSurveyorParts':'Open a camera, scoop or cable work site. Release its fittings, then remove and pack the component.',
'InstructionReturn':'Return to the field kit. Open the cargo manifest and confirm stowage. I opens your evidence album.',
'TutorialWork':'GETTING STARTED 3/3   Approach the table. Use the work key shown below, then left click to check the equipment.',
'PhotoMessage':'Context photo recorded. Open the sampling tool to collect and seal this sample.',
'AntennaWrong':'The signal is weak. Open antenna alignment, rotate toward the strongest signal, then verify.',
'SourcesAdaptation':'Selected work from two EVAs is combined into one single-player chapter. Tools and ALSEP alignment are simplified. Historical mode is untimed; a separate 30-minute exercise is not actual suit oxygen endurance.',
'BriefHelp':'T selects untimed exploration or a timed exercise. F1 opens settings. Controls appear when needed.',
'BriefAdaptation':'Selected work from two EVAs is combined. Tools and alignment are simplified; this is an educational game.',
'EvidenceSamples':'12031 at Head is a rock; 12032 at Bench is fine soil. 12027 at Sharp is a hammer-driven core tube. I opens your actual photographs, sample labels and cargo.',
'EvidencePhoto':'Context photographs connect a sample to its work site. Frame the target before releasing the shutter; evidence records the site and mission time.',
'WinCargo':'Three samples and three selected Surveyor components are stowed. I opens the cargo and photograph debrief.',
}
seen=set()
def replace(m):
 k=m[1]
 if k not in changes:return m[0]
 seen.add(k);return 'APOLLO_TEXT('+json.dumps(k)+', '+json.dumps(changes[k])+')'
text=re.sub(r'APOLLO_TEXT\("([^"]+)", "(?:\\.|[^"\\])*"\)',replace,p.read_text(encoding='utf-8'))
if seen!=changes.keys():raise RuntimeError('Unknown keys '+str(changes.keys()-seen))
p.write_text(text,encoding='utf-8')
