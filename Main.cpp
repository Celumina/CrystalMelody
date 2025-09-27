#include "CrystalMelody.h"
#include <QtWidgets/QApplication>

#include "CrystalExplorer.h"
#include "CrystalFileSystem.h"

#include "DebugLibrary.h"

int main(int argc, char *argv[]){
	QApplication application(argc, argv);

	//auto cfs = CrystalFileSystem::Create(SynopsisType::Folder, "Thumbnails");
	//cfs.appendAllFiles("C:\\Users\\Ayameow\\Desktop\\Thumbnails");
	//cfs.saveAs("./Thumbnails.lna");

	// DL_SHOW(typeid(application).name())

	 auto ce = new CrystalExplorer;
	 ce->show();


	CrystalMelody window(argc, argv);
	window.show();
	return application.exec();
}
