/* \file Config.cpp
 *
 * \brief Classe de configuração para controle do servidor
 *
 * "$Id: $"
*/
#include "Config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits.h>
#include <unistd.h>
#include <string.h>

const std::string Config::ConfigFilePath = "/home/abaiao/repo/treinamento/build_server/server.config";

Config::Config(const std::string& basePath,
               const std::string& port,
               const uint64_t speed)
  : mBasePath(basePath),
    mPort(port),
    mSpeed(speed),
    mConfigFileName(ConfigFilePath)
{
}

std::string Config::getBasePath() const
{
  return mBasePath;
}

std::string Config::getPort() const
{
  return mPort;
}

pid_t Config::getPid() const
{
  return mPid;
}

uint64_t Config::getSpeed() const
{
  return mSpeed;
}

void Config::setBasePath(const std::string &basePath)
{
  mBasePath = basePath;
}

void Config::setPort(const std::__cxx11::string port)
{
  mPort = port;
}

void Config::setSpeed(const uint64_t speed)
{
  mSpeed = speed;
}

void Config::setConfigFileName(const std::string& configFileName)
{
  mConfigFileName = configFileName;
}

void Config::setPid(const pid_t pid)
{
  mPid = pid;
}

bool Config::write()
{
  std::fstream fileStream(mConfigFileName, std::ios::out);
  if(!fileStream.is_open())
  {
    std::cout << "Coudn't open file to write\n";
    return false;
  }

  fileStream << mBasePath << "\n";
  fileStream << mPort     << "\n";
  fileStream << mSpeed    << "\n";
  fileStream << mPid      << "\n";
  return true;
}

bool Config::read()
{
  std::fstream fileStream(mConfigFileName, std::ios::in);
  if (!fileStream.is_open())
  {
    std::cout << "Couldn't open file\n";
    return false;
  }

  std::getline(fileStream, mBasePath);
  std::getline(fileStream, mPort);

  std::string speedStr;
  std::getline(fileStream, speedStr);
  std::stringstream issSpeed(speedStr);
  issSpeed >> mSpeed;

  std::string pidStr;
  std::getline(fileStream, pidStr);
  std::stringstream issPid(pidStr);
  issPid >> mPid;

  return true;
}

Config *create_config()
{
  return new Config();
}

void release_config(Config **config)
{
  (*config)->setPid(DONT_SIGNAL);
  (*config)->write();
  delete *config;
}

void write_into_config_file(Config *config,
                            const char * const base_path,
                            const char * const port,
                            const uint32_t     speed,
                            const pid_t        pid)
{
  config->setBasePath(base_path);
  config->setPort(port);
  config->setSpeed(speed);
  config->setPid(pid);
  config->write();
}

void read_config_file(Config   *config,
                      char     *base_path,
                      char     *port,
                      int32_t  *speed)
{
  config->read();
  memset(base_path, '\0', PATH_MAX);
  memset(port, '\0', MAX_PORT_SIZE);
  realpath(config->getBasePath().c_str(), base_path);
  //strncpy(base_path, config->getBasePath().c_str(), config->getBasePath().size());
  strncpy(port, config->getPort().c_str(), config->getPort().size());
  *speed = config->getSpeed();
}
