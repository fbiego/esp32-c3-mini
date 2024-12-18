import glob, os, shutil, fileinput
import datetime


sep = os.sep

Import("env", "projenv")
print("Post build scripts > hardware_build_extra.py")

def after_upload(source, target, env):
    env_name = str(source[0]).split(sep)[-2]
    print(f"Copying files from action {target[0]} for {env_name}")

    dest_dir = f"test{sep}hardware"
    os.makedirs(dest_dir, exist_ok=True)

    extensions = ['elf', 'bin', 'uf2']
    for ext in extensions:
        src_file = f".pio{sep}build{sep}{env_name}{sep}firmware.{ext}"
        dest_file = f"{dest_dir}{sep}{env_name}.{ext}"
        if os.path.isfile(src_file):
            shutil.copyfile(src_file, dest_file)
        else:
            print(f"Source file {src_file} does not exist.")

    e = datetime.datetime.now()
    print(e.strftime("%H:%M:%S %d-%m-%Y"))
    
env.AddPostAction("upload", after_upload)
env.AddPostAction("buildprog", after_upload)