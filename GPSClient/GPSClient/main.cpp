#include <QtCore/QCoreApplication>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QThread>
#include <QStringList>
#include <QtSql/QtSql>
#include <QObject>
#include <iostream>

using namespace std;

// On cr�er une classe SerialReader pour lire en s�rie les donn�es du GPS
class SerialReader : public QObject
{
	Q_OBJECT

public slots:
	// M�thode onReadyRead quiq sert de m�thode principale qui appelle les autre m�thodes.
	void onReadyRead()
	{
		QSqlQuery r;
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

					// Obtenir la date du jour
					// On appelle la m�thode currentDateTime()
					QDateTime currentDateTime = QDateTime::currentDateTime();
					// Puis on la formate au format voulu.
					QString formattedDate = currentDateTime.toString("yyyy-MM-dd");

					// Conversion de la latitude au format d�cimal
					// On appelle la fonction convertToDecimal() avec comme param�tre la latitude et la latitudeDirection
					QString latitudeDecimal = convertToDecimal(latitude, latitudeDirection);

					// Conversion de la longitude au format d�cimal
					// On appelle la fonction convertToDecimal() avec comme param�tre la longitude et la longitudeDirection
					QString longitudeDecimal = convertToDecimal(longitude, longitudeDirection);

					// Stockez les valeurs dans les variables de classe
					this->latitude = latitudeDecimal;
					this->longitude = longitudeDecimal;
					this->date = formattedDate;
					this->time = time;

					// On envoie les valeurs dans la console pour comprendre ce que fait le code
					qDebug() << "longitude : " << this->longitude;
					qDebug() << "latitude : " << this->latitude;
					qDebug() << "date : " << this->date;
					qDebug() << "heure : " << time;

					// On pr�pare l'insertion en BDD des informations converties du GPS
					r.prepare("INSERT INTO GPS (date, heure, latitude, longitude) VALUES (?, STR_TO_DATE(?, '%H%i%s'), ?, ?)");
					// On ajoute les bon param�tres aux bons endroits
					r.addBindValue(this->date);
					r.addBindValue(this->time);
					r.addBindValue(this->latitude);
					r.addBindValue(this->longitude);
					//On ex�cute l'insertion en BDD et on envoie un message de confirmation
					if (r.exec()) {
						std::cout << "Insertion r�ussie" << std::endl;
					}
					else {
						std::cout << "Echec insertion !" << std::endl;
						qDebug() << r.lastError().text();
					}
				}
			}
			else {
				// La trame n'est pas encore compl�te, attendez plus de donn�es.
				break;
			}
		}
	}



private:
	// Tout les param�tre important pour notre code
	QSerialPort serialPort; // Numero de port (COM1)
	QString nmeaDataBuffer; // Tampon pour collecter les caract�res entrants
	QString latitude;
	QString longitude;
	QString time;
	QString date;

public:
	// M�thode qui convertie les donn�es envoy�es du GPS en nombre d�cimaux
	QString convertToDecimal(const QString& coordinate, const QString& direction)
	{
		// Convertit la latitude ou la longitude au format d�cimal
		double decimalCoordinate = coordinate.toDouble();

		// Condition qui v�rifie la direction
		if (direction.toUpper() == "S" || direction.toUpper() == "W")
		{
			// Supprime les trois premiers chiffres de la longitude
			QString result = QString::number(decimalCoordinate, 'f', 7).mid(3);

			// Supprime les z�ros inutiles
			result = result.trimmed().remove(QRegExp("0+$")).remove(QRegExp("\\.$"));

			return result.prepend('-');
		}
		// Condition qui v�rifie la direction
		else if (direction.toUpper() == "N" || direction.toUpper() == "E")
		{
			// Supprime le premier chiffre de la latitude
			QString result = QString::number(decimalCoordinate, 'f', 7).mid(1);

			// Supprime les z�ros inutiles
			result = result.trimmed().remove(QRegExp("^0+")).remove(QRegExp("\\.$"));

			return result;
		}

		// Si la direction n'est ni "S" ni "W" ni "N" ni "E", retourne une cha�ne vide ou un message d'erreur selon vos besoins.
		return QString("Erreur, la convertion n'a pas march�e");
	}

	SerialReader()
	{
		// Sp�cifiez le nom du port s�rie (v�rifiez le nom du port Arduino sur votre ordinateur)
		serialPort.setPortName("COM1"); // Remplacez "COM1" par le nom de votre port s�rie

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


		QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL"); // ou mettre QSQLITE pour SQLite

		db.setHostName("192.168.65.252");
		db.setUserName("root");
		db.setPassword("root");
		db.setDatabaseName("Lawrence"); // ou mettre le nom du fichier sqlite
		if (db.open())
		{
			std::cout << "Connexion r�ussie � " << db.hostName().toStdString() << std::endl;
		}
		else
		{
			std::cout << "La connexion a �chou�e !" << std::endl;
			
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