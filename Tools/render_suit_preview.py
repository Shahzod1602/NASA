"""Render a credited suit portrait in an unsaved isolated editor world.

No mission map is loaded, regenerated or saved. This static bind-pose portrait
is UI art; it does not verify runtime animation, interaction or performance.
"""
from pathlib import Path
import json
import unreal as u

root=Path(u.Paths.project_dir()).resolve();out=root/'SourceArt/ExpeditionSuit20261113/Preview';out.mkdir(parents=True,exist_ok=True)
world=u.EditorLoadingAndSavingUtils.new_blank_map(False)
actors=u.get_editor_subsystem(u.EditorActorSubsystem) or u.get_default_object(u.EditorActorSubsystem)
mesh=u.EditorAssetLibrary.load_asset('/Game/Lunar/Suit/Expedition20261113/SK_ExpeditionSuit/SkeletalMeshes/SK_ExpeditionSuit')
if not mesh:raise RuntimeError('Missing credited suit derivative')
subject=actors.spawn_actor_from_class(u.SkeletalMeshActor,u.Vector(0,0,0))
subject_mesh=subject.get_editor_property('skeletal_mesh_component')
subject_mesh.set_skeletal_mesh_asset(mesh)
subject_mesh.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
inspection=[]
for name in ['SM_MuseumRover_Body']+['SM_MuseumRover_Wheel'+str(i) for i in range(4)]:
 asset=u.EditorAssetLibrary.load_asset('/Game/Lunar/Vehicles/Museum20261113/'+name+'/StaticMeshes/'+name)
 bounds=asset.get_bounds()
 inspection.append({'name':name,'origin_cm':[bounds.origin.x,bounds.origin.y,bounds.origin.z],'extent_cm':[bounds.box_extent.x,bounds.box_extent.y,bounds.box_extent.z]})
(root/'Reports/Feedback20261113/imported-rover-bounds.json').write_text(json.dumps(inspection,indent=2)+'\n')
for position,intensity in [(u.Vector(240,-200,180),180.),(u.Vector(100,210,100),90.)]:
 light=actors.spawn_actor_from_class(u.PointLight,position);light_component=light.get_editor_property('light_component');light_component.set_mobility(u.ComponentMobility.MOVABLE);light_component.set_intensity(intensity);light_component.set_attenuation_radius(1000.);light_component.set_cast_shadows(False)
target=u.RenderingLibrary.create_render_target2d(world,512,768,u.TextureRenderTargetFormat.RTF_RGBA8,u.LinearColor(.007,.021,.035,1.))
capture=actors.spawn_actor_from_class(u.SceneCapture2D,u.Vector(210,-27,15))
component=capture.get_component_by_class(u.SceneCaptureComponent2D)
component.texture_target=target;component.capture_source=u.SceneCaptureSource.SCS_FINAL_COLOR_LDR;component.capture_every_frame=False;component.capture_on_movement=False;component.fov_angle=36.
capture.set_actor_rotation(u.MathLibrary.find_look_at_rotation(capture.get_actor_location(),u.Vector(0,0,4)),False)
# Do not wait for every unused parent/editor shader permutation. Runtime
# material acceptance is covered separately by the rendered gameplay reviews.
component.capture_scene()
u.RenderingLibrary.export_render_target(world,target,str(out),'SuitPreview.png')
if not (out/'SuitPreview.png').is_file():raise RuntimeError('Suit portrait export missing')
task=u.AssetImportTask();task.filename=str(out/'SuitPreview.png');task.destination_path='/Game/Lunar/UI/Expedition20261114';task.destination_name='SuitPreview';task.automated=True;task.replace_existing=True;task.save=True
u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
if not task.imported_object_paths:raise RuntimeError('Suit portrait texture import missing')
u.log('SUIT_PORTRAIT_COMPLETE / unsaved world / static bind-pose only')
