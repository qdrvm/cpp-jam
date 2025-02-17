#
# Copyright Quadrivium LLC
# All Rights Reserved
# SPDX-License-Identifier: Apache-2.0
#

add_library(snp
  connections/alpn.cpp
  connections/connection.cpp
  connections/connections.cpp
  connections/dns_name.cpp
  connections/lsquic/engine.cpp
  connections/lsquic/init.cpp
  connections/prefer_key.cpp
  connections/protocol_id.cpp
  connections/stream.cpp
  connections/tls_certificate.cpp
)
target_link_libraries(snp
  fmt::fmt
  lsquic::lsquic
  OpenSSL::SSL
  schnorrkel_crust::schnorrkel_crust
  ZLIB::ZLIB
)
