#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "graphics.h"
#include <QMainWindow>

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
public slots:
  // load/save
  bool save();
  void load();

  // misc
  void about();
  void help();

private:
  bool saveFile(const QString &fileName);
  void loadFile(const QString &fileName);
  Ui::MainWindow *ui;
  graphics *scene;
};

#endif // MAINWINDOW_H
