/* \file Config.cpp
 *
 * \brief Classe de configuração para controle do servidor
 *
 * "$Id: $"
*/
#include "Config.h"


namespace server
{
  Config::Config(const std::string& basePath,
                 const uint16_t port,
                 const uint64_t speed)
    : mBasePath(basePath),
      mPort(port),
      mSpeed(speed)
  {
  }

  std::string Config::GetBasePath() const
  {
    return mBasePath;
  }

  uint16_t Config::GetPort() const
  {
    return mPort;
  }

  uint64_t Config::GetSpeed() const
  {
    return mSpeed;
  }

  void Config::SetBasePath(const std::string &basePath)
  {
    mBasePath = basePath;
  }

  void Config::SetPort(const uint16_t port)
  {
    mPort = port;
  }

  void Config::SetSpeed(const uint64_t speed)
  {
    mSpeed = speed;
  }

  void Config::Write(QJsonObject &jsonObject)
  {

  }

  void Config::Read(const QJsonObject &jsonObject)
  {

  }
}
