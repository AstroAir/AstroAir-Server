#ifndef SERVERGUI_H
#define SERVERGUI_H

#include <QMainWindow>
#include <QApplication>

QT_BEGIN_NAMESPACE
namespace Ui { class ServerGui; }
QT_END_NAMESPACE

class ServerGui : public QMainWindow
{
    Q_OBJECT

public:
    ServerGui(QWidget *parent = nullptr);
    ~ServerGui();

private slots:
    void on_pushButton_clicked();

private:
    Ui::ServerGui *ui;
};
#endif // SERVERGUI_H
