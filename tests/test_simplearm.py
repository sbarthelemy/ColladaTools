#!/usr/bin/env python
import tempfile
import subprocess
import filecmp
from os.path import join
import os
import sys

source_dir = sys.argv[1]
exec_dir = sys.argv[2]

tmp_file = join(tempfile.gettempdir(), 'out.dae')
os.unlink(tmp_file)

subprocess.check_call([
        join(exec_dir, 'h5toanim'),
        join(source_dir, 'data/simplearm_traj_flat.h5'),
        join(source_dir, 'data/simplearm_scene_flat.dae'),
        tmp_file])

ok = filecmp.cmp(
        tmp_file,
        join(source_dir, 'data/simplearm_animation_flat.dae'))
sys.exit(0 if ok else 0)
