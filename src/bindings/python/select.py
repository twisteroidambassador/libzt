import libzt


def select(r, w, x, timeout=None):
    r, w, x = libzt.zts_py_select(r, w, x, timeout)
    return r, w, x
