/* \file Config.h
 *
 * \brief Classe de configuração para controle do servidor
 *
 * "$Id: $"
*/
#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include <string>

#include <QJsonObject>

#define DEFAULT_PORT  2196
#define BASE_PATH     "."
#define DEFAULT_SPEED 1024*1024*10

namespace server
{
  class Config
  {
  public:
    Config(const std::string &basePath = BASE_PATH,
            const uint16_t port        = DEFAULT_PORT,
            const uint64_t speed       = DEFAULT_SPEED);

    std::string GetBasePath() const;
    uint16_t    GetPort()     const;
    uint64_t    GetSpeed()    const;

    void SetBasePath(const std::string& basePath);
    void SetPort(const uint16_t port);
    void SetSpeed(const uint64_t speed);

    void Write(QJsonObject& jsonObject);
    void Read(const QJsonObject &jsonObject);

  private:
    std::string mBasePath;
    uint16_t    mPort;
    uint64_t    mSpeed;
  };
}

#endif // CONFIG_H
