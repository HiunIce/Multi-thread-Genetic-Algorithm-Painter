#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "GAPainter.h"
#include "time.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    srand((unsigned)time(NULL));
    // just for use the qthread
    GAPainterCollection* ga = new GAPainterCollection("C:\\Users\\Liang\\Desktop\\hana.png", 3);
    ga->run();

}

MainWindow::~MainWindow()
{
    delete ui;
}
