import unreal as u
tex=u.EditorAssetLibrary.load_asset('/Game/Lunar/V2/Textures/T_Earth_Clouds')
tex.set_editor_property('never_stream',True)
tex.set_editor_property('mip_gen_settings',u.TextureMipGenSettings.TMGS_NO_MIPMAPS)
tex.set_editor_property('max_texture_size',2048)
u.EditorAssetLibrary.save_loaded_asset(tex,only_if_is_dirty=False)
