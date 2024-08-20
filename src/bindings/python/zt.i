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

%include "ZeroTierSockets.h"
%include "PythonSockets.h"

%include "carrays.i"
%array_functions(zts_ip_addr, zts_ip_addr_array);

%extend zts_net_info_t {
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
}