#include "server_config_dialog.h"
#include <QCoreApplication>
#include <QGridLayout>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QHostAddress>
#include <QSettings>
#include <QPalette>

#include "startup.h"
#include "yacreader_global.h"

extern Startup * s;

ServerConfigDialog::ServerConfigDialog(QWidget * parent)
	:QDialog(parent)	
{
	accept = new QPushButton(tr("Update IPs"));
	connect(accept,SIGNAL(clicked()),this,SLOT(generateQR()));
	qrCodeImage = new QPixmap();
	qrCode = new QLabel(this);
	qrCode->move(196,73);
	qrCode->setFixedSize(200,200);

	QLabel * title1 = new QLabel(tr("EASY SERVER CONNECTION"),this);
	title1->move(37,30);
	title1->setStyleSheet("QLabel {color:#1F1F1F; font-size:18px; font-family: Arial; font-weight: bold;}");

	QLabel * title2 = new QLabel(tr("SERVER ADDRESS"),this);
	title2->move(451,30);
	title2->setStyleSheet("QLabel {color:#1F1F1F; font-size:18px; font-family: Arial; font-weight: bold;}");

	QLabel * qrMessage = new QLabel(tr("just scan the code with your device!!"),this);
	qrMessage->move(194,290);//373,627);
	qrMessage->setStyleSheet("QLabel {color:#1F1F1F; font-size:16px; font-family: Arial; font-style: italic;}");
	qrMessage->setWordWrap(true);
	qrMessage->setFixedWidth(200);

	QLabel * propaganda = new QLabel(tr("YACReader is now available for iOS devices, the best comic reading experience now in your iPad, iPhone or iPod touch. <a href='http://ios.yacreader.com'> Discover it! </a>"),this);
	propaganda->move(36,375);
	propaganda->setStyleSheet("QLabel {color:#1F1F1F; font-size:16px; font-family: Arial; font-style: italic;}"
		"QLabel::a {color:#1A1A1A}");
	propaganda->setWordWrap(true);
	propaganda->setFixedWidth(590);
	propaganda->setOpenExternalLinks(true);

	QLabel * ipLabel = new QLabel(tr("IP address"),this);
	ipLabel->move(455,78);
	ipLabel->setStyleSheet("QLabel {color:#1F1F1F; font-size:13px; font-family: Arial; font-weight: bold;}");

	QLabel * portLabel = new QLabel(tr("Port"),this);
	portLabel->move(455, 119);
	portLabel->setStyleSheet("QLabel {color:#1F1F1F; font-size:13px; font-family: Arial; font-weight: bold;}");

	ip = new QComboBox(this);
	connect(ip,SIGNAL(activated(const QString &)),this,SLOT(regenerateQR(const QString &)));
	ip->move(531,71);
	ip->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	port = new QLineEdit("8080",this);
	port->setReadOnly(true);
	port->setMaximumWidth(50);
	port->move(571,110);

	check = new QCheckBox(this);
	check->move(453,314);
	check->setText(tr("enable the server"));
	check->setStyleSheet("QCheckBox {color:#1F1F1F; font-size:13px; font-family: Arial; font-weight: bold;}");
	
	//check->setLayoutDirection(Qt::RightToLeft);
	
	//elementsLayout->setSpacing(40);
	//elementsLayout->addWidget(iphone);
	//elementsLayout->addStretch();
	//elementsLayout->addLayout(configLayout);

	QHBoxLayout * buttons = new QHBoxLayout;
	buttons->addStretch();
	buttons->addWidget(accept);

	//QVBoxLayout *	mainLayout = new QVBoxLayout;
	//mainLayout->addLayout(elementsLayout);
	//mainLayout->addLayout(buttons);
	//mainLayout->addWidget(qrCode,0,1);

	//this->setLayout(mainLayout);

	QPalette Pal(palette()); 
	// set black background
	QPalette palette;
	QImage image(":/images/serverConfigBackground.png");
	palette.setBrush(this->backgroundRole(), QBrush(image));

	setPalette(palette);

	this->setFixedSize(image.size());
	
	QSettings * settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creaci�n del fichero de config con el servidor
	settings->beginGroup("libraryConfig");

	if(settings->value(SERVER_ON,true).toBool())
	{
		check->setChecked(true);
		generateQR();
	}
	else
		check->setChecked(false);

	settings->endGroup();

	connect(check,SIGNAL(stateChanged(int)),this,SLOT(enableServer(int)));
}

