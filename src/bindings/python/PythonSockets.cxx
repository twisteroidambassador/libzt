/*
 * Copyright (c)2013-2021 ZeroTier, Inc.
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file in the project's root directory.
 *
 * Change Date: 2026-01-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2.0 of the Apache License.
 */
/****/

/**
 * @file
 *
 * ZeroTier Socket API (Python)
 *
 * This code derives from the Python standard library:
 *
 * Lib/socket.py
 * Modules/socketmodule.c
 * Modules/fcntlmodule.c
 * Modules/clinic/fcntlmodule.c.h
 * Modules/clinic/selectmodule.c.h
 *
 * Copyright and license text can be found in pypi packaging directory.
 *
 */

#include "ZeroTierSockets.h"

#ifdef ZTS_ENABLE_PYTHON

#include "Python.h"
#include "PythonSockets.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "structmember.h"   // PyMemberDef

#include <errno.h>
#include <string.h>
#include <sys/time.h>

PyObject* set_error(void)
{
    // Assumes ZTS errno are equal to OS errno
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;   // PyErr_SetFromErrno(zts_errno);
}

static int zts_py_tuple_to_sockaddr(int family, PyObject* addr_obj, struct zts_sockaddr* dst_addr, zts_socklen_t* addrlen)
{
    if (family == ZTS_AF_INET) {
        struct zts_sockaddr_in* addr;
        if (*addrlen < sizeof(*addr)) {
            return ZTS_ERR_ARG;
        }
        if (! PyTuple_Check(addr_obj)) {
            return ZTS_ERR_ARG;
        }
        char* host_str;
        int port;
        if (! PyArg_ParseTuple(addr_obj, "eti:zts_py_tuple_to_sockaddr", "idna", &host_str, &port)) {
            return ZTS_ERR_ARG;
        }
        addr = (struct zts_sockaddr_in*)dst_addr;
        int result = zts_inet_pton(ZTS_AF_INET, host_str, &(addr->sin_addr));
        PyMem_Free(host_str);
        if (port < 0 || port > 0xFFFF) {
            return ZTS_ERR_ARG;
        }
        if (result != 1) {
            return ZTS_ERR_ARG;
        }
        addr->sin_family = AF_INET;
        addr->sin_port = lwip_htons((short)port);
        addr->sin_len = sizeof(*addr);
        *addrlen = sizeof(*addr);
        return ZTS_ERR_OK;
    }
    if (family == ZTS_AF_INET6) {
        struct zts_sockaddr_in6* addr;
        if (*addrlen < sizeof(*addr)) {
            return ZTS_ERR_ARG;
        }
        if (! PyTuple_Check(addr_obj)) {
            return ZTS_ERR_ARG;
        }
        char* host_str;
        int port;
        uint32_t flowinfo = 0, scope_id = 0;
        if (! PyArg_ParseTuple(addr_obj, "eti|II:zts_py_tuple_to_sockaddr", "idna", &host_str, &port, &flowinfo, &scope_id)) {
            return ZTS_ERR_ARG;
        }
        addr = (struct zts_sockaddr_in6*)dst_addr;
        int result = zts_inet_pton(ZTS_AF_INET6, host_str, &(addr->sin6_addr));
        PyMem_Free(host_str);
        if (port < 0 || port > 0xFFFF) {
            return ZTS_ERR_ARG;
        }
        if (result != 1) {
            return ZTS_ERR_ARG;
        }
        addr->sin6_family = AF_INET6;
        addr->sin6_port = lwip_htons((short)port);
        addr->sin6_flowinfo = lwip_htonl(flowinfo);
        addr->sin6_scope_id = scope_id;
        addr->sin6_len = sizeof(*addr);
        *addrlen = sizeof *addr;
        return ZTS_ERR_OK;
    }
    return ZTS_ERR_ARG;
}

