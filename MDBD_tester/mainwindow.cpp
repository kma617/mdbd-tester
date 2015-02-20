#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QList>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QList<QSerialPortInfo> serial_ports = QSerialPortInfo::availablePorts();

    foreach (QSerialPortInfo port_info, serial_ports)
    {
        ui->com_combobox->addItem(port_info.portName());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_start_test_button_clicked()
{
    const QString port_name = ui->com_combobox->currentText();

    QSerialPort serial_port(port_name);
    serial_port.setBaudRate(QSerialPort::Baud115200);

    serial_port.setDataBits(QSerialPort::Data8);
    serial_port.setParity(QSerialPort::NoParity);
    serial_port.setStopBits(QSerialPort::OneStop);

    serial_port.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial_port.open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(NULL, "Error opening port", port_name);
        return;
    }

    const QByteArray message("\x1d\x00\x05\x94\xf8\x01\x01\x46\x80\xee", 10);

    const int try_count_max = 10;
    for (int try_count=0; try_count <try_count_max; ++try_count)
    {
        ui->operation_log->appendPlainText(QString("Try to perform test %1").arg(try_count));

        serial_port.write(message);

        if (!serial_port.waitForReadyRead(500))
        {
            ui->operation_log->appendPlainText("No response from MK");
            return;
        }

        char ack_byte=0x00;
        qint64 read_count = serial_port.read(&ack_byte, 1);
        if (read_count != 1 || ack_byte != 0x06)
        {
            ui->operation_log->appendPlainText(QString("ACK symbol error %1").arg(ack_byte));
            return;
        }

        QByteArray response;
        do
        {
            response.append(serial_port.readAll());
        } while (serial_port.waitForReadyRead(100));

        if (response.size() != 13)
        {
            QString hex_response = QString(response.toHex());
            ui->operation_log->appendPlainText("Bad response from MK: " + hex_response);
            return;
        }

        serial_port.write(QByteArray(1, 0x06));
    }

}

void MainWindow::on_check_com_button_clicked()
{
    const QString port_name = ui->com_combobox->currentText();

    QSerialPort serial_port(port_name);
    serial_port.setBaudRate(QSerialPort::Baud115200);

    serial_port.setDataBits(QSerialPort::Data8);
    serial_port.setParity(QSerialPort::NoParity);
    serial_port.setStopBits(QSerialPort::OneStop);

    serial_port.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial_port.open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(NULL, "Error opening port", port_name);
    }
}
