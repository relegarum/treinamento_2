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

#include <QJsonDocument>

#define DEFAULT_PORT     2196
#define BASE_PATH        "."
#define DEFAULT_SPEED    1024*1024*10
#define CONFIG_FILE_NAME "../build_server/config.json"

namespace server
{
  class Config
  {
  public:
    Config(const std::string &basePath = BASE_PATH,
            const uint16_t port        = DEFAULT_PORT,
            const uint64_t speed       = DEFAULT_SPEED);

    std::string getBasePath() const;
    uint16_t    getPort()     const;
    uint64_t    getSpeed()    const;

    void setBasePath(const std::string& basePath);
    void setPort(const uint16_t port);
    void setSpeed(const uint64_t speed);
    void setConfigFileName(const std::string& configFileName);

    void write();
    void read();

  private:
    std::string   mBasePath;
    uint16_t      mPort;
    int32_t       mSpeed;
    std::string   mConfigFileName;

    static const QString LabelSpeed;
    static const QString LabelPort;
    static const QString LabelBasePath;
  };
}

#endif // CONFIG_H
