/* \file Config.h
 *
 * \brief Classe de configuração para controle do servidor
 *
 * "$Id: $"
*/
#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>

#define MAX_PORT_SIZE    6
#define DEFAULT_PORT     "2196"
#define BASE_PATH        "."
#define DEFAULT_SPEED    1024*1024*10
#define CONFIG_FILE_NAME "/home/abaiao/repo/treinamento/build_server/server.config"

#ifdef __cplusplus

#include <string>
class Config
{
public:
  Config(const std::string &basePath = BASE_PATH,
         const std::string &port     = DEFAULT_PORT,
         const uint64_t speed        = DEFAULT_SPEED);

  std::string getBasePath() const;
  std::string getPort()     const;
  uint64_t    getSpeed()    const;
  pid_t       getPid()      const;

  void setBasePath(const std::string& basePath);
  void setPort(const std::string port);
  void setSpeed(const uint64_t speed);
  void setConfigFileName(const std::string& configFileName);
  void setPid(const pid_t pid);

  void write();
  void read();

private:
  std::string   mBasePath;
  std::string   mPort;
  int32_t       mSpeed;
  std::string   mConfigFileName;
  pid_t         mPid;
};

#else
  typedef struct ConfigY Config;
#endif

#ifdef __cplusplus
extern "C" {
#endif

Config *create_config();
void release_config(Config **config);

void write_into_config_file(Config *config,
                                    const char * const base_path,
                                    const char * const port,
                                    const uint32_t     speed,
                                    const pid_t        pid);

void read_config_file(Config   *config,
                      char     *base_path,
                      char     *port,
                      int32_t *speed);
#ifdef __cplusplus
}
#endif

#endif // CONFIG_H