PyObject* zts_py_sockaddr_to_tuple(struct zts_sockaddr* addr) {
    if (addr->sa_family == ZTS_AF_INET) {
        struct zts_sockaddr_in* addr_in = (zts_sockaddr_in*) addr;
        char ipstr[ZTS_INET_ADDRSTRLEN] = { 0 };
        zts_inet_ntop(ZTS_AF_INET, &(addr_in->sin_addr), ipstr, ZTS_INET_ADDRSTRLEN);
        PyObject* t = Py_BuildValue("(sl)", ipstr, lwip_ntohs(addr_in->sin_port));
        return t;
    }
    if (addr->sa_family == ZTS_AF_INET6) {
        struct zts_sockaddr_in6* addr_in6 = (zts_sockaddr_in6*) addr;
        char ipstr[ZTS_INET6_ADDRSTRLEN] = { 0 };
        zts_inet_ntop(ZTS_AF_INET6, &(addr_in6->sin6_addr), ipstr, ZTS_INET6_ADDRSTRLEN);
        PyObject* t = Py_BuildValue(
            "(slII)", ipstr, lwip_ntohs(addr_in6->sin6_port),
            lwip_ntohl(addr_in6->sin6_flowinfo), addr_in6->sin6_scope_id);
        return t;
    }
    if (addr->sa_family == 0) {
        Py_RETURN_NONE;
    }
    PyErr_SetString(PyExc_TypeError, "unknown address family");
    return NULL;
}

PyObject* zts_py_accept(int fd)
{
    struct zts_sockaddr_storage addrbuf = { 0 };
    socklen_t addrlen = sizeof(addrbuf);
    int err;
    Py_BEGIN_ALLOW_THREADS;
    err = zts_bsd_accept(fd, (struct zts_sockaddr*)&addrbuf, &addrlen);
    Py_END_ALLOW_THREADS;
    if (err < 0) {
        return set_error();
    }
    PyObject* t = Py_BuildValue("(iN)", err, zts_py_sockaddr_to_tuple((struct zts_sockaddr*)&addrbuf));
    return t;
}

int zts_py_bind(int fd, int family, int type, PyObject* addr_obj)
{
    struct zts_sockaddr_storage addrbuf;
    zts_socklen_t addrlen = sizeof(addrbuf);
    int err;
    if (zts_py_tuple_to_sockaddr(family, addr_obj, (struct zts_sockaddr*)&addrbuf, &addrlen) != ZTS_ERR_OK) {
        return ZTS_ERR_ARG;
    }
    Py_BEGIN_ALLOW_THREADS;
    err = zts_bsd_bind(fd, (struct zts_sockaddr*)&addrbuf, addrlen);
    Py_END_ALLOW_THREADS;
    return err;
}

int zts_py_connect(int fd, int family, int type, PyObject* addr_obj)
{
    struct zts_sockaddr_storage addrbuf;
    zts_socklen_t addrlen = sizeof(addrbuf);
    int err;
    if (zts_py_tuple_to_sockaddr(family, addr_obj, (struct zts_sockaddr*)&addrbuf, &addrlen) != ZTS_ERR_OK) {
        return ZTS_ERR_ARG;
    }
    Py_BEGIN_ALLOW_THREADS;
    err = zts_bsd_connect(fd, (struct zts_sockaddr*)&addrbuf, addrlen);
    Py_END_ALLOW_THREADS;
    return err;
}

PyObject* zts_py_recv(int fd, size_t len, int flags)
{
    PyObject *t, *buf;
    ssize_t bytes_read;

    buf = PyBytes_FromStringAndSize((char*)0, len);
    if (buf == NULL) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS;
    bytes_read = zts_bsd_recv(fd, PyBytes_AS_STRING(buf), len, flags);
    Py_END_ALLOW_THREADS;
    t = PyTuple_New(2);
    PyTuple_SetItem(t, 0, PyLong_FromLong(bytes_read));

    if (bytes_read < 0) {
        Py_DECREF(buf);
        Py_INCREF(Py_None);
        PyTuple_SetItem(t, 1, Py_None);
        return t;
    }

    if (bytes_read != len) {
        _PyBytes_Resize(&buf, bytes_read);
    }

    PyTuple_SetItem(t, 1, buf);
    return t;
}

PyObject* zts_py_recv_into(int fd, PyObject* buf, size_t nbytes, int flags) {
    Py_buffer pbuf;
    if (PyObject_GetBuffer(buf, &pbuf, PyBUF_SIMPLE|PyBUF_WRITABLE) != 0) {
        return NULL;
    }
    if (nbytes > pbuf.len) {
        PyBuffer_Release(&pbuf);
        PyErr_SetString(PyExc_ValueError,
                        "buffer too small for requested bytes");
        return NULL;
    }
    if (nbytes == 0) {
        nbytes = pbuf.len;
    }
    ssize_t bytes_read;
    Py_BEGIN_ALLOW_THREADS;
    bytes_read = zts_bsd_recv(fd, pbuf.buf, nbytes, flags);
    Py_END_ALLOW_THREADS;
    PyBuffer_Release(&pbuf);
    return PyLong_FromSsize_t(bytes_read);
}

