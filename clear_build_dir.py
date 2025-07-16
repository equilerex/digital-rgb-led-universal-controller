Import("env")
import shutil
import os

# Clean the build directory before each build
def clean_build_dir(source, target, env):
    print("Cleaning build directory...")
    build_dir = env.subst("$BUILD_DIR")
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
    
    # Also force reinstall of FastLED
    lib_dir = env.subst("$PROJECT_LIBDEPS_DIR")
    fastled_dir = os.path.join(lib_dir, env.subst("$PIOENV"), "FastLED")
    if os.path.exists(fastled_dir):
        print("Removing FastLED library for fresh install...")
        shutil.rmtree(fastled_dir)

env.AddPreAction("$BUILD_DIR/libFrameworkArduino.a", clean_build_dir)
