APPNAME = 'polygons'
VERSION = '0.1'

def options(opt):
    opt.load('compiler_cxx')

def configure(cfg):
    cfg.load('compiler_cxx')

    cfg.check_cfg(package='sdl2', args='--cflags --libs', uselib_store='SDL2')
    cfg.check_cxx(lib='Box2D', uselib_store='BOX2D')
    cfg.check_cxx(lib='SDL2_gfx', uselib_store='SDL2_GFX')

    cfg.env.append_value('CXXFLAGS', ['-std=c++11', '-g'])

def build(bld):
    source = [
        'main.cc',
    ]
    bld.program(
        source=source,
        target='polygons',
        use='SDL2 SDL2_GFX BOX2D'
    )
