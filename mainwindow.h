#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "graphics.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  // statusbar
  bool eventFilter(QObject*,QEvent *event);
private:
  Ui::MainWindow *ui;
  graphics *scene;
};

#endif // MAINWINDOW_H
