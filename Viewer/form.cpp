#include "form.h"
#include "ui_form.h"
#include "signal.h"
#include <unistd.h>

#include <sstream>
#include <QMessageBox>

extern "C"
{
#include "../utils/handle_settings.h"
}

#define KB_MULTIPLIER 1024
#define MB_MULTIPLIER KB_MULTIPLIER*1024
#define GB_MULTIPLIER MB_MULTIPLIER*1024


Form::Form(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Form)
{
  ui->setupUi(this);

  char path_base[PATH_MAX];
  if (getcwd(path_base, PATH_MAX) != NULL)
  {
    ui->ui_path_text->setText(path_base);
  }
}

Form::~Form()
{
  delete ui;
}

void Form::on_ui_change_button_clicked()
{
  auto fileExist = mConfig.read();

  auto multiplierString = ui->ui_unit_box->currentText();
  auto multiplier = 1;
  if (multiplierString == "kb")
  {
    multiplier =  KB_MULTIPLIER;
  }
  else if (multiplierString == "Mb")
  {
    multiplier = MB_MULTIPLIER;
  }
  else if (multiplierString == "Gb")
  {
    multiplier = GB_MULTIPLIER;
  }
  auto speed      = multiplier * (ui->ui_speed_text->text().toInt());
  auto port       = ui->ui_port_text->text().toStdString();
  auto basePath   = ui->ui_path_text->text().toStdString();

  if (verify_if_valid_port(port.c_str()) == -1)
  {
    QMessageBox::warning(this,
                         "Porta inválida",
                         "Por favor insira uma porta entre 1024 e 65535");
    return;
  }

  if (treat_transmission_rate(std::to_string(speed).c_str(), &speed))
  {
    QMessageBox::warning(this,
                         "Taxa inválida",
                         "Por favor insira uma taxa de transmissão até"
                         "2147483647 bytes");
    return;
  }

  if (verify_if_valid_path(basePath.c_str()))
  {
    QMessageBox::warning(this,
                         "Path inválido",
                         "Por favor insira um path válido");
    return;
  }

  mConfig.setBasePath(basePath);
  mConfig.setPort(port);
  mConfig.setSpeed(speed);

  mConfig.write();

  if (fileExist &&
      mConfig.getPid() != DONT_SIGNAL)
  {
    kill(mConfig.getPid(), SIGUSR1);
  }
}
