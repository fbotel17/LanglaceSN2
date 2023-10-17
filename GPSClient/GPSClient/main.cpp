#include <QtCore/QCoreApplication>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QThread>
#include <QStringList>

#include <QObject>

class SerialReader : public QObject
{
	Q_OBJECT

public slots:
	void onReadyRead()
	{
		QByteArray newData = serialPort.readAll();
		nmeaDataBuffer += QString(newData);

		while (nmeaDataBuffer.contains("$GPGGA")) {
			int start = nmeaDataBuffer.indexOf("$GPGGA");
			nmeaDataBuffer = nmeaDataBuffer.mid(start); // �liminer tout avant la trame
			int end = nmeaDataBuffer.indexOf("\r\n"); // Rechercher un s�parateur de ligne
			if (end != -1) {
				QString nmeaString = nmeaDataBuffer.left(end);
				nmeaDataBuffer = nmeaDataBuffer.mid(end + 2); // �liminer la trame trait�e et le s�parateur de ligne

				// Maintenant, nmeaString contient une trame GPS compl�te
				QStringList fields = nmeaString.split(',');
				if (fields.size() >= 15) {
					QString time = fields[1]; // Heure (HHMMSS)
					QString latitude = fields[2]; // Latitude (DDMM.MMMM)
					QString latitudeDirection = fields[3]; // Direction de la latitude (N/S)
					QString longitude = fields[4]; // Longitude (DDDMM.MMMM)
					QString longitudeDirection = fields[5]; // Direction de la longitude (E/W)

					// Stockez les valeurs dans les variables de classe
					this->latitude = latitude;
					this->longitude = longitude;
					this->time = time;

					qDebug() << "longitude : "<< longitude;
					qDebug() << "latitude : " << latitude;
					qDebug() << "heure : " << time << endl;


				}
			}
			else {
				// La trame n'est pas encore compl�te, attendez plus de donn�es.
				break;
			}
		}
	}

private:
	QSerialPort serialPort;
	QString nmeaDataBuffer; // Tampon pour collecter les caract�res entrants
	QString latitude;
	QString longitude;
	QString time;

public:
	SerialReader()
	{
		// Sp�cifiez le nom du port s�rie (v�rifiez le nom du port Arduino sur votre ordinateur)
		serialPort.setPortName("COM3"); // Remplacez "COM1" par le nom de votre port s�rie

		// Configurez le d�bit en bauds, les bits de donn�es, la parit�, le nombre de bits d'arr�t, etc.
		serialPort.setBaudRate(QSerialPort::Baud9600);
		serialPort.setDataBits(QSerialPort::Data8);
		serialPort.setParity(QSerialPort::NoParity);
		serialPort.setStopBits(QSerialPort::OneStop);
		serialPort.setFlowControl(QSerialPort::NoFlowControl);

		connect(&serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

		// Ouvrez le port s�rie
		if (serialPort.open(QIODevice::ReadOnly)) {
			qDebug() << "Port serie ouvert.";
		}
		else {
			qDebug() << "Impossible d ouvrir le port serie.";
		}
	}
};

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	SerialReader serialReader;

	return a.exec();
}

#include "main.moc"