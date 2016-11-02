#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphics.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  scene = new graphics;
  ui->graphicsView->setScene(scene);
  ui->graphicsView->show();
  //TODO: set mouse tracking in statusbar

  // connect buttons to draw modes
  connect(ui->actionNode,SIGNAL(triggered()),scene,SLOT(setNodeDrawMode()));
  connect(ui->actionEdge,SIGNAL(triggered()),scene,SLOT(setEdgeDrawMode()));
}

MainWindow::~MainWindow()
{
  delete scene;
  delete ui;
}
