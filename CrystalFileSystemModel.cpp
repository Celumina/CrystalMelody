#include "CrystalFileSystemModel.h"
#include <QApplication>
#include <QStyle>

#include "DefaultMessages.h"
#include "DebugLibrary.h"



const char* CrystalFileSystemModel::headerTags[3] = {
	msg::explorerName,
	msg::explorerType, 
	msg::explorerSize
};

int CrystalFileSystemModel::sizeFloatPrecision = 2;

CrystalFileSystemModel::CrystalFileSystemModel(QObject* parent) : 
QAbstractTableModel(parent), 
_fileSystem(CrystalFileSystem::Create(SynopsisType::Folder)) {
}

int CrystalFileSystemModel::rowCount(const QModelIndex& parent) const {
	// DEV: return CURRENT synofolder childCount
	// DEV, nullptr root.
	return _fileSystem.childCount(_fileSystem.currentFolder());
}

int CrystalFileSystemModel::columnCount(const QModelIndex& parent) const {
	return _columnCount;
}

QVariant CrystalFileSystemModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid()) {
		return QVariant();
	}
	if (role == Qt::CheckStateRole) {
		return QVariant();
	}

	if (role == Qt::SizeHintRole) {
		return QSize(_nameWidth, _rowHeight);
	}
	
	auto synopsis = _fileSystem.child(_fileSystem.currentFolder(), index.row());
	if (synopsis) {
		switch (static_cast<Column>(index.column())) {
		case Column::Name :
			if (role == Qt::DecorationRole) {
				QStyle::StandardPixmap icon = QStyle::StandardPixmap::SP_FileIcon;
				if (_fileSystem.type(synopsis) == SynopsisType::Folder) {
					icon = QStyle::StandardPixmap::SP_DirIcon;
				}
				return QApplication::style()->standardIcon(icon);
			}
			return synopsis->name();
		case Column::Type:
			return _fileSystem.typeName(synopsis);
		case Column::Size:
			return legibleSizeString(index);
		}
	}
	else {
		return QVariant();
	}
	
}

QVariant CrystalFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role != Qt::DisplayRole) {
		// with any ohter role, but it just default now.
		return QAbstractTableModel::headerData(section, orientation, role);
	}
	if (orientation == Qt::Orientation::Horizontal) {
		return headerTags[section];
	}

	return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags CrystalFileSystemModel::flags(const QModelIndex& index) const {
	return QAbstractTableModel::flags(index);
}

bool CrystalFileSystemModel::open(const QString& path) {
	 _fileSystem.reset(CrystalFileSystem::Open(path));
	 auto synopsis = _fileSystem.child(_fileSystem.currentFolder(), 2);

	if (_fileSystem.mode() == CrystalFileSystem::Mode::Null) {
		return false;
	}

	return true;
}

bool CrystalFileSystemModel::setCurrent(const QModelIndex& index) {
	auto synopsis = _fileSystem.child(_fileSystem.currentFolder(), index.row());
	if (!synopsis) {
		return false;
	}
	_fileSystem.setCurrent(synopsis);
	return true;
}

bool CrystalFileSystemModel::setCurrentToParent() {
	if (_fileSystem.current() == _fileSystem.root()) {
		return false;
	}
	_fileSystem.setCurrentToParent();
}

QString CrystalFileSystemModel::currentDirName() const {
	return _fileSystem.currentFolder()->name();
}

QString CrystalFileSystemModel::legibleSizeString(const QModelIndex& index) const {
	auto synopsis = _fileSystem.child(_fileSystem.currentFolder(), index.row());
	uint64_t byteSize = 0;
	if (_fileSystem.isFile(synopsis)) {
		// DEV: replace here as filesystem method
		byteSize += reinterpret_cast<const CrystalSynopsisFile*>(synopsis)->dataSize();
	}
	else {
		// DEV: Calculate the folder size.
		return QString();
	}

	int unitIndex = 0;
	double legibleSize = byteSize;
	// Pray for in the program period, single file size will never over the sizeUnits length.
	while (legibleSize >= 1024) {
		legibleSize /= 1024;
		++unitIndex;
	}
	return QString::number(legibleSize, 'f', sizeFloatPrecision) + " " + msg::sizeUnits[unitIndex];
}
