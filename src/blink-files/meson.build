project('blink-files', 'c', version: '1.0.0', default_options: ['c_std=c17'])

gtk4 = dependency('gtk4')
gio  = dependency('gio-2.0')

src = files(
    'src/main.c',
    'src/files.c',
)

executable('blink-files', src,
    dependencies: [gtk4, gio],
    include_directories: include_directories('include'),
    install: true,
)