PyObject* zts_py_recvfrom(int fd, size_t len, int flags)
{
    ssize_t bytes_read;
    struct zts_sockaddr_storage addr;
    zts_socklen_t addrlen = sizeof(addr);

    PyObject* buf = PyBytes_FromStringAndSize(nullptr, len);
    if (buf == NULL) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS;
    bytes_read = zts_bsd_recvfrom(
        fd, PyBytes_AS_STRING(buf), len, flags, reinterpret_cast<struct zts_sockaddr*>(&addr), &addrlen);
    Py_END_ALLOW_THREADS;

    if (bytes_read < 0) {
        Py_DECREF(buf);
        return Py_BuildValue("nss", bytes_read, NULL, NULL);
    }

    if (bytes_read != len) {
        _PyBytes_Resize(&buf, bytes_read);
    }

    return Py_BuildValue(
        "nNN", bytes_read, buf, zts_py_sockaddr_to_tuple(reinterpret_cast<struct zts_sockaddr*>(&addr)));
}

PyObject* zts_py_recvfrom_into(int fd, PyObject* buf, size_t nbytes, int flags) {
    Py_buffer pbuf;
    if (PyObject_GetBuffer(buf, &pbuf, PyBUF_SIMPLE|PyBUF_WRITABLE) != 0) {
        return NULL;
    }
    if (nbytes > pbuf.len) {
        PyBuffer_Release(&pbuf);
        PyErr_SetString(PyExc_ValueError,
                        "buffer too small for requested bytes");
        return NULL;
    }
    if (nbytes == 0) {
        nbytes = pbuf.len;
    }

    ssize_t bytes_read;
    struct zts_sockaddr_storage addr;
    zts_socklen_t addrlen = sizeof(addr);
    Py_BEGIN_ALLOW_THREADS;
    bytes_read = zts_bsd_recv(fd, pbuf.buf, nbytes, flags);
    bytes_read = zts_bsd_recvfrom(
        fd, pbuf.buf, nbytes, flags, reinterpret_cast<struct zts_sockaddr*>(&addr), &addrlen);
    Py_END_ALLOW_THREADS;
    PyBuffer_Release(&pbuf);
    if (bytes_read < 0) {
        return Py_BuildValue("ns", bytes_read, NULL);
    }
    return Py_BuildValue("nN", bytes_read, zts_py_sockaddr_to_tuple(reinterpret_cast<struct zts_sockaddr*>(&addr)));;
}

