#include "main_form.h"
#include "ui_main_form.h"

main_form::main_form(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::main_form)
{
  ui->setupUi(this);
}

main_form::~main_form()
{
  delete ui;
}
