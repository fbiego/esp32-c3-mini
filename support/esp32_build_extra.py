import glob, os, shutil, fileinput
import datetime


sep = os.sep

Import("env", "projenv")
print("Post build scripts > copy_elf.py")


def after_upload(source, target, env):
    env_name = str(source[0]).split(sep)[-2]
    print(f"Copying file from action {target[0]} for {env_name}")

    dest_dir = f"test{sep}esp32"
    os.makedirs(dest_dir, exist_ok=True)

    shutil.copyfile(f".pio{sep}build{sep}{env_name}{sep}firmware.elf", f"test{sep}esp32{sep}{env_name}.elf")
    shutil.copyfile(f".pio{sep}build{sep}{env_name}{sep}firmware.bin", f"test{sep}esp32{sep}{env_name}.bin")
    e = datetime.datetime.now()
    print (e.strftime("%H:%M:%S %d-%m-%Y"))
    
    
    
env.AddPostAction("upload", after_upload)
env.AddPostAction("buildprog", after_upload)