project(
    'libflush',
    'c',
    version: '1.0.0',
    license: 'MIT',
    default_options: [],
)


# Set compiler
compiler = meson.get_compiler('c')

# Get build opts
platform = get_option('platform')
timer = get_option('timer')
build_examples = get_option('examples')

# Set timesource
if timer == 'perf'
  timesource = '-DTIME_SOURCE=TIME_SOURCE_PERF'
elif timer == 'thread-counter'
  timesource = '-DTIME_SOURCE=THREAD_COUNTER'
else
  timesource = '-DTIME_SOURCE=TIME_SOURCE_MONOTONIC_CLOCK'
endif

# Find dependencies
thread = compiler.find_library('pthread', required: true)
check = compiler.find_library('check', required: true)
zlib = compiler.find_library('z', required: true)
math = compiler.find_library('m', required: true)
timer = compiler.find_library('rt', required: true)
subunit = compiler.find_library('subunit', required: true)

src = ['src/eviction.c', 'src/libflush.c', 'src/timing.c', 'src/utils.c']
arm = ['arch/arm/flush.c', 'arch/arm/memory.c', 'arch/arm/timing.c']
x86 = ['arch/x86/flush.c', 'arch/x86/memory.c', 'arch/x86/timing.c']

tests = [
  'tests/test_runner.c',
  'tests/test_eviction.c',
  'tests/test_memory.c',
  'tests/test_timing.c',
  'tests/test_libflush.c'
]

include = include_directories('include')
arch = include_directories('arch')

if platform == 'x86'
  libflush = shared_library('flush',
    [src, x86],
    include_directories: [include, arch],
    dependencies: thread,
    c_args: ['-DHAVE_PAGEMAP_ACCESS=1', '-D__x86__', '-DHAVE_STROPTS_H=0', timesource]
  )
endif

if platform == 'arm' 
  libflush = shared_library('flush',
    [src, arm],
    include_directories: [include, arch],
    dependencies: thread,
    c_args: ['-DHAVE_PAGEMAP_ACCESS=1', '-D__ARM__', '-DHAVE_STROPTS_H=0', timesource]
  )
endif


utest = executable(
  'utest',
  tests, 
  include_directories: [include, arch], 
  dependencies: [thread, check, zlib, math, timer, subunit],
  link_with: libflush
)

if build_examples
  executable(
    'sample-session',
    ['examples/sample-session.c'], 
    include_directories: [include, arch],
    dependencies: [thread, check, zlib, math, timer, subunit],
    link_with: libflush
  )
  executable(
    'evict-time',
    ['examples/evict-time.c'], 
    include_directories: [include, arch],
    dependencies: [thread, check, zlib, math, timer, subunit],
    link_with: libflush
  )
  executable(
    'prime-probe',
    ['examples/prime-probe.c'], 
    include_directories: [include, arch],
    dependencies: [thread, check, zlib, math, timer, subunit],
    link_with: libflush
  )
endif

test('Running unit tests for libflush', utest)
