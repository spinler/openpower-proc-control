installdir = join_paths(get_option('libexecdir'), 'openpower-proc-control')

cfam_deps = [
      sdbusplus_dep,
      pdi_dep,
      phosphor_logging_dep,
]

executable('rbmc-cfamd',
    'application.cpp',
    'cfam_main.cpp',
    dependencies: [
      cfam_deps
    ],
    install: true,
    install_dir: installdir,
)
