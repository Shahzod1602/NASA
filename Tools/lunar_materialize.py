import json
from pathlib import Path
import unreal as u

source=Path(r'C:\Users\shaxz\lunar_staging')
library=u.EditorAssetLibrary
tool=u.AssetToolsHelpers.get_asset_tools()
edit=u.MaterialEditingLibrary
report={}
for folder in sorted(p for p in source.iterdir() if p.is_dir() and (p/'material.mtl').exists()):
    name=folder.name
    prefix='/Game/Lunar/Imported/'+name
    mesh=library.load_asset(prefix+'/'+name)
    if not mesh: raise RuntimeError('Missing mesh '+name)
    definitions={}
    current=None
    for line in (folder/'material.mtl').read_text().splitlines():
        items=line.split()
        if not items: continue
        if items[0]=='newmtl':
            current=items[1];definitions[current]={'color':(.5,.5,.5),'image':None}
        elif current and items[0]=='Kd':
            definitions[current]['color']=tuple(float(x) for x in items[1:4])
        elif current and items[0]=='map_Kd':
            definitions[current]['image']=' '.join(items[1:])
    images={}
    for image in folder.glob('*.png'):
        key=image.stem.replace('.','_')
        asset_name='T_'+key
        task=u.AssetImportTask()
        task.filename=str(image)
        task.destination_path=prefix
        task.destination_name=asset_name
        task.automated=True;task.save=True;task.replace_existing=True
        tool.import_asset_tasks([task])
        tex=library.load_asset(prefix+'/'+asset_name)
        if not tex: raise RuntimeError('Texture failed '+str(image))
        tex.set_editor_property('max_texture_size',2048)
        library.save_loaded_asset(tex,only_if_is_dirty=False)
        images[image.name]=tex
    mats=[]
    slots=mesh.get_editor_property('static_materials')
    for i,slot in enumerate(slots):
        raw=str(slot.material_slot_name)
        original=next((k for k in definitions if k.replace('.','_')==raw),raw)
        spec=definitions.get(original,{'color':(.5,.5,.5),'image':None})
        matname='M_'+name+'_'+raw
        path=prefix+'/'+matname
        mat=library.load_asset(path) if library.does_asset_exist(path) else tool.create_asset(matname,prefix,u.Material,u.MaterialFactoryNew())
        edit.delete_all_material_expressions(mat)
        image=images.get(spec['image'])
        if image:
            color=edit.create_material_expression(mat,u.MaterialExpressionTextureSample)
            color.set_editor_property('texture',image)
            edit.connect_material_property(color,'RGB',u.MaterialProperty.MP_BASE_COLOR)
        else:
            color=edit.create_material_expression(mat,u.MaterialExpressionConstant3Vector)
            color.set_editor_property('constant',u.LinearColor(*spec['color']))
            edit.connect_material_property(color,'',u.MaterialProperty.MP_BASE_COLOR)
        rough=edit.create_material_expression(mat,u.MaterialExpressionConstant)
        rough.set_editor_property('r',0.72 if name not in ('Lunokhod','ApolloLander') else 0.62)
        edit.connect_material_property(rough,'',u.MaterialProperty.MP_ROUGHNESS)
        edit.recompile_material(mat)
        library.save_loaded_asset(mat,only_if_is_dirty=False)
        mesh.set_material(i,mat)
        mats.append(mat.get_path_name())
    library.save_loaded_asset(mesh,only_if_is_dirty=False)
    report[name]={'materials':mats,'textures':[x.get_path_name() for x in images.values()]}
    u.log('LUNAR_MATERIALIZED '+name+' '+str(len(mats))+' materials')
(source/'materials_report.json').write_text(json.dumps(report,indent=2))
