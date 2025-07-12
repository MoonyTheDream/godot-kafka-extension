# SConstruct
import os
# Import('env')  # if needed

# Import Godot-cpp SConstruct to get a configured SCons Environment
env = SConscript('godot-cpp/SConstruct')

# Add our source and include directories
env.Append(CPPPATH=[
    'src',
    'include',
    'thirdparty/modern-cpp-kafka/include'  # where Kafka headers are
])

env.Append(CXXFLAGS=['-fexceptions'])

# Link against librdkafka (librdkafka++ if needed)
env.Append(LIBPATH=['/usr/local/lib'])
env.Append(LIBS=['rdkafka'])
# If librdkafka is not in standard paths, set LIBPATH accordingly:
# env.Append(LIBPATH=['/usr/local/lib', 'C:/path/to/librdkafka/lib'])

sources = Glob('src/*.cpp')

# Build the shared library (extension)
if env['platform'] == 'macos':
    target = f'demo/kafkaextensionstest/bin/KafkaExtension.{env["platform"]}.{env["target"]}.{env["platform"]}.{env["target"]}{env["SHLIBSUFFIX"]}'
else:
    target = f'demo/kafkaextensionstest/bin/KafkaExtension{env["suffix"]}{env["SHLIBSUFFIX"]}'
library = env.SharedLibrary(target, source=sources)

# On Windows, copy the librdkafka DLL into the output folder
if env['platform'] == 'windows':
    dlls = ['path/to/rdkafka.dll']  # adjust this path
    for dll in dlls:
        env.Command(
            target=os.path.join('demo/kafkaextensionstest/bin', os.path.basename(dll)),
            source=dll,
            action=Copy("$TARGET", "$SOURCE")
        )

Default(library)


