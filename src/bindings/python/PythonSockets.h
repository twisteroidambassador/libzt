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

#ifndef ZTS_PYTHON_SOCKETS_H
#define ZTS_PYTHON_SOCKETS_H

#ifdef ZTS_ENABLE_PYTHON
#include "Python.h"

typedef int SOCKET_T;

PyObject* zts_py_sockaddr_to_tuple(struct zts_sockaddr* addr);

int zts_py_bind(int fd, int family, int type, PyObject* addro);

int zts_py_connect(int fd, int family, int type, PyObject* addro);

PyObject* zts_py_accept(int fd);

PyObject* zts_py_recv(int fd, size_t len, int flags);

PyObject* zts_py_recv_into(int fd, PyObject* buf, size_t nbytes, int flags);

PyObject* zts_py_recvfrom(int fd, size_t len, int flags);

PyObject* zts_py_recvfrom_into(int fd, PyObject* buf, size_t nbytes, int flags);

PyObject* zts_py_send(int fd, PyObject* buf, int flags);

int zts_py_sendall(int fd, PyObject* bytes, int flags);

PyObject* zts_py_sendto(int fd, int family, PyObject* buf, int flags, PyObject* addr_obj);

int zts_py_close(int fd);

PyObject* zts_py_addr_get_str(uint64_t net_id, int family);

PyObject* zts_py_select(PyObject* rlist, PyObject* wlist, PyObject* xlist, PyObject* timeout_obj);

int zts_py_setsockopt(int fd, PyObject* args);

PyObject* zts_py_getsockopt(int fd, PyObject* args);

int zts_py_settimeout(int fd, PyObject* value);

PyObject* zts_py_gettimeout(int fd);

PyObject* zts_py_get_native_errno();

PyObject* zts_py_getpeername(int fd);

PyObject* zts_py_getsockname(int fd);

PyObject* zts_py_inet_pton(int family, const char* src_string);

PyObject* zts_py_ioctl(int fd, unsigned int code, PyObject* ob_arg, int mutate_arg);

PyObject* zts_py_fcntl(int fd, int code, PyObject* arg);

#endif   // ZTS_ENABLE_PYTHON

#endif   // ZTS_PYTHON_SOCKETS_H
