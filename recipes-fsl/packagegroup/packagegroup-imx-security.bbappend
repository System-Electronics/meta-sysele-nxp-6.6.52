# the packages openssl-provider-se050 and plug-and-trust already provided by se05x recipe
RDEPENDS_EDGE_LOCK:remove = "${@bb.utils.contains('ENABLE_SE05', '1', ' openssl-provider-se050 plug-and-trust-ecc ', '', d)}"
