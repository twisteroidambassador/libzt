/* libzt.i */

%begin
%{
#define SWIG_PYTHON_CAST_MODE
%}

%include <stdint.i>

#define ZTS_ENABLE_PYTHON 1

%module(directors="1") libzt
%module libzt
%{
#include "ZeroTierSockets.h"
#include "PythonSockets.h"
%}

%feature("director") PythonDirectorCallbackClass;

%ignore zts_in6_addr;
%ignore zts_sockaddr;
%ignore zts_in_addr;
%ignore zts_sockaddr_in;
%ignore zts_sockaddr_storage;
%ignore zts_sockaddr_in6;

%ignore zts_linger;
%ignore zts_ip_mreq;
%ignore zts_in_pktinfo;
%ignore zts_ipv6_mreq;

%ignore zts_fd_set;
%ignore zts_pollfd;
%ignore zts_nfds_t;
%ignore zts_msghdr;

%apply (char *STRING, int LENGTH) { (const char* prefix, unsigned int len)}

%include "ZeroTierSockets.h"
%include "PythonSockets.h"

%extend zts_net_info_t {
    PyObject* get_all_assigned_addresses() {
        PyObject* addrs_list = PyList_New($self->assigned_addr_count);
        if (addrs_list == NULL) {
            return NULL;
        }
        for (unsigned int i = 0; i < $self->assigned_addr_count; i++) {
            PyObject* a = zts_py_sockaddr_to_tuple(reinterpret_cast<struct zts_sockaddr*> (&($self->assigned_addrs[i])));
            if (a == NULL) {
                Py_DECREF(addrs_list);
                return NULL;
            }
            PyList_SET_ITEM(addrs_list, i, a);
        }
        return addrs_list;
    }

    PyObject* get_all_routes() {
        PyObject* routes_list = PyList_New($self->route_count);
        if (routes_list == NULL) {
            /* Do I need to set an exception here? */
            return NULL;
        }
        for (unsigned int i = 0; i < $self->route_count; i++) {
            PyObject* t = Py_BuildValue(
                "(NNHH)", zts_py_sockaddr_to_tuple((struct zts_sockaddr*) &($self->routes[i].target)),
                zts_py_sockaddr_to_tuple((struct zts_sockaddr*) &($self->routes[i].via)),
                $self->routes[i].flags, $self->routes[i].metric);
            if (t == NULL) {
                Py_DECREF(routes_list);
                return NULL;
            }
            PyList_SET_ITEM(routes_list, i, t);
        }
        return routes_list;
    }

    PyObject* get_all_dns_addresses() {
        PyObject* dns_list = PyList_New(0);
        if (dns_list == NULL) {
            return NULL;
        }
        for (unsigned int i = 0; i < ZTS_MAX_DNS_SERVERS; i++) {
            PyObject* a = zts_py_sockaddr_to_tuple(reinterpret_cast<struct zts_sockaddr*> (&($self->dns_addresses[i])));
            if (a == NULL) {
                Py_DECREF(dns_list);
                return NULL;
            }
            else if (a == Py_None) {
                Py_DECREF(a);
            }
            else {
                int ret = PyList_Append(dns_list, a);
                Py_DECREF(a);
                if (ret != 0) {
                    Py_DECREF(dns_list);
                    return NULL;
                }
            }
        }
        return dns_list;
    }
}

%extend zts_addr_info_t {
    PyObject* get_address() {
        return zts_py_sockaddr_to_tuple(reinterpret_cast<struct zts_sockaddr*> (&($self->addr)));
    }
};