#pragma once
#include <QTableVIew>
#include <QFileSystemModel>

#include "CrystalFileSystemModel.h"
#include "CrystalExplorerTabWidget.h"


class CrystalExplorerPage : public QTableView {
	Q_OBJECT
public:
	// DEVL add a widget tab handle
	CrystalExplorerPage(CrystalExplorerTabWidget* parent = nullptr);

	QString currentPathName();
	QIcon currentPathIcon();

	void setCurrentPathToParent();

protected:
	void resizeEvent(QResizeEvent* _event) override;


protected slots:
	void onDoubleClicked(const QModelIndex& modelIndex);

private:
	void setTabText(const QString& name);

	QFileSystemModel _fileModel;
	CrystalFileSystemModel _crystalFileModel;

	CrystalExplorerTabWidget* _parentTabWidget;

	bool _crystalModelEnabled;
};