void ServerConfigDialog::enableServer(int status)
{
	QSettings * settings = new QSettings(QCoreApplication::applicationDirPath()+"/YACReaderLibrary.ini",QSettings::IniFormat); //TODO unificar la creaci�n del fichero de config con el servidor
	settings->beginGroup("libraryConfig");

	if(status == Qt::Checked)
	{
		s->start();
		this->generateQR();
		settings->setValue(SERVER_ON,true);
	}
	else
	{
		s->stop();
		qrCode->setPixmap(QPixmap());
		ip->clear();
		port->setText("");
		settings->setValue(SERVER_ON,false);
	}
	settings->endGroup();
}

void ServerConfigDialog::generateQR()
{
	//QString items;
	//foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
	//{
	//	if (~interface.flags() & QNetworkInterface::IsLoopBack)//interface.flags().testFlag(QNetworkInterface::IsRunning))
	//		foreach (QNetworkAddressEntry entry, interface.addressEntries())
	//		{
	//			if ( interface.hardwareAddress() != "00:00:00:00:00:00" &&     entry.ip().toString().contains("."))
	//				items.append(interface.name() + entry.ip().toString());
	//		}
	//}
	ip->clear();
	QString dir;
	QList<QHostAddress> list = QHostInfo::fromName( QHostInfo::localHostName()  ).addresses();
	QList<QString> otherAddresses;
	foreach(QHostAddress add, list)
	{
		QString tmp = add.toString();
		if(tmp.contains(".") && tmp != "127.0.0.1")
		{
			if(dir.isEmpty())
				dir = tmp;
			else
				otherAddresses.push_back(tmp);
			
		}	
	}
	if(!dir.isEmpty())
	{
		generateQR(dir+":"+s->getPort());
		ip->addItem(dir);
		ip->addItems(otherAddresses);
		port->setText(s->getPort());
	}
	else
	{

	}
	//qrCode->setText(dir+":8080");

	
}

void ServerConfigDialog::generateQR(const QString & serverAddress)
{
	qrGenerator = new QProcess();
	QStringList attributes;
	attributes << "-o" << "-" /*QCoreApplication::applicationDirPath()+"/utils/tmp.png"*/ << "-s" << "8" << "-l" << "H" << "-m" << "0" << serverAddress;
	connect(qrGenerator,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(updateImage(void)));
	connect(qrGenerator,SIGNAL(error(QProcess::ProcessError)),this,SLOT(openingError(QProcess::ProcessError)));
	qrGenerator->start(QCoreApplication::applicationDirPath()+"/utils/qrencode",attributes);
}

void ServerConfigDialog::updateImage()
{
	QByteArray imgBinary = qrGenerator->readAllStandardOutput();
	//imgBinary = imgBinary.replace(0x0D0A,0x0A);

	if(!qrCodeImage->loadFromData(imgBinary))
		qrCode->setText(tr("QR generator error!"));
	else
		qrCode->setPixmap(*qrCodeImage);
	
	delete qrGenerator;

	

/*	qrCodeImage->load(QCoreApplication::applicationDirPath()+"/utils/tmp.png");
	qrCode->setPixmap(*qrCodeImage);

	delete qrGenerator;*/
}

void ServerConfigDialog::regenerateQR(const QString & ip)
{
	generateQR(ip+":"+s->getPort());
}
