"""ZeroTier low-level socket interface"""
from enum import IntEnum

import libzt


class zts_errno(IntEnum):
    """Enum of socket error codes"""

    # Operation not permitted 
    ZTS_EPERM = libzt.ZTS_EPERM,
    # No such file or directory 
    ZTS_ENOENT = libzt.ZTS_ENOENT,
    # No such process 
    ZTS_ESRCH = libzt.ZTS_ESRCH,
    # Interrupted system call 
    ZTS_EINTR = libzt.ZTS_EINTR,
    # I/O error 
    ZTS_EIO = libzt.ZTS_EIO,
    # No such device or address 
    ZTS_ENXIO = libzt.ZTS_ENXIO,
    # Bad file number 
    ZTS_EBADF = libzt.ZTS_EBADF,
    # Try again 
    ZTS_EAGAIN = libzt.ZTS_EAGAIN,
    # Operation would block 
    ZTS_EWOULDBLOCK = libzt.ZTS_EWOULDBLOCK,
    # Out of memory 
    ZTS_ENOMEM = libzt.ZTS_ENOMEM,
    # Permission denied 
    ZTS_EACCES = libzt.ZTS_EACCES,
    # Bad address 
    ZTS_EFAULT = libzt.ZTS_EFAULT,
    # Device or resource busy 
    ZTS_EBUSY = libzt.ZTS_EBUSY,
    # File exists 
    ZTS_EEXIST = libzt.ZTS_EEXIST,
    # No such device 
    ZTS_ENODEV = libzt.ZTS_ENODEV,
    # Invalid argument 
    ZTS_EINVAL = libzt.ZTS_EINVAL,
    # File table overflow 
    ZTS_ENFILE = libzt.ZTS_ENFILE,
    # Too many open files 
    ZTS_EMFILE = libzt.ZTS_EMFILE,
    # Function not implemented 
    ZTS_ENOSYS = libzt.ZTS_ENOSYS,
    # Socket operation on non-socket 
    ZTS_ENOTSOCK = libzt.ZTS_ENOTSOCK,
    # Destination address required 
    ZTS_EDESTADDRREQ = libzt.ZTS_EDESTADDRREQ,
    # Message too long 
    ZTS_EMSGSIZE = libzt.ZTS_EMSGSIZE,
    # Protocol wrong type for socket 
    ZTS_EPROTOTYPE = libzt.ZTS_EPROTOTYPE,
    # Protocol not available 
    ZTS_ENOPROTOOPT = libzt.ZTS_ENOPROTOOPT,
    # Protocol not supported 
    ZTS_EPROTONOSUPPORT = libzt.ZTS_EPROTONOSUPPORT,
    # Socket type not supported 
    ZTS_ESOCKTNOSUPPORT = libzt.ZTS_ESOCKTNOSUPPORT,
    # Operation not supported on transport endpoint 
    ZTS_EOPNOTSUPP = libzt.ZTS_EOPNOTSUPP,
    # Protocol family not supported 
    ZTS_EPFNOSUPPORT = libzt.ZTS_EPFNOSUPPORT,
    # Address family not supported by protocol 
    ZTS_EAFNOSUPPORT = libzt.ZTS_EAFNOSUPPORT,
    # Address already in use 
    ZTS_EADDRINUSE = libzt.ZTS_EADDRINUSE,
    # Cannot assign requested address 
    ZTS_EADDRNOTAVAIL = libzt.ZTS_EADDRNOTAVAIL,
    # Network is down 
    ZTS_ENETDOWN = libzt.ZTS_ENETDOWN,
    # Network is unreachable 
    ZTS_ENETUNREACH = libzt.ZTS_ENETUNREACH,
    # Software caused connection abort 
    ZTS_ECONNABORTED = libzt.ZTS_ECONNABORTED,
    # Connection reset by peer 
    ZTS_ECONNRESET = libzt.ZTS_ECONNRESET,
    # No buffer space available 
    ZTS_ENOBUFS = libzt.ZTS_ENOBUFS,
    # Transport endpoint is already connected 
    ZTS_EISCONN = libzt.ZTS_EISCONN,
    # Transport endpoint is not connected 
    ZTS_ENOTCONN = libzt.ZTS_ENOTCONN,
    # Connection timed out 
    ZTS_ETIMEDOUT = libzt.ZTS_ETIMEDOUT,
    # Connection refused
    ZTS_ECONNREFUSED = libzt.ZTS_ECONNREFUSED,
    # No route to host 
    ZTS_EHOSTUNREACH = libzt.ZTS_EHOSTUNREACH,
    # Operation already in progress 
    ZTS_EALREADY = libzt.ZTS_EALREADY,
    # Operation now in progress 
    ZTS_EINPROGRESS = libzt.ZTS_EINPROGRESS


