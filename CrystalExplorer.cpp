#include "CrystalExplorer.h"
#include <QResizeEvent>

#include "CrystalExplorerPage.h"
#include "DefaultWidgetConfigurations.h"

#include "DebugLibrary.h"

CrystalExplorer::CrystalExplorer(QWidget* parent) : 
CrystalFramelessWidget(parent) {
	ui.setupUi(this);
	setAttribute(Qt::WA_TranslucentBackground);

	connect(ui.tabWidgetMain, SIGNAL(tabBarChanged()), this, SLOT(onExplorerTabBarChanged()));

}

CrystalExplorer::~CrystalExplorer() {
}

void CrystalExplorer::resizeEvent(QResizeEvent* _event) {
	ui.centralWidget->resize(size());
	ui.tabWidgetMain->resize(width() - borderWidth() * 2, height() - borderWidth() * 2);

	if (_event->oldSize() != QSize(-1, -1)) {
		ui.horizontalWidgetWindowStatus->move(width() - _event->oldSize().width() + ui.horizontalWidgetWindowStatus->x(), ui.horizontalWidgetWindowStatus->y());
		clampWidgets();
	}

	// For Update tabbar width, Ugly.
	if (isMaximized() || halfScreenState()) {
		ui.tabWidgetMain->removeTab(ui.tabWidgetMain->addExplorerTab());
	}

	CrystalFramelessWidget::resizeEvent(_event);
}

void CrystalExplorer::closeEvent(QCloseEvent* _event) {
	// DEV: save active  tab to disk
	deleteLater();
	CrystalFramelessWidget::closeEvent(_event);
}


void CrystalExplorer::onClickedPushButtonAddTab() {
	int currentIndex = ui.tabWidgetMain->addExplorerTab();
	ui.tabWidgetMain->setCurrentIndex(currentIndex);
}

void CrystalExplorer::onClickedPushButtonClose() {
	close();
}

void CrystalExplorer::onClickedPushButtonHide() {
	showMinimized();
}

void CrystalExplorer::onClickedPushButtonFullScreen() {
	showMaximized();
}

void CrystalExplorer::onClickedPushButtonParentFolder() {
	ui.tabWidgetMain->changePathToParent(ui.tabWidgetMain->currentIndex());
}


void CrystalExplorer::onExplorerTabBarChanged() {
	clampWidgets();
}

void CrystalExplorer::clampWidgets() {
	int addTabButtonMaxX = 0;
	int currentX = ui.tabWidgetMain->tabBar()->width() + explorercfg::addTabButtonPadding;
	addTabButtonMaxX = ui.horizontalWidgetWindowStatus->x() - ui.pushButtonAddTab->width() - explorercfg::addTabButtonPadding;
	

	if (addTabButtonMaxX < currentX) {
		ui.pushButtonAddTab->move(addTabButtonMaxX, ui.pushButtonAddTab->y());
	}
	else {
		ui.pushButtonAddTab->move(currentX, ui.pushButtonAddTab->y());
	}

	ui.tabWidgetMain->tabBar()->setMaximumWidth(addTabButtonMaxX - explorercfg::addTabButtonPadding);
}
