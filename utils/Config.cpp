/* \file Config.cpp
 *
 * \brief Classe de configuração para controle do servidor
 *
 * "$Id: $"
*/
#include "Config.h"
#include <fstream>
#include <iostream>

#include <QJsonObject>
#include <QFile>

namespace server
{

  const QString Config::LabelSpeed    = "speed";
  const QString Config::LabelPort     = "port";
  const QString Config::LabelBasePath = "base_path";

  Config::Config(const std::string& basePath,
                 const uint16_t port,
                 const uint64_t speed)
    : mBasePath(basePath),
      mPort(port),
      mSpeed(speed),
      mConfigFileName(CONFIG_FILE_NAME)
  {
  }

  std::string Config::getBasePath() const
  {
    return mBasePath;
  }

  uint16_t Config::getPort() const
  {
    return mPort;
  }

  uint64_t Config::getSpeed() const
  {
    return mSpeed;
  }

  void Config::setBasePath(const std::string &basePath)
  {
    mBasePath = basePath;
  }

  void Config::setPort(const uint16_t port)
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

  void Config::write()
  {
    QJsonObject jsonObject;
    jsonObject[LabelBasePath] = QString(mBasePath.c_str());
    jsonObject[LabelPort]     = mPort;
    jsonObject[LabelSpeed]    = mSpeed;

    QFile file(mConfigFileName.c_str());
    if (!file.open(QIODevice::WriteOnly))
    {
      qWarning("Coudn't open file to write\n");
      return;
    }

    QJsonDocument jsonDocument(jsonObject);
    file.write(jsonDocument.toJson());
    return;
  }

  void Config::read()
  {
    QFile file(mConfigFileName.c_str());
    if (!file.open(QIODevice::ReadOnly))
    {
      std::cout << "Couldn't open file\n";
      return;
    }
    QByteArray data = file.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    QJsonObject jsonObj = loadDoc.object();

    mBasePath = jsonObj[LabelBasePath].toString().toStdString();
    mPort     = jsonObj[LabelPort    ].toInt();
    mSpeed    = jsonObj[LabelSpeed   ].toInt();

    return;
  }
}
