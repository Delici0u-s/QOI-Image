project('compiler2', ['cpp', 'c'],
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++23', 'buildtype=debug', 'optimization=2', 'debug=true'])
# type "meson setup build --buildtype=release" in console if release version is wanted

# dont change variable names, dont remove them. amca might not work then. You may change the value
main_file = 'src/main.cpp'
output_name = 'a'
output_dir = '../../compiled' # Relative to meson.build location
build_dir_where = 'MesonBuildStuff/build' # Relative to meson.build location
# from here change whatever


cpp_compiler_args = [   
  '-Og',
  '-g',
  '-Wall', 
  '-Wextra',
  '-pedantic',
  '-O0',
  '-ggdb',
  '-fno-omit-frame-pointer', # Keep function frames for profiling
  # '-Ofast',
  # '-Os',
  # '-Oz', '-s'
  #'-DNDEBUG',
]

c_compiler_args = [
]

main_file = main_file.replace('/', '\\')
output_dir = output_dir.replace('/', '\\')
if not output_dir.startswith('\\')
  output_dir = '\\' + output_dir
endif
if not output_dir.endswith('\\') and output_dir != ''
  output_dir = ''.join([output_dir, '\\'])
endif
output_dir = meson.project_build_root() + output_dir

cmake           = import('cmake')
fs              = import('fs')
opt_var_mimalloc = cmake.subproject_options()

opt_var_mimalloc.add_cmake_defines({'MI_SECURE': false})
opt_var_mimalloc.add_cmake_defines({'MI_BUILD_TESTS': false})
opt_var_mimalloc.add_cmake_defines({'MI_BUILD_SHARED': false})


dependencies = []
dependencies_str = []

foreach item : dependencies_str
  dependencies += dependency(item)
endforeach


source_files = run_command('python', 'MesonBuildStuff/globber.py', './', '*.cpp', '*.cxx', '*.cc', '*.c', check: true).stdout().strip().split('\n')

# remove main_file from source_files so ninja wont complain
source_files_no_main_file = []
foreach item : source_files
  if item != main_file
    source_files_no_main_file += item
  endif
endforeach

if fs.is_dir('./include/')
  headers = include_directories('./include/') 
elif fs.is_dir('./Code/include/')
  headers = include_directories('./Code/include/') 
else
  headers = []
endif

executable(output_name,
           [main_file, source_files_no_main_file],
           dependencies : dependencies,
           include_directories : headers,
           cpp_args : cpp_compiler_args,
           c_args : c_compiler_args,
           link_args : ['-static-libgcc', '-static-libstdc++'],
           install : true,
           install_dir : output_dir)

# printing context
message('\033[2K\r\nsource files: \n   ', '   '.join(source_files), '\noutputs to:\n   ', output_dir + output_name, '\n')