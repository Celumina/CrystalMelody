#include "CrystalStyleSheetEditor.h"

CrystalStyleSheetEditor::CrystalStyleSheetEditor() {
	data[""] = QMap<QString, QString>();
}

CrystalStyleSheetEditor::CrystalStyleSheetEditor(const QString& sheetString) {
	qsizetype cursorPos = 0;
	qsizetype elementLength = 0;
	while (true) {
		if ((elementLength = sheetString.indexOf('{', cursorPos) - cursorPos) < 0) {
			break;
		}
		QString domain = sheetString.mid(cursorPos, elementLength).trimmed();
		if (domain.indexOf('}') != -1) {
			cursorPos += domain.indexOf('}') + 1;
			continue;
		}
		cursorPos += elementLength + 1;
		// qDebug("{%s}\n", qUtf8Printable(domain));

		while (true) {
			if ((elementLength = sheetString.indexOf(':', cursorPos) - cursorPos) < 0) {
				break;
			}
			QString key = sheetString.mid(cursorPos, elementLength).trimmed();
			if (key.indexOf('}') != -1) {
				cursorPos += key.indexOf('}') + 1;
				break;
			}
			cursorPos += elementLength + 1;
			// qDebug("%s:", qUtf8Printable(key));
			if ((elementLength = sheetString.indexOf(';', cursorPos) - cursorPos) < 0) {
				break;
			}
			QString value = sheetString.mid(cursorPos, elementLength).trimmed();
			if (value.indexOf('}') != -1) {
				cursorPos += value.indexOf('}') + 1;
				break;
			}
			cursorPos += elementLength + 1;
			// qDebug("%s\n", qUtf8Printable(value));
			data[domain][key] = value;
		}
	}
}

void CrystalStyleSheetEditor::setColor(const QColor& color, const QString& domain) {
	_setColorImplement("color", color, domain);
}

void CrystalStyleSheetEditor::setBackground(const QColor& color, const QString& domain) {
	_setColorImplement("background", color, domain);
}

QString CrystalStyleSheetEditor::toString() {
	QString output;
	for (auto domainIter = data.keyValueBegin(); domainIter != data.keyValueEnd(); ++domainIter) {
		if (domainIter->first != "") {
			output += domainIter->first + '{';
		}
		for (auto attributeIter = domainIter->second.keyValueBegin(); attributeIter != domainIter->second.keyValueEnd(); ++attributeIter) {
			output += attributeIter->first + ':' + attributeIter->second + ';';
		}
		if (domainIter->first != "") {
			output += '}';
		}
	}
	// qDebug("%s", qUtf8Printable(output));
	return output;
}

void CrystalStyleSheetEditor::_setColorImplement(const QString& key, const QColor& color, const QString& domain) {
	if (domain != "") {
		data[domain][key] =
			QString("rgb(") +
			QString::number(color.red()) + ',' +
			QString::number(color.green()) + ',' +
			QString::number(color.blue()) + ')';
		return;
	}
	for (auto& attribute : data) {
		attribute[key] =
			QString("rgb(") +
			QString::number(color.red()) + ',' +
			QString::number(color.green()) + ',' +
			QString::number(color.blue()) + ')';
	}
}
