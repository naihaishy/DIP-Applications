#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "checker.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_Video_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load video"), tr(""), tr("Video (*.mp4 *.avi *.wav)") );

    if(!fileName.isEmpty())
    {
        m_SrcVideoPath = fileName;
        QMessageBox box(this);
        box.setWindowTitle(tr("Video Load Success"));
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("Click the menu to do next"));
    }
}

void MainWindow::on_actionStatus_Checker_triggered()
{
    Checker checker(m_SrcVideoPath.toStdString());
    checker.doing();
}
