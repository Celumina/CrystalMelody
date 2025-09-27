#pragma once
#include <QString>
#include <QMap>
#include <QColor>

class CrystalStyleSheetEditor {
public:
	//														domain						 key			value
	using SheetType = QMap<QString, QMap<QString, QString>>;

	explicit CrystalStyleSheetEditor();
	explicit CrystalStyleSheetEditor(const QString& sheetString);

	void setColor(const QColor& color, const QString& domain = "");
	void setBackground(const QColor& color, const QString& domain = "");
	QString toString();
private:
	void _setColorImplement(const QString& key, const QColor& color, const QString& domain = "");
	SheetType data;
};

