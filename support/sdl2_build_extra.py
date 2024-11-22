import glob, os, shutil, fileinput
import datetime


sep = os.sep

Import("env", "projenv")

for e in [ env, projenv ]:
    # If compiler uses `-m32`, propagate it to linker.
    # Add via script, because `-Wl,-m32` does not work.
    if "-m32" in e['CCFLAGS']:
        e.Append(LINKFLAGS = ["-m32"])
    # e.Append(LINKFLAGS = ["-mwindows"])

exec_name = "${BUILD_DIR}/${PROGNAME}${PROGSUFFIX}"

# Override unused "upload" to execute compiled binary
from SCons.Script import AlwaysBuild
AlwaysBuild(env.Alias("upload", exec_name, exec_name))

# Add custom target to explorer
env.AddTarget(
    name = "execute",
    dependencies = exec_name,
    actions = exec_name,
    title = "Execute",
    description = "Build and execute",
    group="General"
)

def after_upload(source, target, env):

    env_prog = str(source[0]).split(sep)[-1]
    env_name = str(source[0]).split(sep)[-2]
    print(f"Copying file from action {target[0]} for {env_name}")

    dest_dir = f"test{sep}{env_name}"
    os.makedirs(dest_dir, exist_ok=True)

    shutil.copyfile(f".pio{sep}build{sep}{env_name}{sep}{env_prog}", f"test{sep}{env_name}{sep}{env_prog}")
    e = datetime.datetime.now()
    print (e.strftime("%H:%M:%S %d-%m-%Y"))
    
    
    
env.AddPostAction("upload", after_upload)
env.AddPostAction("buildprog", after_upload)