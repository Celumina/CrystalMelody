#pragma once
#include <QAbstractTableModel>

#include "CrystalFileSystem.h"

class CrystalFileSystemModel : public QAbstractTableModel {
public:
	// Note: order relative with headTags and header section.
	enum class Column {
		Name, 
		Type, 
		Size
	};

	CrystalFileSystemModel(QObject* parent = nullptr);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	// bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override; 

	bool open(const QString& fpath);
	bool setCurrent(const QModelIndex& index);
	bool setCurrentToParent();

	QString currentDirName() const;
	QString legibleSizeString(const QModelIndex& index) const;

private:

	CrystalFileSystem _fileSystem;

	// DEV: drop these variables to a impl struct
	int _columnCount = 3;

	int  _nameWidth = 200;
	// int _typeWIdth = 60;
	// int _sizeWidth = 100;
	int _rowHeight = 50;


	static const char* headerTags[3];
	static int sizeFloatPrecision;
};


