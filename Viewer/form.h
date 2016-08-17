#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "../utils/Config.h"

namespace Ui {
  class Form;
}

class Form : public QWidget
{
  Q_OBJECT

public:
  explicit Form(QWidget *parent = 0);
  ~Form();

private slots:
  void on_ui_change_button_clicked();

private:
  Ui::Form *ui;
  Config mConfig;
};

#endif // FORM_H
