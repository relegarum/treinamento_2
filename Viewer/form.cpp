#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Form)
{
  ui->setupUi(this);
}

Form::~Form()
{
  delete ui;
}

void Form::on_ui_change_button_clicked()
{
  auto multiplierStr = ui->ui_unit_box->currentText();
  auto multiplier = 1;
  if (multiplierStr == "kb")
  {
    multiplier =  1024;
  }
  else if (multiplierStr == "Mb")
  {
    multiplier = 1024*1024;
  }
  else if (multiplierStr == "Gb")
  {
    multiplier = 1024*1024*1024;
  }
  auto speed      = multiplier * (ui->ui_speed_text->text().toInt());

  mConfig.setBasePath(ui->ui_path_text->text().toStdString());
  mConfig.setPort(ui->ui_port_text->text().toInt());
  mConfig.setSpeed(speed);
  mConfig.write();
}
