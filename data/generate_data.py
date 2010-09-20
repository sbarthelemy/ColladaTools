from numpy import *
import h5py

def lexicographic(shape):
    d = zeros((shape))
    if len(shape)==2:
        for i in range(shape[0]):
            for j in range(shape[1]):
                d[i,j] = 10 * i + j
    elif len(shape)==3:
        for i in range(shape[0]):
            for j in range(shape[1]):
                for k in range(shape[2]):
                    d[i,j,k] = 100*i + 10*j + k
    else:
        raise ValueError("len(shape) should be 2 or 3")
    return d

if True:
    file = h5py.File('elementary.h5', 'w')
    group = file
    group['timeline'] = linspace(0., 1., 10)
    group['matrix'] = lexicographic((10, 4, 4))
    group['matrix'].attrs["ArborisViewerType"] = "matrix"
    group['noattribute'] = lexicographic((10, 4, 4))
    group['wrongattribute'] = lexicographic((10, 4, 4))
    group['wrongattribute'].attrs["ArborisViewerType"] = "marix"
    group['wrongsize'] = lexicographic((3, 4, 4))
    group['wrongsize'].attrs["ArborisViewerType"] = "matrix"
    file.close()
