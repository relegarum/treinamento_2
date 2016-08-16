#include "main_form.h"
#include "ui_main_form.h"

MainForm::MainForm(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::main_form)
{
  ui->setupUi(this);
}

MainForm::~MainForm()
{
  delete ui;
}

void MainForm::on_Signal_button_ui_accepted()
{
}

void MainForm::on_Signal_button_ui_rejected()
{
}
