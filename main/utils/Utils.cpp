#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "Utils.h"

namespace iotTouch
{
  void Utils::toupper(std::string& data)
  {
    std::for_each(data.begin(), data.end(), [](char & c){
      c = ::toupper(c);
    });
  }

  void Utils::tolower(std::string& data)
  {
    std::for_each(data.begin(), data.end(), [](char & c){
      c = ::tolower(c);
    });
  }

  std::string Utils::convertHextoString(std::vector<uint8_t> data)
  {
    std::stringstream ss;

    ss << std::hex << std::setfill('0');

    for (int i = 0; i < data.size(); i++) {
      ss << std::hex << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
  }

  std::vector<uint8_t> Utils::convertHexStrToBytes(std::string hex)
  {
    std::vector<uint8_t> bytes;

    for (unsigned int i = 0; i < hex.length(); i += 2) {
      std::string byteString = hex.substr(i, 2);
      uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
      bytes.push_back(byte);
    }

    return bytes;
  }

  int Utils::getVersion(const std::string& data)
  {
    const char *p_version = data.c_str();
    int v_int = 0;

    while (*p_version != 0) {
        if (*p_version <= '9' && *p_version >= '0') {
            v_int = v_int * 10 + *p_version - '0';
        }
        p_version ++;
    }
    return v_int;
  }
}