def raise_oserror_by_errno(sock_err):
    raise OSError(sock_err, zts_errno(sock_err).name + " (" + str(sock_err) + ")")


def handle_error(err):
    """Convert libzt error code to exception"""
    if err == libzt.ZTS_ERR_SOCKET:
        sock_err = errno()
        raise_oserror_by_errno(sock_err)
    if err == libzt.ZTS_ERR_SERVICE:
        raise RuntimeError("ZTS_ERR_SERVICE (" + str(err) + ")")
    if err == libzt.ZTS_ERR_ARG:
        raise TypeError("ZTS_ERR_ARG (" + str(err) + ")")
    # ZTS_ERR_NO_RESULT isn't strictly an error
    # if (err == libzt.ZTS_ERR_NO_RESULT):
    #   raise Exception('ZTS_ERR_NO_RESULT (' + err + ')')
    if err == libzt.ZTS_ERR_GENERAL:
        raise Exception("ZTS_ERR_GENERAL (" + str(err) + ")")


# This uses the OS's native errno, which should be thread safe
def errno():
    """Return errno value of low-level socket layer"""
    return libzt.zts_py_get_native_errno()

def has_dualstack_ipv6(self):
    """Return whether libzt supports dual stack sockets: yes"""
    return True

def socketpair(family=libzt.ZTS_AF_INET, type=libzt.ZTS_SOCK_STREAM, proto=0):
    if family == libzt.ZTS_AF_INET:
        host = '127.0.0.1'
    elif family == libzt.ZTS_AF_INET6:
        host = '::1'
    else:
        raise ValueError("Only AF_INET and AF_INET6 socket address families "
                         "are supported")
    if type != libzt.ZTS_SOCK_STREAM:
        raise ValueError("Only SOCK_STREAM socket type is supported")
    if proto != 0:
        raise ValueError("Only protocol zero is supported")

    # We create a connected TCP socket. Note the trick with
    # setblocking(False) that prevents us from having to create a thread.
    lsock = socket(family, type, proto)
    try:
        lsock.bind((host, 0))
        lsock.listen()
        # On IPv6, ignore flow_info and scope_id
        addr, port = lsock.getsockname()[:2]
        csock = socket(family, type, proto)
        try:
            csock.setblocking(False)
            try:
                csock.connect((addr, port))
            except (BlockingIOError, InterruptedError):
                pass
            csock.setblocking(True)
            ssock, _ = lsock.accept()
        except:
            csock.close()
            raise
    finally:
        lsock.close()

    # Authenticating avoids using a connection from something else
    # able to connect to {host}:{port} instead of us.
    # We expect only AF_INET and AF_INET6 families.
    try:
        if (
                ssock.getsockname() != csock.getpeername()
                or csock.getsockname() != ssock.getpeername()
        ):
            raise ConnectionError("Unexpected peer connection")
    except:
        # getsockname() and getpeername() can fail
        # if either socket isn't connected.
        ssock.close()
        csock.close()
        raise

    return (ssock, csock)

def create_connection(remote_address):
    """Convenience function to create a connection to a remote host"""
    # TODO: implement timeout
    conn = socket(libzt.ZTS_AF_INET, libzt.ZTS_SOCK_STREAM, 0)
    conn.connect(remote_address)
    return conn

def create_server(local_address, sock_family=libzt.ZTS_AF_INET, backlog=None):
    """Convenience function to create a listening socket"""
    # TODO: implement reuse_port
    conn = socket(sock_family, libzt.ZTS_SOCK_STREAM, 0)
    conn.bind(local_address)
    conn.listen(backlog)
    return conn

def fromfd(fd, sock_family, sock_type, sock_proto=0):
    """libzt does not support this (yet)"""
    raise NotImplementedError("ZeroTier does not expose OS-level sockets")

