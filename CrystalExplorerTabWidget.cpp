#include "CrystalExplorerTabWidget.h"

#include "CrystalExplorerPage.h"

#include "DefaultWidgetConfigurations.h"
#include "DebugLibrary.h"

CrystalExplorerTabWidget::CrystalExplorerTabWidget(QWidget* parent) : 
	QTabWidget(parent) {
	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));

}

int CrystalExplorerTabWidget::addTab(QWidget* page, const QString& label) {
	int tabIndex = QTabWidget::addTab(page, label);
	emit tabBarChanged();
	return tabIndex;
}


int CrystalExplorerTabWidget::addTab(QWidget* page, const QIcon& icon, const QString& label) {
	int tabIndex = QTabWidget::addTab(page, icon, label);
	emit tabBarChanged();
	return tabIndex;
}

int CrystalExplorerTabWidget::insertTab(int index, QWidget* page, const QString& label) {
	int tabIndex =  QTabWidget::insertTab(index, page, label);
	emit tabBarChanged();
	return tabIndex;

}

int CrystalExplorerTabWidget::insertTab(int index, QWidget* page, const QIcon& icon, const QString& label) {
	int tabIndex = QTabWidget::insertTab(index, page, icon, label);
	emit tabBarChanged();
	return tabIndex;
}

void CrystalExplorerTabWidget::removeTab(int index) {
	QTabWidget::removeTab(index);
	emit tabBarChanged();
}

int CrystalExplorerTabWidget::addExplorerTab() {
	auto* page = new CrystalExplorerPage(this);
	return addTab(page, page->currentPathIcon(), page->currentPathName());
}

void CrystalExplorerTabWidget::changePathToParent(int index) {
	if (index >= 0) {
		reinterpret_cast<CrystalExplorerPage*>(widget(index))->setCurrentPathToParent();
	}	
}

void CrystalExplorerTabWidget::emitTabBarChanged() {
	emit tabBarChanged();
}


void CrystalExplorerTabWidget::onTabCloseRequested(int index) {
	widget(index)->deleteLater();
	removeTab(index);
};