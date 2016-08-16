#ifndef MAIN_FORM_H
#define MAIN_FORM_H

#include <QDialog>

namespace Ui {
  class main_form;
}

class MainForm : public QDialog
{
  Q_OBJECT

public:
  explicit MainForm(QWidget *parent = 0);
  ~MainForm();

private slots:
  void on_Signal_button_ui_accepted();

  void on_Signal_button_ui_rejected();

private:
  Ui::main_form *ui;
};

#endif // MAIN_FORM_H