def fromshare(data):
    """ZeroTier sockets cannot be shared"""
    raise NotImplementedError("ZeroTier sockets cannot be shared")

def getaddrinfo(
        host, port, sock_family=0, sock_type=0, sock_proto=0, flags=0
):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def getfqdn(name):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def gethostbyname(hostname):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def gethostbyname_ex(hostname):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def gethostname():
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def gethostbyaddr(ip_address):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def getnameinfo(sockaddr, flags):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def getprotobyname(protocolname):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def getservbyname(servicename, protocolname):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def getservbyport(port, protocolname):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def ntohl(x):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def ntohs(x):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def htonl(x):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def htons(x):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def inet_aton(ip_string):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def inet_ntoa(packed_ip):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def inet_pton(address_family, ip_string):
    """Convert an IP address from its family-specific string format to a packed, binary format."""
    return libzt.zts_py_inet_pton(address_family, ip_string)

def inet_ntop(address_family, packed_ip):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def CMSG_LEN(length):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def CMSG_SPACE(length):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def getdefaulttimeout(self):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def setdefaulttimeout(timeout):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def sethostname(name):
    """libzt does not support this (yet)"""
    raise NotImplementedError("libzt does not support this (yet?)")

def if_nameindex():
    """libzt does not support this"""
    raise NotImplementedError("if_nameindex(): libzt does not name interfaces.")

def if_nametoindex(if_name):
    """libzt does not support this"""
    raise NotImplementedError("if_nametoindex(): libzt does not name interfaces.")

def if_indextoname(if_index):
    """libzt does not support this"""
    raise NotImplementedError("if_indextoname(): libzt does not name interfaces.")

