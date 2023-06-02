#include <memory>
#include <vector>
#include <cstring>
#include "CaCertPem.h"

std::unique_ptr<std::vector<unsigned char>> get_certs()
{
  auto ca_cert = std::make_unique<std::vector<unsigned char>>();
  size_t buflen = ca_cert_pem_end - ca_cert_pem_start;
  for (size_t i = 0; i < buflen; ++i) {
    ca_cert->push_back(static_cast<unsigned char>(ca_cert_pem_start[i]));
  }

  // the mbedtls_x509_crt_parse function wants the size of the buffer, including the terminating 0 so we
  ca_cert->push_back('\0');

  return ca_cert;
}