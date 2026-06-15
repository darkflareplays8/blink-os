project('blink-panel', 'c', version: '1.0.0', default_options: ['c_std=c17'])

gtk4       = dependency('gtk4')
gio        = dependency('gio-2.0')
gtk_layer  = dependency('gtk4-layer-shell-0')
dl         = meson.get_compiler('c').find_library('dl')

src = files(
    'src/main.c',
    'src/panel.c',
)

executable('blink-panel', src,
    dependencies: [gtk4, gio, gtk_layer, dl],
    include_directories: include_directories('include'),
    install: true,
)

plugin_deps = [gtk4, gio]
plugin_dir  = get_option('prefix') / 'lib' / 'blink' / 'panel-plugins'

foreach plugin : ['clock', 'launcher', 'volume', 'network']
    shared_library('blink-plugin-' + plugin,
        'plugins/' + plugin + '.c',
        dependencies: plugin_deps,
        include_directories: include_directories('include'),
        install: true,
        install_dir: plugin_dir,
    )
endforeach
