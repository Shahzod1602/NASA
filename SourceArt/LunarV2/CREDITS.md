# Environment V2 sources and realism notes

Earth cloud/continent texture: three.js example asset, earth_atmos_2048.jpg.
https://github.com/mrdoob/three.js/blob/dev/examples/textures/planets/earth_atmos_2048.jpg
Repository license saved as LICENSE-threejs.txt.

Star positions, visual magnitudes and B-V colors: Olaf Frohn, D3-Celestial stars.6.json.
https://github.com/ofrohn/d3-celestial
https://github.com/ofrohn/d3-celestial/blob/master/data/stars.6.json
Repository license saved as LICENSE-d3-celestial.txt.
5,044 catalog stars are rendered as small discs, combined into three color-group meshes.

NASA Deep Star Maps and Blue Marble were investigated but the local downloads timed out.
No NASA EXR or Blue Marble download is included in the final environment assets.

The terrain is an original procedural game mesh with impact bowls, rims, smaller craters,
undulating ground and irregular basalt fragments. It is not a surveyed lunar location.
Working areas are leveled to preserve accessibility to the existing mission objects.

Earth is approximately two degrees across. Planet points are artist-placed representations
of Venus, Jupiter and Mars; their placement is not an ephemeris for a particular date.
Star brightness and shadow fill are adjusted for game readability. This is not a physical
simulation of eye/camera exposure in lunar daylight. No atmosphere or atmospheric twinkling
is added to the Moon. Earth has a thin artistic limb glow.
