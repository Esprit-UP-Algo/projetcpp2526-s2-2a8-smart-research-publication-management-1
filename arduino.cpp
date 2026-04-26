#include "arduino.h"

Arduino::Arduino()
{
    data = "";
    arduino_port_name = "";
    arduino_is_available = false;
    serial = new QSerialPort;
    buffer.clear();
}

QString Arduino::getarduino_port_name()
{
    return arduino_port_name;
}

QSerialPort* Arduino::getserial()
{
    return serial;
}

// ============================================================================
// connect_arduino() — v5
//
// CORRECTION PRINCIPALE v5 :
//   Apres serial->open(), on appelle immediatement :
//     serial->setDataTerminalReady(false);
//
//   Explication :
//   L'Arduino Uno possede un circuit auto-reset : quand le signal DTR
//   du port serie passe de HIGH a LOW (ou inverse selon le driver), il
//   reset le microcontroleur via le condensateur de 100nF sur la broche
//   RESET. Ce comportement est intentionnel pour permettre le
//   telechargement du sketch depuis l'IDE Arduino.
//
//   Probleme : Qt active DTR=HIGH a l'ouverture du port (comportement
//   par defaut de QSerialPort). Ensuite, flushReadBuffer() (v4)
//   provoquait un second toggle. En mettant DTR=LOW juste apres
//   l'ouverture du port, on empeche tout reset ulterieur du a des
//   operations sur le port Qt (readAll, flush, etc.).
//
//   Le premier reset (ouverture du port) est inevitable et est gere
//   par le delay(3000) au debut du setup() Arduino.
//   Tous les resets suivants sont bloques par setDataTerminalReady(false).
//
//   Note : setRequestToSend(false) est aussi appele pour desactiver RTS,
//   un second signal qui peut causer le meme probleme sur certains clones
//   Arduino avec des chips CH340 ou CP2102.
// ============================================================================
int Arduino::connect_arduino()
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (info.hasVendorIdentifier() && info.hasProductIdentifier()) {
            if (info.vendorIdentifier()  == arduino_uno_vendor_id &&
                info.productIdentifier() == arduino_uno_producy_id)
            {
                arduino_is_available = true;
                arduino_port_name    = info.portName();
            }
        }
    }

    qDebug() << "[Arduino] Port detecte :" << arduino_port_name;

    if (arduino_is_available) {
        serial->setPortName(arduino_port_name);

        if (serial->open(QSerialPort::ReadWrite)) {
            // ── Configuration du port ─────────────────────────────────
            serial->setBaudRate(QSerialPort::Baud9600);
            serial->setDataBits(QSerialPort::Data8);
            serial->setParity(QSerialPort::NoParity);
            serial->setStopBits(QSerialPort::OneStop);
            serial->setFlowControl(QSerialPort::NoFlowControl);

            // ── CORRECTION v5 : desactiver DTR et RTS ─────────────────
            // Empeche tout reset Arduino intempestif apres l'ouverture.
            // Le premier reset (DTR a l'ouverture) est inevitable et
            // est absorbe par le delay(3000) dans setup() Arduino.
            // Les resets suivants (causes par des operations Qt sur le
            // port : readAll, flush, write rapide) sont bloques ici.
            serial->setDataTerminalReady(false);
            serial->setRequestToSend(false);

            qDebug() << "[Arduino] Port ouvert, DTR/RTS desactives :" << arduino_port_name;
            return 0;
        }

        qDebug() << "[Arduino] Ouverture echouee :" << serial->errorString();
        return 1;
    }

    qDebug() << "[Arduino] Aucun Arduino Uno detecte.";
    return -1;
}

// ============================================================================
// close_arduino()
// ============================================================================
int Arduino::close_arduino()
{
    if (serial->isOpen()) {
        serial->close();
        return 0;
    }
    return 1;
}

// ============================================================================
// read_from_arduino() — conserve pour compatibilite
// ============================================================================
QByteArray Arduino::read_from_arduino()
{
    if (serial->isReadable()) {
        data = serial->readAll();
        return data;
    }
    return QByteArray();
}

// ============================================================================
// write_to_arduino()
// ============================================================================
void Arduino::write_to_arduino(QByteArray d)
{
    if (serial->isWritable()) {
        serial->write(d);
    } else {
        qDebug() << "[Arduino] Impossible d'ecrire sur le port serie !";
    }
}

// ============================================================================
// readLine()
// Lecture ligne par ligne avec buffer d'accumulation.
// ============================================================================
QString Arduino::readLine()
{
    if (serial->bytesAvailable() > 0) {
        buffer.append(serial->readAll());
    }

    int pos = buffer.indexOf('\n');
    if (pos != -1) {
        QByteArray line = buffer.left(pos);
        buffer.remove(0, pos + 1);
        return QString::fromUtf8(line).trimmed();
    }

    return QString();
}
