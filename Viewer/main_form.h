#ifndef MAIN_FORM_H
#define MAIN_FORM_H

#include <QDialog>

namespace Ui {
  class main_form;
}

class main_form : public QDialog
{
  Q_OBJECT

public:
  explicit main_form(QWidget *parent = 0);
  ~main_form();

private:
  Ui::main_form *ui;
};

#endif // MAIN_FORM_H
