#pragma once
#include <QTabWidget>

class CrystalExplorerTabWidget : public QTabWidget {
	Q_OBJECT
public:
	CrystalExplorerTabWidget(QWidget* parent = nullptr);

	// Overwrite Base
	int addTab(QWidget* page, const QString& label);
	int addTab(QWidget* page, const QIcon& icon, const QString& label);

	int insertTab(int index, QWidget* page, const QString& label);
	int insertTab(int index, QWidget* page, const QIcon& icon, const QString& label);


	void removeTab(int index);


	// Public
	int addExplorerTab();
	void changePathToParent(int index);
	void emitTabBarChanged();

signals:
	void tabBarChanged();

protected slots:
	void onTabCloseRequested(int index); 


protected:



private:
};

