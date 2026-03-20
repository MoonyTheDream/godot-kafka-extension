# SConstruct
import os
import subprocess

env = SConscript('godot-cpp/SConstruct')

# --- Auto-build librdkafka if not installed ---
rdkafka_install = 'thirdparty/librdkafka/install'
rdkafka_src = 'thirdparty/librdkafka'
rdkafka_build = os.path.join(rdkafka_src, 'build')

if not os.path.exists(os.path.join(rdkafka_install, 'include', 'librdkafka', 'rdkafka.h')):
    if not os.path.exists(os.path.join(rdkafka_src, 'CMakeLists.txt')):
        print('ERROR: thirdparty/librdkafka submodule not initialized.')
        print('Run: git submodule update --init --recursive thirdparty/librdkafka')
        Exit(1)
    print('=== Building librdkafka ===')
    os.makedirs(rdkafka_build, exist_ok=True)
    cmake_args = [
        'cmake', '-S', rdkafka_src, '-B', rdkafka_build,
        '-DRDKAFKA_BUILD_STATIC=OFF', '-DWITH_SSL=ON',
        '-DRDKAFKA_BUILD_TESTS=OFF', '-DRDKAFKA_BUILD_EXAMPLES=OFF',
    ]
    if env['platform'] == 'windows':
        cmake_args += ['-A', 'x64']
        # Auto-detect OpenSSL; override with OPENSSL_ROOT_DIR env var
        openssl_candidates = [
            os.environ.get('OPENSSL_ROOT_DIR', ''),
            r'C:\Program Files\FireDaemon-OpenSSL-3',
            r'C:\Program Files\OpenSSL-Win64',
            r'C:\Program Files\OpenSSL',
        ]
        for candidate in openssl_candidates:
            if candidate and os.path.exists(os.path.join(candidate, 'include', 'openssl', 'ssl.h')):
                cmake_args += [f'-DOPENSSL_ROOT_DIR={candidate}']
                print(f'=== Using OpenSSL from: {candidate} ===')
                break
    subprocess.check_call(cmake_args)
    subprocess.check_call(['cmake', '--build', rdkafka_build, '--config', 'Release'])
    subprocess.check_call(['cmake', '--install', rdkafka_build,
                           '--prefix', rdkafka_install, '--config', 'Release'])
    print('=== librdkafka built and installed ===')

# --- Extension build ---
env.Append(CPPPATH=['src', 'include',
                    'thirdparty/modern-cpp-kafka/include',
                    os.path.join(rdkafka_install, 'include')])
env.Append(LIBPATH=[os.path.join(rdkafka_install, 'lib')])
env.Append(LIBS=['rdkafka'])

if env['platform'] == 'windows':
    env.Append(CXXFLAGS=['/EHsc'])
else:
    env.Append(CXXFLAGS=['-fexceptions'])
    if env['platform'] == 'linux':
        env.Append(LINKFLAGS=['-Wl,-rpath,$ORIGIN'])

sources = Glob('src/*.cpp')

if env['platform'] == 'macos':
    target = f'demo/kafkaextensionstest/bin/KafkaExtension.{env["platform"]}.{env["target"]}.{env["platform"]}.{env["target"]}{env["SHLIBSUFFIX"]}'
else:
    target = f'demo/kafkaextensionstest/bin/KafkaExtension{env["suffix"]}{env["SHLIBSUFFIX"]}'

library = env.SharedLibrary(target, source=sources)

# Copy librdkafka runtime libraries to output
bin_dir = 'demo/kafkaextensionstest/bin'
if env['platform'] == 'windows':
    for dll in Glob(os.path.join(rdkafka_install, 'bin', '*.dll')):
        env.Command(os.path.join(bin_dir, os.path.basename(str(dll))),
                    dll, Copy("$TARGET", "$SOURCE"))
elif env['platform'] == 'linux':
    for so in Glob(os.path.join(rdkafka_install, 'lib', 'librdkafka.so*')):
        env.Command(os.path.join(bin_dir, os.path.basename(str(so))),
                    so, Copy("$TARGET", "$SOURCE"))

Default(library)
