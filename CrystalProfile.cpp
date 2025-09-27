#include "CrystalProfile.h"
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

CrystalProfile::CrystalProfile() {
	QFile file(QCoreApplication::applicationDirPath() +  "/profile.cfg");
	file.open(QIODevice::ReadWrite);
	if (!file.isOpen()) {
		throw expection::fileOpenFailed;
	}
	QTextStream inStream(&file);
	while (!inStream.atEnd()) {
		QString line = inStream.readLine();
		qsizetype splitPos = line.indexOf(spliter);
		data.insert(line.mid(0, splitPos), line.mid(splitPos + 1, line.length() - splitPos - 1));
	}
	file.close();
}

CrystalProfile::~CrystalProfile() {
	QFile file(QCoreApplication::applicationDirPath() + "/profile.cfg");
	file.open(QIODevice::WriteOnly);
	if (file.isOpen()) {
		for (auto it = data.begin(); it != data.end(); ++it) {
			file.write((it.key() + spliter + it.value() + '\n').toUtf8());
		}
	}
	file.close();
}

QString& CrystalProfile::operator[](const QString& key) {
	if (key.contains(spliter)) {
		throw expection::illegalChar;
	}
	return data[key];
}

QString CrystalProfile::value(const QString& key) const {
	return data[key];
}

bool CrystalProfile::exists(const QString& key) const {
	return data.contains(key);
}

CrystalProfile::Iterator CrystalProfile::begin() {
	return data.begin();
}

CrystalProfile::Iterator CrystalProfile::end() {
	return data.end();
}

void CrystalProfile::set(const QString& key, const QString& value) {
	if (key.contains(spliter)) {
		throw expection::illegalChar;
	}
	data[key] = value;
}
