test_exe = executable('test-intelliselect-backend',
  [
    'test-intelliselect-backend.c',
  ],
  include_directories: [ rootInclude, rootAppInclude ],
  dependencies: [ gegl, gtk3 ],
  link_with: [ libapptools ],
)

test('intelliselect-backend',
  test_exe,
  suite: 'plug-ins/intelliselect')
