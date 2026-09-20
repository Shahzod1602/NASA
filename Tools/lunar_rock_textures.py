import json
from pathlib import Path
import unreal as u

root=Path(u.Paths.project_dir())
source=Path(r'C:\Users\shaxz\lunar_staging\Rocks')
library=u.EditorAssetLibrary
tools=u.AssetToolsHelpers.get_asset_tools()
edit=u.MaterialEditingLibrary
materials={}
for index in ('01','06'):
    image=source/f'MoonRock{index}_Color.jpg'
    task=u.AssetImportTask()
    task.filename=str(image)
    task.destination_path='/Game/Lunar/Imported/Rocks'
    task.destination_name='T_MoonRock'+index
    task.automated=True;task.replace_existing=True;task.save=True
    tools.import_asset_tasks([task])
    tex=library.load_asset('/Game/Lunar/Imported/Rocks/T_MoonRock'+index)
    if not tex: raise RuntimeError('Failed rock texture '+index)
    tex.set_editor_property('max_texture_size',2048)
    library.save_loaded_asset(tex,only_if_is_dirty=False)
    name='M_MoonRock'+index
    path='/Game/Lunar/Imported/Rocks/'+name
    mat=library.load_asset(path) if library.does_asset_exist(path) else tools.create_asset(name,'/Game/Lunar/Imported/Rocks',u.Material,u.MaterialFactoryNew())
    edit.delete_all_material_expressions(mat)
    sample=edit.create_material_expression(mat,u.MaterialExpressionTextureSample)
    sample.set_editor_property('texture',tex)
    edit.connect_material_property(sample,'RGB',u.MaterialProperty.MP_BASE_COLOR)
    rough=edit.create_material_expression(mat,u.MaterialExpressionConstant)
    rough.set_editor_property('r',0.96)
    edit.connect_material_property(rough,'',u.MaterialProperty.MP_ROUGHNESS)
    edit.recompile_material(mat)
    library.save_loaded_asset(mat,only_if_is_dirty=False)
    materials[index]=mat
levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
levels.load_level('/Game/Lunar/Maps/MoonBase')
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
count=0
for actor in actors.get_all_level_actors():
    if actor.get_actor_label()=='V3_Basalt fragment':
        actor.static_mesh_component.set_material(0,materials['01' if count%2==0 else '06'])
        count+=1
levels.save_current_level()
report={'textured_rock_actors':count,'source_archives':['moon_rock_01_4k.blend.zip','moon_rock_06_4k.blend.zip'],'texture_max_size':2048}
(root/'Reports/rock-textures.json').write_text(json.dumps(report,indent=2))
u.log('LUNAR_ROCK_TEXTURES '+str(report))
