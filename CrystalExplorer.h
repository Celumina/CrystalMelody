#pragma once
#include "CrystalFramelessWidget.h"
#include "ui_CrystalExplorer.h"

class CrystalExplorer :
    public CrystalFramelessWidget {
    Q_OBJECT

public:
    CrystalExplorer(QWidget* parent = nullptr);
    ~CrystalExplorer();

protected:
    void resizeEvent(QResizeEvent* _event) override;
    void closeEvent(QCloseEvent* _event) override;

protected slots:
    void onClickedPushButtonAddTab();

    void onClickedPushButtonClose();
    void onClickedPushButtonHide();
    void onClickedPushButtonFullScreen();

    void onClickedPushButtonParentFolder();

    void onExplorerTabBarChanged();

private:
    void clampWidgets();

    Ui::CrystalExplorerClass ui;
};

