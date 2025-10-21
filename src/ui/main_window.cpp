// SPDX-License-Identifier: MIT
// MainWindow - Implementation

#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QApplication>

namespace configgui {
namespace ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , file_menu_(nullptr)
    , help_menu_(nullptr)
    , toolbar_(nullptr)
    , central_widget_(nullptr)
{
    setWindowTitle("ConfigGUI - Schema-Driven Configuration Manager");
    setGeometry(100, 100, 800, 600);

    setupUI();
    createMenuBar();
    createToolBar();
    connectSignals();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    // Create central widget with label
    central_widget_ = new QWidget(this);
    auto* layout = new QVBoxLayout(central_widget_);

    auto* label = new QLabel("Welcome to ConfigGUI\n\nOpen a schema to begin", central_widget_);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    setCentralWidget(central_widget_);
}

void MainWindow::createMenuBar()
{
    // File Menu
    file_menu_ = menuBar()->addMenu(tr("&File"));

    auto* open_action = file_menu_->addAction(tr("&Open Configuration"));
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::onFileOpen);

    auto* save_action = file_menu_->addAction(tr("&Save Configuration"));
    save_action->setShortcut(QKeySequence::Save);
    connect(save_action, &QAction::triggered, this, &MainWindow::onFileSave);

    file_menu_->addSeparator();

    auto* exit_action = file_menu_->addAction(tr("E&xit"));
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &MainWindow::onFileExit);

    // Help Menu
    help_menu_ = menuBar()->addMenu(tr("&Help"));

    auto* about_action = help_menu_->addAction(tr("&About"));
    connect(about_action, &QAction::triggered, this, &MainWindow::onHelpAbout);
}

void MainWindow::createToolBar()
{
    toolbar_ = addToolBar(tr("Main Toolbar"));
    toolbar_->setObjectName("MainToolbar");

    auto* open_action = toolbar_->addAction(tr("Open"));
    connect(open_action, &QAction::triggered, this, &MainWindow::onFileOpen);

    auto* save_action = toolbar_->addAction(tr("Save"));
    connect(save_action, &QAction::triggered, this, &MainWindow::onFileSave);
}

void MainWindow::connectSignals()
{
    // Additional signal connections can be added here
}

void MainWindow::onFileOpen()
{
    QMessageBox::information(this, tr("Open Configuration"), tr("File open dialog would appear here"));
}

void MainWindow::onFileSave()
{
    QMessageBox::information(this, tr("Save Configuration"), tr("File save dialog would appear here"));
}

void MainWindow::onFileExit()
{
    QApplication::quit();
}

void MainWindow::onHelpAbout()
{
    QMessageBox::about(this, tr("About ConfigGUI"),
                       tr("ConfigGUI v1.0\n\n"
                          "Schema-Driven Configuration Manager\n"
                          "Built with Qt 6 and C++17\n"
                          "MISRA C++ Compliant"));
}

} // namespace ui
} // namespace configgui
