project('hw-cache', 'c',
  version         : '0.0',
  default_options : ['warning_level=3']
)

# option('type',
#  type        : 'combo',
#  choices     : ['static', 'dynamic'],
#  value       : 'dynamic',
#  description : 'Build static or dynamic library'
#)

sources = [
  'src/cache.c'
]

headers = [
  'src/cache.h'
]

headers_dir = include_directories('src')

if 'static' == get_option('type')
  cache_lib = static_library('hw-cache', sources,
    include_directories : headers_dir,
    install             : true
  )
else
  cache_lib = shared_library('hw-cache', sources,
    include_directories : headers_dir,
    install             : true
  )
endif

install_headers(headers)

test_exe = executable('hw-cache-test', 'src/main.c',
  include_directories : headers_dir,
  link_with           : cache_lib
)

test('Cache test', test_exe)
