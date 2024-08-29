from enum import IntEnum

import libzt

_user_specified_event_handler = None


class ZtsEvent(IntEnum):
    NODE_UP = libzt.ZTS_EVENT_NODE_UP
    NODE_ONLINE = libzt.ZTS_EVENT_NODE_ONLINE
    NODE_OFFLINE = libzt.ZTS_EVENT_NODE_OFFLINE
    NODE_DOWN = libzt.ZTS_EVENT_NODE_DOWN
    NODE_FATAL_ERROR = libzt.ZTS_EVENT_NODE_FATAL_ERROR
    NETWORK_NOT_FOUND = libzt.ZTS_EVENT_NETWORK_NOT_FOUND
    NETWORK_CLIENT_TOO_OLD = libzt.ZTS_EVENT_NETWORK_CLIENT_TOO_OLD
    NETWORK_REQ_CONFIG = libzt.ZTS_EVENT_NETWORK_REQ_CONFIG
    NETWORK_OK = libzt.ZTS_EVENT_NETWORK_OK
    NETWORK_ACCESS_DENIED = libzt.ZTS_EVENT_NETWORK_ACCESS_DENIED
    NETWORK_READY_IP4 = libzt.ZTS_EVENT_NETWORK_READY_IP4
    NETWORK_READY_IP6 = libzt.ZTS_EVENT_NETWORK_READY_IP6
    NETWORK_READY_IP4_IP6 = libzt.ZTS_EVENT_NETWORK_READY_IP4_IP6
    NETWORK_DOWN = libzt.ZTS_EVENT_NETWORK_DOWN
    NETWORK_UPDATE = libzt.ZTS_EVENT_NETWORK_UPDATE
    STACK_UP = libzt.ZTS_EVENT_STACK_UP
    STACK_DOWN = libzt.ZTS_EVENT_STACK_DOWN
    NETIF_UP = libzt.ZTS_EVENT_NETIF_UP
    NETIF_DOWN = libzt.ZTS_EVENT_NETIF_DOWN
    NETIF_REMOVED = libzt.ZTS_EVENT_NETIF_REMOVED
    NETIF_LINK_UP = libzt.ZTS_EVENT_NETIF_LINK_UP
    NETIF_LINK_DOWN = libzt.ZTS_EVENT_NETIF_LINK_DOWN
    PEER_DIRECT = libzt.ZTS_EVENT_PEER_DIRECT
    PEER_RELAY = libzt.ZTS_EVENT_PEER_RELAY
    PEER_UNREACHABLE = libzt.ZTS_EVENT_PEER_UNREACHABLE
    PEER_PATH_DISCOVERED = libzt.ZTS_EVENT_PEER_PATH_DISCOVERED
    PEER_PATH_DEAD = libzt.ZTS_EVENT_PEER_PATH_DEAD
    ROUTE_ADDED = libzt.ZTS_EVENT_ROUTE_ADDED
    ROUTE_REMOVED = libzt.ZTS_EVENT_ROUTE_REMOVED
    ADDR_ADDED_IP4 = libzt.ZTS_EVENT_ADDR_ADDED_IP4
    ADDR_REMOVED_IP4 = libzt.ZTS_EVENT_ADDR_REMOVED_IP4
    ADDR_ADDED_IP6 = libzt.ZTS_EVENT_ADDR_ADDED_IP6
    ADDR_REMOVED_IP6 = libzt.ZTS_EVENT_ADDR_REMOVED_IP6
    STORE_IDENTITY_SECRET = libzt.ZTS_EVENT_STORE_IDENTITY_SECRET
    STORE_IDENTITY_PUBLIC = libzt.ZTS_EVENT_STORE_IDENTITY_PUBLIC
    STORE_PLANET = libzt.ZTS_EVENT_STORE_PLANET
    STORE_PEER = libzt.ZTS_EVENT_STORE_PEER
    STORE_NETWORK = libzt.ZTS_EVENT_STORE_NETWORK


class ZtsNetworkStatus(IntEnum):
    REQUESTING_CONFIGURATION = libzt.ZTS_NETWORK_STATUS_REQUESTING_CONFIGURATION
    OK = libzt.ZTS_NETWORK_STATUS_OK
    ACCESS_DENIED = libzt.ZTS_NETWORK_STATUS_ACCESS_DENIED
    NOT_FOUND = libzt.ZTS_NETWORK_STATUS_NOT_FOUND
    PORT_ERROR = libzt.ZTS_NETWORK_STATUS_PORT_ERROR
    CLIENT_TOO_OLD = libzt.ZTS_NETWORK_STATUS_CLIENT_TOO_OLD


class ZtsNetworkType(IntEnum):
    PRIVATE = libzt.ZTS_NETWORK_TYPE_PRIVATE
    PUBLIC = libzt.ZTS_NETWORK_TYPE_PUBLIC


class _EventCallbackClass(libzt.PythonDirectorCallbackClass):
    """ZeroTier event callback class"""

    pass


class MyEventCallbackClass(_EventCallbackClass):
    def on_zerotier_event(self, msg):
        global _user_specified_event_handler
        if _user_specified_event_handler is not None:
            _user_specified_event_handler(msg)


class ZeroTierNode:
    native_event_handler = None

    def __init__(self):
        self.native_event_handler = MyEventCallbackClass()
        libzt.zts_init_set_event_handler(self.native_event_handler)

    """ZeroTier Node"""

    def init_from_storage(self, storage_path):
        """Initialize the node from storage (or tell it to write to that location)"""
        return libzt.zts_init_from_storage(storage_path)

    """Set the node's event handler"""

    def init_set_event_handler(self, event_handler):
        global _user_specified_event_handler
        _user_specified_event_handler = event_handler

    def init_set_port(self, port):
        """Set the node's primary port"""
        return libzt.zts_init_set_port(port)

    def node_start(self):
        """Start the ZeroTier service"""
        return libzt.zts_node_start()

    def node_stop(self):
        """Stop the ZeroTier service"""
        return libzt.zts_node_stop()

    def node_free(self):
        """Permanently shut down the network stack"""
        return libzt.zts_node_free()

    def net_join(self, net_id):
        """Join a ZeroTier network"""
        return libzt.zts_net_join(net_id)

    def net_leave(self, net_id):
        """Leave a ZeroTier network"""
        return libzt.zts_net_leave(net_id)

    def node_is_online(self):
        return libzt.zts_node_is_online()

    def node_id(self):
        return libzt.zts_node_get_id()

    def net_transport_is_ready(self, net_id):
        return libzt.zts_net_transport_is_ready(net_id)

    def addr_get_ipv4(self, net_id):
        return libzt.zts_py_addr_get_str(net_id, libzt.ZTS_AF_INET)

    def addr_get_ipv6(self, net_id):
        return libzt.zts_py_addr_get_str(net_id, libzt.ZTS_AF_INET6)
