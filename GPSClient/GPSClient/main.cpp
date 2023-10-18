#include <QtCore/QCoreApplication>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QThread>
#include <QStringList>
#include <QtSql/QtSql>
#include <QObject>
#include <iostream>

using namespace std;

class SerialReader : public QObject
{
	Q_OBJECT

public slots:
	void onReadyRead()
	{
		QSqlQuery r;
		QByteArray newData = serialPort.readAll();
		nmeaDataBuffer += QString(newData);

		while (nmeaDataBuffer.contains("$GPGGA")) {
			int start = nmeaDataBuffer.indexOf("$GPGGA");
			nmeaDataBuffer = nmeaDataBuffer.mid(start); // Éliminer tout avant la trame
			int end = nmeaDataBuffer.indexOf("\r\n"); // Rechercher un séparateur de ligne
			if (end != -1) {
				QString nmeaString = nmeaDataBuffer.left(end);
				nmeaDataBuffer = nmeaDataBuffer.mid(end + 2); // Éliminer la trame traitée et le séparateur de ligne

				// Maintenant, nmeaString contient une trame GPS complète
				QStringList fields = nmeaString.split(',');
				if (fields.size() >= 15) {
					QString time = fields[1]; // Heure (HHMMSS)
					QString latitude = fields[2]; // Latitude (DDMM.MMMM)
					QString latitudeDirection = fields[3]; // Direction de la latitude (N/S)
					QString longitude = fields[4]; // Longitude (DDDMM.MMMM)
					QString longitudeDirection = fields[5]; // Direction de la longitude (E/W)

					// Obtenir la date du jour
					QDateTime currentDateTime = QDateTime::currentDateTime();
					QString formattedDate = currentDateTime.toString("yyyy-MM-dd");

					// Conversion de la latitude au format décimal
					QString latitudeDecimal = latitude.left(2) + "." + latitude.mid(2);
					if (latitudeDirection == "S") {
						latitudeDecimal.prepend("-");
					}

					// Conversion de la longitude au format décimal
					QString longitudeDecimal = longitude.left(3) + "." + longitude.mid(3);
					if (longitudeDirection == "W") {
						longitudeDecimal.prepend("-");
					}

					// Stockez les valeurs dans les variables de classe
					this->latitude = latitudeDecimal;
					this->longitude = longitudeDecimal;
					this->date = formattedDate;
					this->time = time;

					qDebug() << "longitude : " << this->longitude;
					qDebug() << "latitude : " << this->latitude;
					qDebug() << "date : " << this->date;
					qDebug() << "heure : " << time;

					r.prepare("INSERT INTO GPS (date, heure, latitude, longitude) VALUES (?, STR_TO_DATE(?, '%H%i%s'), ?, ?)");
					r.addBindValue(this->date);
					r.addBindValue(this->time);
					r.addBindValue(this->latitude);
					r.addBindValue(this->longitude);
					if (r.exec()) {
						std::cout << "Insertion réussie" << std::endl;
					}
					else {
						std::cout << "Echec insertion !" << std::endl;
						qDebug() << r.lastError().text();
					}
				}
			}
			else {
				// La trame n'est pas encore complète, attendez plus de données.
				break;
			}
		}
	}



private:
	QSerialPort serialPort;
	QString nmeaDataBuffer; // Tampon pour collecter les caractères entrants
	QString latitude;
	QString longitude;
	QString time;
	QString date;

public:
	SerialReader()
	{
		// Spécifiez le nom du port série (vérifiez le nom du port Arduino sur votre ordinateur)
		serialPort.setPortName("COM3"); // Remplacez "COM1" par le nom de votre port série

		// Configurez le débit en bauds, les bits de données, la parité, le nombre de bits d'arrêt, etc.
		serialPort.setBaudRate(QSerialPort::Baud9600);
		serialPort.setDataBits(QSerialPort::Data8);
		serialPort.setParity(QSerialPort::NoParity);
		serialPort.setStopBits(QSerialPort::OneStop);
		serialPort.setFlowControl(QSerialPort::NoFlowControl);

		connect(&serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

		// Ouvrez le port série
		if (serialPort.open(QIODevice::ReadOnly)) {
			qDebug() << "Port serie ouvert.";
		}
		else {
			qDebug() << "Impossible d ouvrir le port serie.";
		}


		QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL"); // ou mettre QSQLITE pour SQLite

		db.setHostName("192.168.65.252");
		db.setUserName("root");
		db.setPassword("root");
		db.setDatabaseName("Lawrence"); // ou mettre le nom du fichier sqlite
		if (db.open())
		{
			std::cout << "Connexion réussie à " << db.hostName().toStdString() << std::endl;
		}
		else
		{
			std::cout << "La connexion a échouée !" << std::endl;
			
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