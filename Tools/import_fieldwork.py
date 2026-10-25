"""Import the reference-led suit/tool revisions and the illustrative coating material."""
from pathlib import Path
import unreal as u

root = Path(u.Paths.project_dir()).resolve()
lib = u.EditorAssetLibrary
folder = '/Game/Lunar/Apollo12/Fieldwork'
lib.make_directory(folder)
path = folder + '/M_SurveyorCoating'
material = lib.load_asset(path) if lib.does_asset_exist(path) else u.AssetToolsHelpers.get_asset_tools().create_asset('M_SurveyorCoating', folder, u.Material, u.MaterialFactoryNew())
edit = u.MaterialEditingLibrary
edit.delete_all_material_expressions(material)
color = edit.create_material_expression(material, u.MaterialExpressionVectorParameter, -250, 0)
color.set_editor_property('parameter_name', 'CoatingColor')
color.set_editor_property('default_value', u.LinearColor(.64, .62, .56, 1))
edit.connect_material_property(color, '', u.MaterialProperty.MP_BASE_COLOR)
rough = edit.create_material_expression(material, u.MaterialExpressionConstant, -250, 160)
rough.set_editor_property('r', .83)
edit.connect_material_property(rough, '', u.MaterialProperty.MP_ROUGHNESS)
edit.recompile_material(material)
lib.save_loaded_asset(material)
for filename in ('import_apollo_realism.py', 'import_apollo_polish.py'):
    script = root / 'Tools' / filename
    exec(compile(script.read_text(encoding='utf-8'), str(script), 'exec'), {'__name__': '__main__', '__file__': str(script)})
for name in ('SM_TongsBase','SM_TongsJawL','SM_TongsJawR','SM_CuttersBase','SM_CuttersJawL','SM_CuttersJawR','SM_SamplePouch'):
    if not lib.does_asset_exist('/Game/Lunar/Apollo12/Tools/' + name):
        raise RuntimeError('Missing work mesh ' + name)
u.log('APOLLO_FIELDWORK_ASSETS_READY')
