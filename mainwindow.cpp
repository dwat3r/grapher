#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphics.h"
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  scene = new graphics;
  ui->graphicsView->setScene(scene);
  ui->graphicsView->show();
  // set mouse tracking in statusbar
  ui->graphicsView->viewport()->installEventFilter(this);

  // connect buttons to draw modes
  connect(ui->actionNode,&QAction::triggered,scene,&graphics::setNodeDrawMode);
  connect(ui->actionEdge,&QAction::triggered,scene,&graphics::setEdgeDrawMode);
  // connect menu buttons
  connect(ui->actionLoad,&QAction::triggered,this,&MainWindow::load);
  connect(ui->actionSave,&QAction::triggered,this,&MainWindow::save);
  connect(ui->actionAbout,&QAction::triggered,this,&MainWindow::about);
  connect(ui->actionHelp,&QAction::triggered,this,&MainWindow::help);
}

MainWindow::~MainWindow()
{
  delete scene;
  delete ui;
}

bool MainWindow::eventFilter(QObject*, QEvent *event)
{
  if (event->type() == QEvent::MouseMove)
  {
      QMouseEvent *mouseCursor = static_cast<QMouseEvent*>(event);
      statusBar()->showMessage(QString("(x,y) coordinates: (%1,%2)")
                               .arg(mouseCursor->x())
                               .arg(mouseCursor->y()));
  }
  else if (event->type() == QEvent::Leave)
  {
      statusBar()->showMessage("");
  }
  return false;
}

bool MainWindow::save()
{
  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted)
    return false;
  return saveFile(dialog.selectedFiles().first());
}

void MainWindow::load()
{
  QString fileName = QFileDialog::getOpenFileName(this);
  if (!fileName.isEmpty())
      loadFile(fileName);
}

void MainWindow::about()
{
  QMessageBox::about(this, tr("About Grapher"),
                     tr("<b>Grapher</b> is awesome. "));
}

void MainWindow::help()
{
  QMessageBox::about(this, tr("Help"),
                     tr("Todo."));
}

void MainWindow::loadFile(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Error"),
                           tr("Cannot read file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString()));
      return;
  }
  QTextStream in(&file);
#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
  scene->cleanup();
  in >> *scene;
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif

  ui->statusBar->showMessage(tr("File loaded"),2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QFile::WriteOnly | QFile::Text))
    {
      QMessageBox::warning(this, tr("Error"), tr("Cannot write file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName),
                                file.errorString()));
      return false;
    }
  QTextStream out(&file);
#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
  out << *scene;
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
  ui->statusBar->showMessage(tr("File saved"), 2000);
  return true;
}
