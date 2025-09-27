#pragma once
#include <vector>
#include "FunctionalLibrary.h"

class CrystalIndexInfos {
public:
	struct Info {
		CrystalFileType type = CrystalFileType::unknown;
		int index = 0;
	};
	void append(Info info);
	void clear();
	void shuffle();
	void nextMedia();

	int currentIndex();
	CrystalFileType currentType();
	void setDataIndex(int index) noexcept;
	void setCurrentAsEnd();

private:
	void next() noexcept;
	void prev() noexcept;

	int dataIndex = 0;
	std::vector<Info> data;
};

