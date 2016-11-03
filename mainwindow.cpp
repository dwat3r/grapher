#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphics.h"
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
  connect(ui->actionNode,SIGNAL(triggered()),scene,SLOT(setNodeDrawMode()));
  connect(ui->actionEdge,SIGNAL(triggered()),scene,SLOT(setEdgeDrawMode()));
}

MainWindow::~MainWindow()
{
  delete scene;
  delete ui;
}
