import sys
import subprocess
from pathlib import Path
import os

## pcd to xyz
python_path = Path("C:/Users/3DSMART/AppData/Local/Programs/Python/Python311/python.exe")
script_path = Path("./pcd2xyz.py").resolve()
cmd = f'"{python_path}" "{script_path}"'
subprocess.run(cmd, shell=True)

## Region growing segmentation
build_dir = Path("C:/SLAM/floorplan_code_v1/Region_growing/build")
exe_path = build_dir / "Region_growing.exe"
subprocess.run([str(exe_path)], shell=True)


## normal estimation
script_path2 = Path("./normal_estimation.py").resolve()
cmd = f'"{python_path}" "{script_path2}"'
subprocess.run(cmd, shell=True)

## Segmentation of walls floors and ceilings
build_dir = Path("C:/SLAM/floorplan_code_v1/Segmentation/build")
exe_path = build_dir / "Segmentation.exe"
subprocess.run([str(exe_path)], shell=True)

## normal estimation of wall points
script_path3 = Path("./normal_estimation_txt.py").resolve()
cmd = f'"{python_path}" "{script_path3}"'
subprocess.run(cmd, shell=True)

## floorplan generation
build_dir = Path("C:/SLAM/floorplan_code_v1/floorplan/build/Facade")
exe_path = build_dir / "Facade.exe"
subprocess.run([str(exe_path)], shell=True)

### visual floorplan and BIM
script_path3 = Path("./floorplan_vis.py").resolve()
cmd = f'"{python_path}" "{script_path3}"'
subprocess.run(cmd, shell=True)

script_path4 = Path("./main_generate_wall_vis.py").resolve()
cmd = f'"{python_path}" "{script_path4}"'
subprocess.run(cmd, shell=True)