class socket:
    """Pythonic class that wraps low-level sockets"""

    def __init__(self, sock_family=-1, sock_type=-1, sock_proto=-1, sock_fd=None):
        self._fd = sock_fd
        self._family = sock_family
        self._type = sock_type
        self._proto = sock_proto
        # Only create native socket if no fd was provided. We may have
        # accepted a connection
        if sock_fd is None:
            self._fd = libzt.zts_bsd_socket(sock_family, sock_type, sock_proto)
            if self._fd < 0:
                handle_error(self._fd)

        self._closed = False

    @property
    def family(self):
        """Return family of socket"""
        return self._family

    @property
    def type(self):
        """Return type of socket"""
        return self._type

    @property
    def proto(self):
        """Return protocol of socket"""
        return self._proto

    def accept(self):
        """accept() -> (socket, address_info)

        Wait for incoming connection and return a tuple of socket object and
        client address info.  This address info may be a tuple of host address
        and port."""
        new_conn_fd, addr = libzt.zts_py_accept(self._fd)
        return socket(self._family, self._type, self._proto, new_conn_fd), addr

    def bind(self, local_address):
        """bind(address)

        Bind the socket to a local interface address"""
        err = libzt.zts_py_bind(self._fd, self._family, self._type, local_address)
        if err < 0:
            handle_error(err)

    def close(self):
        """close()

        Close the socket"""
        if self._closed:
            return
        self._closed = True
        err = libzt.zts_py_close(self._fd)
        self._fd = -1
        if err < 0:
            handle_error(err)

    def connect(self, address):
        """connect(address)

        Connect the socket to a remote address"""
        err = libzt.zts_py_connect(self._fd, self._family, self._type, address)
        if err < 0:
            handle_error(err)

    def connect_ex(self, address):
        """connect_ex(address) -> errno

        Connect to remote host but return low-level result code, and errno on failure
        This uses a non-thread-safe implementation of errno
        """
        err = libzt.zts_py_connect(self._fd, self._family, self._type, address)
        if err < 0:
            return errno()
        return err

    def detach(self):
        """libzt does not support this"""
        raise NotImplementedError(
            "detach(): Not supported. ZeroTier sockets are not OS-level sockets"
        )

    def dup(self):
        """libzt does not support this"""
        raise NotImplementedError("libzt does not support this (yet?)")

    def fileno(self):
        """Return ZeroTier socket file descriptor. This is not OS-level. Can
        only be used with ZeroTier's version of select"""
        return self._fd

    def get_inheritable(self):
        """ZeroTier sockets cannot be shared. This always returns False"""
        return False

    def getblocking(self):
        """getblocking()

        Return True if the socket is in blocking mode, False if it is non-blocking"""
        return libzt.zts_get_blocking(self._fd)

    def getpeername(self):
        """Return the remote address to which the socket is connected."""
        err, peername = libzt.zts_py_getpeername(self._fd)
        if err != libzt.ZTS_ERR_OK:
            return handle_error(err)
        return peername

    def getsockname(self):
        """Return the socket’s own address."""
        err, sockname = libzt.zts_py_getsockname(self._fd)
        if err != libzt.ZTS_ERR_OK:
            return handle_error(err)
        return sockname

    def getsockopt(self, level, optname, buflen=None):
        """Get a socket option value"""
        return libzt.zts_py_getsockopt(self._fd, (level, optname))

    def ioctl(self, request, arg=0, mutate_flag=True):
        """Perform I/O control operations"""
        return libzt.zts_py_ioctl(self._fd, request, arg, mutate_flag)

    def listen(self, backlog=None):
        """listen([backlog])

        Put the socket in a listening state.  Backlog specifies the number of
        outstanding connections the OS will queue without being accepted.  If
        less than 0, it is set to 0.  If not specified, a reasonable default
        will be used."""

        if backlog is not None and backlog < 0:
            backlog = 0
        if backlog is None:
            backlog = -1  # Lower-level code picks default

        err = libzt.zts_bsd_listen(self._fd, backlog)
        if err < 0:
            handle_error(err)

    def makefile(self, mode="r", buffering=None, *, encoding=None, errors=None, newline=None):
        """libzt does not support this (yet)"""
        raise NotImplementedError("libzt does not support this (yet?)")

    def recv(self, n_bytes, flags=0):
        """recv(buffersize[, flags]) -> data

        Read up to buffersize bytes from remote.  Wait until at least one byte
        is read, or remote is closed.  If all data is read and remote is closed,
        returns empty string.  Flags may be:

          - ZTS_MSG_PEEK - Peeks at an incoming message.
          - ZTS_MSG_DONTWAIT - Nonblocking I/O for this operation only.
          - ZTS_MSG_MORE - Wait for more than one message.
        """
        if n_bytes < 0:
            raise ValueError('negative buffersize in recv')
        err, data = libzt.zts_py_recv(self._fd, n_bytes, flags)
        if err < 0:
            handle_error(err)
            return None
        return data

    def recvfrom(self, bufsize, flags=0):
        """recvfrom(buffersize[, flags]) -> data, address

        Receive data from the socket.
        The return value is a pair (bytes, address) where bytes is a bytes object
        representing the data received and address is the address of the socket
        sending the data."""
        if bufsize < 0:
            raise ValueError('negative buffersize in recvfrom')
        err, data, addr = libzt.zts_py_recvfrom(self._fd, bufsize, flags)
        if err < 0:
            return handle_error(err)
        return data, addr

    def recvmsg(self, bufsize, ancbufsize, flags):
        """libzt does not support this (yet)"""
        raise NotImplementedError("libzt does not support this (yet?)")

    def recvmsg_into(self, buffers, ancbufsize, flags):
        """libzt does not support this (yet)"""
        raise NotImplementedError("libzt does not support this (yet?)")

    def recvfrom_into(self, buffer, n_bytes=0, flags=0):
        """Receive data from the socket, writing it into buffer instead of creating a new bytestring."""
        if n_bytes < 0:
            raise ValueError('negative buffersize in recv_into')
        err, addr = libzt.zts_py_recvfrom_into(self._fd, buffer, n_bytes, flags)
        if err < 0:
            return handle_error(err)
        return err, addr

    def recv_into(self, buffer, n_bytes=0, flags=0):
        """Receive up to nbytes bytes from the socket, storing the data into a buffer rather than creating a new bytestring."""
        if n_bytes < 0:
            raise ValueError('negative buffersize in recv_into')
        err = libzt.zts_py_recv_into(self._fd, buffer, n_bytes, flags)
        if err < 0:
            return handle_error(err)
        return err

    def send(self, data, flags=0):
        """send(data[, flags]) -> count

        Write data to the socket.  Returns the number of bytes sent, which
        may be less than the full data size if the network is busy.
        Optional flags may be:

          - ZTS_MSG_PEEK - Peeks at an incoming message.
          - ZTS_MSG_DONTWAIT - Nonblocking I/O for this operation only.
          - ZTS_MSG_MORE - Sender will send more.
        """
        err = libzt.zts_py_send(self._fd, data, flags)
        if err < 0:
            handle_error(err)
        return err

    def sendall(self, bytes, flags=0):
        """sendall(data[, flags])

        | Write data to the socket. Sends data until all data is sent, then returns None. Optional flags may be:
        |  - ZTS_MSG_PEEK - Peeks at an incoming message.
        |  - ZTS_MSG_DONTWAIT - Nonblocking I/O for this operation only.
        |  - ZTS_MSG_MORE - Sender will send more.

        :param bytes: Data to send
        :type bytes: Union[bytes, bytearray]
        :param flags: Optional flags
        :type flags: int
        :return: None
        """
        err = libzt.zts_py_sendall(self._fd, bytes, flags)
        if err < 0:
            handle_error(err)

    def sendto(self, data, flags_or_address, address=None):
        """libzt does not support this (yet)"""
        if address is None:
            address = flags_or_address
            flags = 0
        else:
            flags = flags_or_address
        err = libzt.zts_py_sendto(self._fd, self._family, data, flags, address)
        if err < 0:
            handle_error(err)
        return err

    def sendmsg(self, buffers, ancdata, flags, address):
        """libzt does not support this (yet)"""
        raise NotImplementedError("libzt does not support this (yet?)")

    def sendmsg_afalg(self, msg, *, op, iv, assoclen, flags):
        """libzt does not support this (yet)"""
        raise NotImplementedError("sendmsg_afalg(): libzt does not support AF_ALG")

    def send_fds(self, sock, buffers, fds, flags, address):
        """libzt does not support this (yet)"""
        raise NotImplementedError("libzt does not support this (yet?)")

    def recv_fds(self, sock, bufsize, maxfds, flags):
        """libzt does not support this (yet)"""
        raise NotImplementedError("libzt does not support this (yet?)")

    def sendfile(self, file, offset=0, count=None):
        """libzt does not support this (yet)"""
        raise NotImplementedError("libzt does not support this (yet?)")

    def set_inheritable(self, inheritable):
        """ZeroTier sockets cannot be shared"""
        raise NotImplementedError("ZeroTier sockets cannot be shared")

    def setblocking(self, flag):
        """setblocking(flag)

        Sets the socket to blocking mode if flag=True, non-blocking if flag=False."""
        libzt.zts_set_blocking(self._fd, flag)

    def settimeout(self, value):
        """
        |Set timeout for socket send/recv operations. Socket can be in one of three modes:
        | - Blocking: Operations block until complete or the system returns an error (e.g. connection timed out)
        | - Non-blocking: Operations fail if they cannot be completed immediately
        | - Timeout: Operations fail if they cannot be completed within the specified timeout (and raise TimeoutError)

        :param value: Timeout in seconds (must be non-negative).
            If zero, socket is put in non-blocking mode.
            If None, socket is put in blocking mode.
        :type value: Float
        :return: None
        """
        err = libzt.zts_py_settimeout(self._fd, value)
        if err < 0:
            handle_error(err)

    def gettimeout(self):
        """
        |Get timeout for socket send/recv operations. Socket can be in one of three modes:
        | - Blocking: Operations block until complete or the system returns an error (e.g. connection timed out)
        | - Non-blocking: Operations fail if they cannot be completed immediately
        | - Timeout: Operations fail if they cannot be completed within the specified timeout (and raise TimeoutError)

        :return: Timeout in seconds.
            If zero, socket is in non-blocking mode.
            If None, socket is in blocking mode.
        :rtype: Optional[float]
        """
        err, res = libzt.zts_py_gettimeout(self._fd)
        if err < 0:
            handle_error(err)
        else:
            return res

    def setsockopt(self, level, optname, value):
        """Set a socket option value"""
        err = libzt.zts_py_setsockopt(self._fd, (level, optname, value))
        if err < 0:
            handle_error(err)
        return err

    def shutdown(self, how):
        """shutdown(how)

        Shut down one or more aspects (rx/tx) of the socket depending on how:

          - ZTS_SHUT_RD - Shut down reading side of socket.
          - ZTS_SHUT_WR - Shut down writing side of socket.
          - ZTS_SHUT_RDWR - Both ends of the socket.
        """
        libzt.zts_bsd_shutdown(self._fd, how)
