plugin_name = 'intelliselect'

plugin_sources = [
  'intelliselect.c',
]

if platform_windows
  plugin_sources += windows.compile_resources(
    gimp_plugins_rc,
    args: [
      '--define', 'ORIGINALFILENAME_STR="@0@"'.format(plugin_name + '.exe'),
      '--define', 'INTERNALNAME_STR="@0@"'.format(plugin_name),
      '--define', 'TOP_SRCDIR="@0@"'.format(meson.project_source_root()),
    ],
    include_directories: [
      rootInclude, appInclude,
    ],
  )
endif

plugin_intelliselect_deps = [
  libgimpui_dep,
  gegl,
]

intelliselect = executable(plugin_name,
                           plugin_sources,
                           dependencies: plugin_intelliselect_deps,
                           win_subsystem: 'windows',
                           install: true,
                           install_dir: gimpplugindir / 'plug-ins' / plugin_name)

install_subdir('vendor',
               install_dir: gimpplugindir / 'plug-ins' / plugin_name / 'vendor')

plugin_executables += [intelliselect.full_path()]
