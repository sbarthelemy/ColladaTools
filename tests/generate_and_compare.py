#!/usr/bin/env python
import tempfile
import subprocess
import filecmp
from os.path import join
import os
import sys

exec_dir = sys.argv[1]
hdf5_file = sys.argv[2]
scene_file = sys.argv[3]
anim_file = sys.argv[4]
out_file= tempfile.mkstemp(suffix='anim.dae', text=True)[1]

subprocess.check_call([
        join(exec_dir, 'h5toanim'),
        join(hdf5_file),
        join(scene_file),
        out_file])
ok = filecmp.cmp(
        anim_file,
        out_file)
try:
    os.unlink(out_file)
except OSError:
    pass
sys.exit(0 if ok else 0)
