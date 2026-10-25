import unreal as u
from pathlib import Path
root=Path(u.Paths.project_dir()).resolve();lib=u.EditorAssetLibrary;assets=u.AssetToolsHelpers.get_asset_tools()
u.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.OBJ 0')
for p in (root/'SourceArt/Apollo12/Tools').glob('*.obj'):
    opts=u.FbxImportUI();opts.automated_import_should_detect_type=False;opts.import_materials=False;opts.import_textures=False;opts.import_as_skeletal=False;opts.mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH
    opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=False;opts.static_mesh_import_data.generate_lightmap_u_vs=False
    t=u.AssetImportTask();t.filename=str(p);t.destination_path='/Game/Lunar/Apollo12/Tools';t.automated=True;t.replace_existing=True;t.save=True;t.factory=u.FbxFactory();t.options=opts;assets.import_asset_tasks([t])
    mesh=lib.load_asset(t.destination_path+'/'+p.stem)
    if not mesh:raise RuntimeError('Tool import failed '+p.name)
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        matpath='/Game/Lunar/Suit/Materials/M_'+str(slot.material_slot_name)
        mat=lib.load_asset(matpath) if lib.does_asset_exist(matpath) else lib.load_asset('/Game/Lunar/Suit/Materials/M_Metal')
        if mat:mesh.set_material(i,mat)
    lib.save_loaded_asset(mesh)
for p in (root/'SourceArt/Apollo12/Radio').glob('*.wav'):
    t=u.AssetImportTask();t.filename=str(p);t.destination_path='/Game/Lunar/Apollo12/Radio';t.automated=True;t.replace_existing=True;t.save=True;assets.import_asset_tasks([t])
    if not t.imported_object_paths:raise RuntimeError('Radio import failed '+p.name)
u.EditorLoadingAndSavingUtils.load_map('/Game/Lunar/Maps/Apollo12')
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
all_actors=actors.get_all_level_actors()
for actor in all_actors:
    if actor.get_actor_label()=='Readability fill':actor.light_component.set_intensity(.19)
bounce=next((a for a in all_actors if a.get_actor_label()=='Readability bounce'),None)
if bounce is None:
    bounce=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,9000),u.Rotator(pitch=-35,yaw=180,roll=0));bounce.set_actor_label('Readability bounce')
bounce.light_component.set_mobility(u.ComponentMobility.MOVABLE);bounce.light_component.set_intensity(.14);bounce.light_component.set_cast_shadows(False)
u.EditorLoadingAndSavingUtils.save_current_level()
skeletal=u.get_editor_subsystem(u.SkeletalMeshEditorSubsystem)
for path in ('/Game/Lunar/Suit/Rig/SK_A7L/SkeletalMeshes/SK_A7L','/Game/Lunar/Suit/Rig/SK_A7L_Hands/SkeletalMeshes/SK_A7L_Hands'):
    mesh=lib.load_asset(path)
    if not mesh:raise RuntimeError('Missing skeletal suit '+path)
    if skeletal.get_lod_count(mesh)<3:
        if not skeletal.regenerate_lod(mesh,3,False,False):raise RuntimeError('Suit LOD generation failed')
    u.log('SUIT_LODS '+path+' '+str([skeletal.get_num_verts(mesh,i) for i in range(skeletal.get_lod_count(mesh))]))
    lib.save_loaded_asset(mesh)
lib.save_directory('/Game/Lunar/Apollo12/Tools');lib.save_directory('/Game/Lunar/Apollo12/Radio')
u.log('APOLLO_POLISH_ASSETS_READY')
