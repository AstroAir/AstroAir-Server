#include "servergui.h"
#include "./ui_servergui.h"

#include <iostream>
#include <thread>

ServerGui::ServerGui(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerGui)
{
    ui->setupUi(this);
}

ServerGui::~ServerGui()
{
    delete ui;
}

void ServerGui::on_pushButton_clicked()
{
    int port = ui->lineEdit->text().toInt();
    std::cout << "This is a test" << port << std::endl;
}