PyObject* zts_py_send(int fd, PyObject* buf, int flags)
{
    Py_buffer output;
    int bytes_sent;

    if (PyObject_GetBuffer(buf, &output, PyBUF_SIMPLE) != 0) {
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS;
    bytes_sent = zts_bsd_send(fd, output.buf, output.len, flags);
    Py_END_ALLOW_THREADS;
    PyBuffer_Release(&output);

    return PyLong_FromSsize_t(bytes_sent);
}

int zts_py_sendall(int fd, PyObject* bytes, int flags)
{
    int res;
    Py_buffer output;

    char *buf;
    int bytes_left;

    int has_timeout;
    int deadline_initialized = 0;

    _PyTime_t timeout;  // Timeout duration
    _PyTime_t interval;  // Time remaining until deadline
    _PyTime_t deadline;  // System clock deadline for timeout

    if (PyObject_GetBuffer(bytes, &output, PyBUF_SIMPLE) != 0) {
        // BufferError has been raised. No need to set our own error.
        res = ZTS_ERR_OK;
        goto done;
    }

    buf = (char *) output.buf;
    bytes_left = output.len;

    res = zts_get_send_timeout(fd);
    if (res < 0)
        goto done;

    timeout = (_PyTime_t) 1000 * 1000 * (int64_t) res; // Convert ms to ns
    interval = timeout;
    has_timeout = (interval > 0);

    /* Call zts_bsd_send() until no more bytes left to send in the buffer.
    Keep track of remaining time until timeout and exit with ZTS_ETIMEDOUT if timeout exceeded.
    Check signals between calls to send() to prevent undue blocking.*/
    do {
        if (has_timeout) {
            if (deadline_initialized) {
                interval = deadline - _PyTime_GetMonotonicClock();
            } else {
                deadline_initialized = 1;
                deadline = _PyTime_GetMonotonicClock() + timeout;
            }

            if (interval <= 0) {
                zts_errno = ZTS_ETIMEDOUT;
                res = ZTS_ERR_SOCKET;
                goto done;
            }
        }

        Py_BEGIN_ALLOW_THREADS;
        res = zts_bsd_send(fd, buf, bytes_left, flags);
        Py_END_ALLOW_THREADS;
        if (res < 0)
            goto done;

        int bytes_sent = res;
        assert(bytes_sent > 0);

        buf += bytes_sent;  // Advance pointer
        bytes_left -= bytes_sent;

        if (PyErr_CheckSignals())  // Handle interrupts, etc.
            goto done;

    } while (bytes_left > 0);

    res = ZTS_ERR_OK;  // Success

done:
    if (output.obj != NULL)
        PyBuffer_Release(&output);
    return res;
}

PyObject* zts_py_sendto(int fd, int family, PyObject* buf, int flags, PyObject* addr_obj)
{
    Py_buffer data_to_send;
    /* PyObject_GetBuffer raises BufferError on failure, so this method must be able to raise
     * an exception, otherwise Python only sees a SystemError.
     * That's why its return type is PyObject*.
     * */
    if (PyObject_GetBuffer(buf, &data_to_send, PyBUF_SIMPLE) != 0) {
        return NULL;
    }

    struct zts_sockaddr_storage addrbuf;
    zts_socklen_t addrlen = sizeof(addrbuf);
    if (zts_py_tuple_to_sockaddr(family, addr_obj, reinterpret_cast<struct zts_sockaddr*>(&addrbuf), &addrlen) != ZTS_ERR_OK) {
        PyErr_SetString(PyExc_TypeError, "Invalid address");
        return NULL;
    }
    ssize_t bytes_sent;
    Py_BEGIN_ALLOW_THREADS;
    bytes_sent = zts_bsd_sendto(fd, data_to_send.buf, data_to_send.len, flags, reinterpret_cast<struct zts_sockaddr*>(&addrbuf), addrlen);
    Py_END_ALLOW_THREADS;
    PyBuffer_Release(&data_to_send);

    /* Error handling for the sendto call is left for Python side */
    return PyLong_FromSsize_t(bytes_sent);
}

int zts_py_close(int fd)
{
    int err;
    Py_BEGIN_ALLOW_THREADS;
    err = zts_bsd_close(fd);
    Py_END_ALLOW_THREADS;
    return err;
}

PyObject* zts_py_addr_get_str(uint64_t net_id, int family)
{
    char addr_str[ZTS_IP_MAX_STR_LEN] = { 0 };
    if (zts_addr_get_str(net_id, family, addr_str, ZTS_IP_MAX_STR_LEN) < 0) {
        PyErr_SetString(PyExc_Warning, "No address of the given type has been assigned by the network");
        return NULL;
    }
    PyObject* t = PyUnicode_FromString(addr_str);
    return t;
}

/* list of Python objects and their file descriptor */
typedef struct {
    PyObject* obj; /* owned reference */
    int fd;
    int sentinel; /* -1 == sentinel */
} pylist;

void reap_obj(pylist fd2obj[ZTS_FD_SETSIZE + 1])
{
    unsigned int i;
    for (i = 0; i < (unsigned int)ZTS_FD_SETSIZE + 1 && fd2obj[i].sentinel >= 0; i++) {
        Py_CLEAR(fd2obj[i].obj);
    }
    fd2obj[0].sentinel = -1;
}

/* returns NULL and sets the Python exception if an error occurred */
PyObject* set2list(zts_fd_set* set, pylist fd2obj[ZTS_FD_SETSIZE + 1])
{
    int i, j, count = 0;
    PyObject *list, *o;
    int fd;

    for (j = 0; fd2obj[j].sentinel >= 0; j++) {
        if (ZTS_FD_ISSET(fd2obj[j].fd, set)) {
            count++;
        }
    }
    list = PyList_New(count);
    if (! list) {
        return NULL;
    }

    i = 0;
    for (j = 0; fd2obj[j].sentinel >= 0; j++) {
        fd = fd2obj[j].fd;
        if (ZTS_FD_ISSET(fd, set)) {
            o = fd2obj[j].obj;
            fd2obj[j].obj = NULL;
            /* transfer ownership */
            if (PyList_SetItem(list, i, o) < 0) {
                goto finally;
            }
            i++;
        }
    }
    return list;
finally:
    Py_DECREF(list);
    return NULL;
}

/* returns -1 and sets the Python exception if an error occurred, otherwise
   returns a number >= 0
*/
int seq2set(PyObject* seq, zts_fd_set* set, pylist fd2obj[ZTS_FD_SETSIZE + 1])
{
    int max = -1;
    unsigned int index = 0;
    Py_ssize_t i;
    PyObject* fast_seq = NULL;
    PyObject* o = NULL;

    fd2obj[0].obj = (PyObject*)0; /* set list to zero size */
    ZTS_FD_ZERO(set);

    fast_seq = PySequence_Fast(seq, "arguments 1-3 must be sequences");
    if (! fast_seq) {
        return -1;
    }

    for (i = 0; i < PySequence_Fast_GET_SIZE(fast_seq); i++) {
        int v;

        /* any intervening fileno() calls could decr this refcnt */
        if (! (o = PySequence_Fast_GET_ITEM(fast_seq, i))) {
            goto finally;
        }

        Py_INCREF(o);
        v = PyObject_AsFileDescriptor(o);
        if (v == -1) {
            goto finally;
        }

        if (v > max) {
            max = v;
        }
        ZTS_FD_SET(v, set);

        /* add object and its file descriptor to the list */
        if (index >= (unsigned int)ZTS_FD_SETSIZE) {
            PyErr_SetString(PyExc_ValueError, "too many file descriptors in select()");
            goto finally;
        }
        fd2obj[index].obj = o;
        fd2obj[index].fd = v;
        fd2obj[index].sentinel = 0;
        fd2obj[++index].sentinel = -1;
    }
    Py_DECREF(fast_seq);
    return max + 1;

finally:
    Py_XDECREF(o);
    Py_DECREF(fast_seq);
    return -1;
}

PyObject* zts_py_select(PyObject* rlist, PyObject* wlist, PyObject* xlist, PyObject* timeout_obj)
{
    pylist rfd2obj[ZTS_FD_SETSIZE + 1];
    pylist wfd2obj[ZTS_FD_SETSIZE + 1];
    pylist efd2obj[ZTS_FD_SETSIZE + 1];
    PyObject* ret = NULL;
    zts_fd_set ifdset, ofdset, efdset;
    struct timeval tv, *tvp;
    int imax, omax, emax, max;
    int n;
    _PyTime_t timeout, deadline = 0;

    if (timeout_obj == Py_None) {
        tvp = (struct timeval*)NULL;
    }
    else {
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION <= 5
        _PyTime_round_t roundingMode = _PyTime_ROUND_CEILING;
#else
        _PyTime_round_t roundingMode = _PyTime_ROUND_UP;
#endif
        if (_PyTime_FromSecondsObject(&timeout, timeout_obj, roundingMode) < 0) {
            if (PyErr_ExceptionMatches(PyExc_TypeError)) {
                PyErr_SetString(PyExc_TypeError, "timeout must be a float or None");
            }
            return NULL;
        }
        if (_PyTime_AsTimeval(timeout, &tv, roundingMode) == -1) {
            return NULL;
        }
        if (tv.tv_sec < 0) {
            PyErr_SetString(PyExc_ValueError, "timeout must be non-negative");
            return NULL;
        }
        tvp = &tv;
    }
    /* Convert iterables to zts_fd_sets, and get maximum fd number
     * propagates the Python exception set in seq2set()
     */
    rfd2obj[0].sentinel = -1;
    wfd2obj[0].sentinel = -1;
    efd2obj[0].sentinel = -1;
    if ((imax = seq2set(rlist, &ifdset, rfd2obj)) < 0) {
        goto finally;
    }
    if ((omax = seq2set(wlist, &ofdset, wfd2obj)) < 0) {
        goto finally;
    }
    if ((emax = seq2set(xlist, &efdset, efd2obj)) < 0) {
        goto finally;
    }

    max = imax;
    if (omax > max) {
        max = omax;
    }
    if (emax > max) {
        max = emax;
    }
    if (tvp) {
        deadline = _PyTime_GetMonotonicClock() + timeout;
    }

    Py_BEGIN_ALLOW_THREADS;
    /* zts_bsd_* methods are supposed to set zts_errno.
     * In fact they also set the regular errno, and zts_errno is not thread safe,
     * so might as well just use errno.
     * But lwIP never sets EINTR anyways.
     * */
    errno = 0;
    // struct zts_timeval zts_tvp;
    // zts_tvp.tv_sec = tvp.tv_sec;
    // zts_tvp.tv_sec = tvp.tv_sec;

    n = zts_bsd_select(max, &ifdset, &ofdset, &efdset, (struct zts_timeval*)tvp);
    Py_END_ALLOW_THREADS;

    if (n < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
    }
    else {
        /* any of these three calls can raise an exception.  it's more
           convenient to test for this after all three calls... but
           is that acceptable?
        */
        rlist = set2list(&ifdset, rfd2obj);
        wlist = set2list(&ofdset, wfd2obj);
        xlist = set2list(&efdset, efd2obj);
        if (PyErr_Occurred()) {
            ret = NULL;
        }
        else {
            ret = PyTuple_Pack(3, rlist, wlist, xlist);
        }
        Py_XDECREF(rlist);
        Py_XDECREF(wlist);
        Py_XDECREF(xlist);
    }

finally:
    reap_obj(rfd2obj);
    reap_obj(wfd2obj);
    reap_obj(efd2obj);
    return ret;
}

int zts_py_setsockopt(int fd, PyObject* args)
{
    int level;
    int optname;
    int res;
    Py_buffer optval;
    int flag;
    unsigned int optlen;
    PyObject* none;

    // setsockopt(level, opt, flag)
    if (PyArg_ParseTuple(args, "iii:setsockopt", &level, &optname, &flag)) {
        res = zts_bsd_setsockopt(fd, level, optname, (char*)&flag, sizeof flag);
        goto done;
    }

    PyErr_Clear();
    // setsockopt(level, opt, None, flag)
    if (PyArg_ParseTuple(args, "iiO!I:setsockopt", &level, &optname, Py_TYPE(Py_None), &none, &optlen)) {
        assert(sizeof(socklen_t) >= sizeof(unsigned int));
        res = zts_bsd_setsockopt(fd, level, optname, NULL, (socklen_t)optlen);
        goto done;
    }

    PyErr_Clear();
    // setsockopt(level, opt, buffer)
    if (! PyArg_ParseTuple(args, "iiy*:setsockopt", &level, &optname, &optval)) {
        return (int)NULL;
    }

#ifdef MS_WINDOWS
    if (optval.len > INT_MAX) {
        PyBuffer_Release(&optval);
        PyErr_Format(PyExc_OverflowError, "socket option is larger than %i bytes", INT_MAX);
        return (int)NULL;
    }
    res = zts_bsd_setsockopt(fd, level, optname, optval.buf, (int)optval.len);
#else
    res = zts_bsd_setsockopt(fd, level, optname, optval.buf, optval.len);
#endif
    PyBuffer_Release(&optval);

done:
    return res;
}

int zts_py_settimeout(int fd, PyObject* value)
{
    int res;

    // If None, set blocking mode
    if (value == Py_None) {
        res = zts_set_blocking(fd, true);
        return res;
    }

    double double_val = PyFloat_AsDouble(value);

    // If negative, throw an error
    if (double_val < 0.) {
        return ZTS_ERR_ARG;
    }

    // Set non-blocking mode
    res = zts_set_blocking(fd, false);
    if (res < 0) {
        return res;
    }

    // Calculate timeout secs/microseconds
    int total_micros = (int) floor(double_val * 1e6);
    div_t div_res = div(total_micros, 1000);

    int secs = div_res.quot;
    int micros = div_res.rem;

    // Set both send and recv timeouts
    res = zts_set_send_timeout(fd, secs, micros);
    if (res < 0) {
        return res;
    }
    res = zts_set_recv_timeout(fd, secs, micros);
    return res;
}

PyObject* zts_py_gettimeout(int fd)
{
    PyObject *t;
    int res;

    t = PyTuple_New(2);

    res = zts_get_blocking(fd);

    // If err, return (err, None)
    if (res < 0) {
        PyTuple_SetItem(t, 0, PyLong_FromLong((long) res));
        Py_INCREF(Py_None);
        PyTuple_SetItem(t, 1, Py_None);
        return t;
    }

    // If socket in blocking mode, return (0, None)
    if (res == 1) {
        PyTuple_SetItem(t, 0, PyLong_FromLong(0L));
        Py_INCREF(Py_None);
        PyTuple_SetItem(t, 1, Py_None);
        return t;
    }

    // Send and recv timeouts should be equal
    res = zts_get_recv_timeout(fd);

    // If err, return (err, None)
    if (res < 0) {
        PyTuple_SetItem(t, 0, PyLong_FromLong((long) res));
        Py_INCREF(Py_None);
        PyTuple_SetItem(t, 1, Py_None);
        return t;
    }

    // Return (0, timeout)
    // Result of zts_get_recv_timeout() is in milliseconds
    double timeout = 1e-3 * (double) res;
    PyTuple_SetItem(t, 0, PyLong_FromLong(0L));
    PyTuple_SetItem(t, 1, PyFloat_FromDouble(timeout));
    return t;
}

PyObject* zts_py_getsockopt(int fd, PyObject* args)
{
    int level;
    int optname;
    int res;
    PyObject* buf;
    socklen_t buflen = 0;
    int flag = 0;
    socklen_t flagsize;

    if (! PyArg_ParseTuple(args, "ii|i:getsockopt", &level, &optname, &buflen)) {
        return NULL;
    }
    if (buflen == 0) {
        flagsize = sizeof flag;
        res = zts_bsd_getsockopt(fd, level, optname, (void*)&flag, &flagsize);
        if (res < 0) {
            return set_error();
        }
        return PyLong_FromLong(flag);
    }
    if (buflen <= 0 || buflen > 1024) {
        PyErr_SetString(PyExc_OSError, "getsockopt buflen out of range");
        return NULL;
    }
    buf = PyBytes_FromStringAndSize((char*)NULL, buflen);
    if (buf == NULL) {
        return NULL;
    }
    res = zts_bsd_getsockopt(fd, level, optname, (void*)PyBytes_AS_STRING(buf), &buflen);
    if (res < 0) {
        Py_DECREF(buf);
        return set_error();
    }
    _PyBytes_Resize(&buf, buflen);
    return buf;
}

PyObject* zts_py_fcntl(int fd, int code, PyObject* arg)
{
    unsigned int int_arg = 0;
    int ret;

    if (arg != NULL) {
        int parse_result;
        PyErr_Clear();
        parse_result = PyArg_Parse(
            arg,
            "I;fcntl requires a file or file descriptor,"
            " an integer and optionally a third integer",
            &int_arg);
        if (! parse_result) {
            return NULL;
        }
    }
    do {
        Py_BEGIN_ALLOW_THREADS;
        ret = zts_bsd_fcntl(fd, code, (int)int_arg);
        Py_END_ALLOW_THREADS;
    } while (ret == -1 && zts_errno == ZTS_EINTR);
    if (ret < 0) {
        return set_error();
    }
    return PyLong_FromLong((long)ret);
}

PyObject* zts_py_ioctl(int fd, unsigned int code, PyObject* ob_arg, int mutate_arg)
{
#define IOCTL_BUFSZ 1024
    int arg = 0;
    int ret;
    Py_buffer pstr;
    char* str;
    Py_ssize_t len;
    char buf[IOCTL_BUFSZ + 1]; /* argument plus NUL byte */

    if (ob_arg != NULL) {
        if (PyArg_Parse(ob_arg, "w*:ioctl", &pstr)) {
            char* arg;
            str = (char*)pstr.buf;
            len = pstr.len;

            if (mutate_arg) {
                if (len <= IOCTL_BUFSZ) {
                    memcpy(buf, str, len);
                    buf[len] = '\0';
                    arg = buf;
                }
                else {
                    arg = str;
                }
            }
            else {
                if (len > IOCTL_BUFSZ) {
                    PyBuffer_Release(&pstr);
                    PyErr_SetString(PyExc_ValueError, "ioctl string arg too long");
                    return NULL;
                }
                else {
                    memcpy(buf, str, len);
                    buf[len] = '\0';
                    arg = buf;
                }
            }
            if (buf == arg) {
                Py_BEGIN_ALLOW_THREADS;
                /* think array.resize() */
                ret = zts_bsd_ioctl(fd, code, arg);
                Py_END_ALLOW_THREADS;
            }
            else {
                ret = zts_bsd_ioctl(fd, code, arg);
            }
            if (mutate_arg && (len <= IOCTL_BUFSZ)) {
                memcpy(str, buf, len);
            }
            PyBuffer_Release(&pstr); /* No further access to str below this point */
            if (ret < 0) {
                PyErr_SetFromErrno(PyExc_OSError);
                return NULL;
            }
            if (mutate_arg) {
                return PyLong_FromLong(ret);
            }
            else {
                return PyBytes_FromStringAndSize(buf, len);
            }
        }

        PyErr_Clear();
        if (PyArg_Parse(ob_arg, "s*:ioctl", &pstr)) {
            str = (char*)pstr.buf;
            len = pstr.len;
            if (len > IOCTL_BUFSZ) {
                PyBuffer_Release(&pstr);
                PyErr_SetString(PyExc_ValueError, "ioctl string arg too long");
                return NULL;
            }
            memcpy(buf, str, len);
            buf[len] = '\0';
            Py_BEGIN_ALLOW_THREADS;
            ret = zts_bsd_ioctl(fd, code, buf);
            Py_END_ALLOW_THREADS;
            if (ret < 0) {
                PyBuffer_Release(&pstr);
                PyErr_SetFromErrno(PyExc_OSError);
                return NULL;
            }
            PyBuffer_Release(&pstr);
            return PyBytes_FromStringAndSize(buf, len);
        }

        PyErr_Clear();
        if (! PyArg_Parse(
                ob_arg,
                "i;ioctl requires a file or file descriptor,"
                " an integer and optionally an integer or buffer argument",
                &arg)) {
            return NULL;
        }
        // Fall-through to outside the 'if' statement.
    }
    // TODO: Double check that &arg is correct
    Py_BEGIN_ALLOW_THREADS;
    ret = zts_bsd_ioctl(fd, code, &arg);
    Py_END_ALLOW_THREADS;
    if (ret < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    }
    return PyLong_FromLong((long)ret);
#undef IOCTL_BUFSZ
}

PyObject* zts_py_get_native_errno() {
    return Py_BuildValue("i", errno);
}

PyObject* zts_py_getpeername(int fd) {
    int err;
    struct zts_sockaddr_storage sa;
    zts_socklen_t len = sizeof(sa);
    Py_BEGIN_ALLOW_THREADS;
    err = zts_bsd_getpeername(fd, reinterpret_cast<struct zts_sockaddr*>(&sa), &len);
    Py_END_ALLOW_THREADS;
    if (err != ZTS_ERR_OK) {
        return Py_BuildValue("is", err, NULL);
    }
    return Py_BuildValue("iN", err, zts_py_sockaddr_to_tuple(reinterpret_cast<struct zts_sockaddr*>(&sa)));
}

PyObject* zts_py_getsockname(int fd) {
    int err;
    struct zts_sockaddr_storage sa;
    zts_socklen_t len = sizeof(sa);
    Py_BEGIN_ALLOW_THREADS;
    err = zts_bsd_getsockname(fd, reinterpret_cast<struct zts_sockaddr*>(&sa), &len);
    Py_END_ALLOW_THREADS;
    if (err != ZTS_ERR_OK) {
        return Py_BuildValue("is", err, NULL);
    }
    return Py_BuildValue("iN", err, zts_py_sockaddr_to_tuple(reinterpret_cast<struct zts_sockaddr*>(&sa)));
}

PyObject* zts_py_inet_pton(int family, const char* src_string) {
    PyObject* packed;
    if (family == ZTS_AF_INET) {
        packed = PyBytes_FromStringAndSize(NULL, sizeof(ip4_addr_t));
    }
    else if (family == ZTS_AF_INET6) {
        packed = PyBytes_FromStringAndSize(NULL, sizeof(ip6_addr_t::addr));
    }
    else {
        PyErr_SetString(PyExc_OSError, "unknown address family");
        return NULL;
    }

    if (packed == NULL) {
        return NULL;
    }
    if (zts_inet_pton(family, src_string, PyBytes_AsString(packed)) != 1) {
        Py_DECREF(packed);
        PyErr_SetString(PyExc_ValueError, "Invalid IP address");
        return NULL;
    }
    return packed;
}

#endif   // ZTS_ENABLE_PYTHON
