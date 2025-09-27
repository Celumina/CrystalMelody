#include "CrystalIndexInfos.h"
#include <algorithm>
#include <random>
void CrystalIndexInfos::append(Info info) {
	data.push_back(info);
}

void CrystalIndexInfos::clear() {
	data.clear();
}

void CrystalIndexInfos::shuffle() {
	std::shuffle(data.begin(), data.end(), std::mt19937(std::random_device{}()));
}

void CrystalIndexInfos::nextMedia() {
	for (int count = 0; count < data.size(); ++count) {
		next();
		if (data[dataIndex].type == CrystalFileType::audio ||
			data[dataIndex].type == CrystalFileType::video) {
			return;
		}
	}
	next();
}

int CrystalIndexInfos::currentIndex() {
	return data[dataIndex].index;
}

CrystalFileType CrystalIndexInfos::currentType() {
	return data[dataIndex].type;
}

void CrystalIndexInfos::setDataIndex(int index) noexcept {
	dataIndex = index;
}

void CrystalIndexInfos::setCurrentAsEnd() {
	dataIndex = data.size() - 1;
}

void CrystalIndexInfos::next() noexcept {
	if (dataIndex < data.size()) {
		++dataIndex;
	}
	else {
		dataIndex = 0;
	}
}

void CrystalIndexInfos::prev() noexcept {
	if (dataIndex > 0) {
		--dataIndex;
	}
	else {
		dataIndex = data.size() - 1;
	}
}
