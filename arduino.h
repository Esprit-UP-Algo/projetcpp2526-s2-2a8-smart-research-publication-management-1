#ifndef ARDUINO_H
#define ARDUINO_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QByteArray>

// ============================================================================
// Classe Arduino — v5
//
// CORRECTION v5 : suppression de flushReadBuffer()
//   flushReadBuffer() appelait serial->readAll() apres l'ouverture du port.
//   Sous certains drivers (CH340, FTDI), cela togglait les lignes DTR/RTS
//   et resetait l'Arduino une seconde fois pendant initRC522() → 0xFF.
//   Le vidage du buffer residuel est desormais gere entierement cote
//   Arduino via drainSerialBuffer() dans le .ino.
//
//   A la place : disableDTRReset() desactive le signal DTR apres ouverture
//   du port pour eviter tout reset intempestif ulterieur.
// ============================================================================
class Arduino
{
public:
    Arduino();

    int  connect_arduino();
    int  close_arduino();
    void write_to_arduino(QByteArray data);
    QByteArray read_from_arduino();
    QString    readLine();

    QSerialPort* getserial();
    QString      getarduino_port_name();

private:
    QSerialPort* serial;

    static const quint16 arduino_uno_vendor_id  = 9025;
    static const quint16 arduino_uno_producy_id =   67;

    QString    arduino_port_name;
    bool       arduino_is_available;
    QByteArray data;
    QByteArray buffer;
};

#endif // ARDUINO_H
