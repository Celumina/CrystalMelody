#include "CrystalExplorerPage.h"

#include <QFileInfo>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QHeaderView>

#include "FunctionalLibrary.h"
#include "DebugLibrary.h"

CrystalExplorerPage::CrystalExplorerPage(CrystalExplorerTabWidget* parent) :
QTableView(parent), 
_fileModel(this), 
_crystalFileModel(), 
_crystalModelEnabled(false), 
_parentTabWidget(parent) {
	setGeometry(parent->geometry());

	// Style
	verticalHeader()->setVisible(false);
	setAlternatingRowColors(true);
	setShowGrid(false);
	setFrameShape(QFrame::NoFrame);
	setFocusPolicy(Qt::NoFocus);
	setSelectionBehavior(SelectionBehavior::SelectRows);


	// Default Path
	// DEV: from lastest path
	 _fileModel.setRootPath(QDir::currentPath());
	 setModel(&_fileModel);
	 setRootIndex(_fileModel.index(QDir::currentPath()));


	// Signals
	connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onDoubleClicked(const QModelIndex&)));


	// DEV: add Default Style head file.
	// DEV: Customable qss file
	// DEV: Dynamic qss Editor
	setStyleSheet(fnlib::LoadStyleSheet("./StyleSheets/CrystalExplorerPage.qss"));

}

QString CrystalExplorerPage::currentPathName() {
	if (_crystalModelEnabled) {
		return _crystalFileModel.currentDirName();
	}
	return QFileInfo(_fileModel.rootPath()).fileName();
}

QIcon CrystalExplorerPage::currentPathIcon() {
	return QFileIconProvider().icon(QFileInfo(_fileModel.rootPath()));
}

void CrystalExplorerPage::setCurrentPathToParent() {
	if (_crystalModelEnabled) {
		if (!_crystalFileModel.setCurrentToParent()) {
			_crystalModelEnabled = false;
			setModel(&_fileModel);
			setRootIndex(_fileModel.index(_fileModel.rootPath()));
			setTabText(_fileModel.rootDirectory().dirName());
			return;
		}
		setTabText(_crystalFileModel.currentDirName());
		update();
		return;
	}

	if (_fileModel.rootPath() == "") {
		return;
	}

	QDir parentDir = _fileModel.rootPath();
	if (parentDir.cdUp()) {
		_fileModel.setRootPath(parentDir.path());
		setRootIndex(_fileModel.index(parentDir.path()));
	}
	else {
		_fileModel.setRootPath("");
		setRootIndex(QModelIndex());
	}
	setTabText(_fileModel.rootDirectory().dirName());
}

void CrystalExplorerPage::resizeEvent(QResizeEvent* _event) {
	resizeColumnsToContents();
	QTableView::resizeEvent(_event);
}

void CrystalExplorerPage::setTabText(const QString& text) {
	if (!_parentTabWidget) {
		return;
	}
	_parentTabWidget->setTabText(_parentTabWidget->indexOf(this), text);
	_parentTabWidget->emitTabBarChanged();
	// DEV: call _parentTab to sent tab width changed event.

}

void CrystalExplorerPage::onDoubleClicked(const QModelIndex& modelIndex) {
	if (_crystalModelEnabled) {
		// DEV: if set current no success, open this file
		_crystalFileModel.setCurrent(modelIndex);
		setTabText(_crystalFileModel.currentDirName());
		update();
		return;
	}

	auto filePath = _fileModel.filePath(modelIndex);

	// DEV: TEMP add regonize flag
	if (filePath.endsWith(".lna")) {
		if (_crystalFileModel.open(filePath)) {
			setModel(&_crystalFileModel);
			_crystalModelEnabled = true;
			setTabText(_crystalFileModel.currentDirName());
			return;
		}
	}

	QFileInfo fileInfo(filePath);
	if (fileInfo.isDir()) {
		_fileModel.setRootPath(filePath);
		setRootIndex(_fileModel.index(filePath));
		setTabText(fileInfo.baseName());
	}
	else{
		QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
	}
}

