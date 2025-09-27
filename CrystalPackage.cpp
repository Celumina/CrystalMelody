#include "CrystalPackage.h"

#include <QDir>

struct CrystalPackage::Pimpl {
	QFile package;
	IndexesType indexes;
	DirsType dirs;
};

struct CrystalPackage::IndexData {
	QString name;
	IndexSizeType dirIndex;
	DataSizeType offset;
	DataSizeType size;
};

CrystalPackage::Index::Index(const CrystalPackage* packagePtr, IndexSizeType indexOffset) :
	_thisPackage(packagePtr), _indexOffset(indexOffset) {
}

QString CrystalPackage::Index::name() const {
	return _thisPackage->data->indexes[_indexOffset].name;
}

QString CrystalPackage::Index::dir() const {
	return _thisPackage->data->dirs[_thisPackage->data->indexes[_indexOffset].dirIndex].absoluteFilePath().replace("/", "\\");
}

QString CrystalPackage::Index::path() const {
	return dir() + name();
}

CrystalPackage::DataSizeType CrystalPackage::Index::offset() const {
	return _thisPackage->data->indexes[_indexOffset].offset;
}

CrystalPackage::DataSizeType CrystalPackage::Index::size() const {
	return _thisPackage->data->indexes[_indexOffset].size;
}

bool CrystalPackage::Index::empty() const noexcept {
	if (!_thisPackage || _indexOffset >= _thisPackage->indexLength()) {
		return true;
	}
	return false;
}

CrystalPackage::Index& CrystalPackage::Index::operator++() noexcept {
	++_indexOffset;
	return *this;
}

CrystalPackage::Index& CrystalPackage::Index::operator--() noexcept {
	--_indexOffset;
	return *this;
}

CrystalPackage::Stream::Stream(DataSizeType dataSize) :
	_data(new char[dataSize]), _size(dataSize) {
}

CrystalPackage::Stream::Stream(Index index) :
	_data(new char[index.size()]), _size(index.size()) {
	index._thisPackage->data->package.seek(index.offset());
	setData(index._thisPackage->data->package, _size);
}


CrystalPackage::Stream::~Stream() {
	delete[] _data;
	_data = nullptr;
}

void CrystalPackage::Stream::setData(QFile& src, DataSizeType writeSize) {
	src.read(_data, _size > writeSize ? writeSize : _size);
}

const char* CrystalPackage::Stream::data() {
	return _data;
}

// see ctor, param can using akas, but ret_type not.
CrystalPackage::DataSizeType CrystalPackage::Stream::size() {
	return _size;
}

QString CrystalPackage::Stream::toString() {
	return QString::fromUtf8(_data, _size);
}

void CrystalPackage::pack(const QString& inDir, const QString& outDir) {
	DirsType paths;
	QString formatInDir = formatDirectory(inDir);
	QString formatOutDir = formatDirectory(outDir);
	loadFilePaths(formatInDir, paths);

	QString subInDir;
	qsizetype rBackslashPos = formatInDir.lastIndexOf("\\");
	subInDir = rBackslashPos == -1 ? formatInDir : formatInDir.mid(rBackslashPos, formatInDir.length() - rBackslashPos);
	QFile outPackage(formatOutDir + subInDir + ".crystal");
	outPackage.open(QIODevice::WriteOnly);
	if (!outPackage.isOpen()) {
		throw exception::outDirError;
	}
	if (paths.empty()) { //when input is a file, not a folder
		IndexSizeType filenameSize = subInDir.length();
		outPackage.write(reinterpret_cast<const char*>(&filenameSize), sizeof(IndexSizeType));
		outPackage.write(subInDir.toUtf8(), filenameSize);
		writeFile(formatInDir, outPackage);
		outPackage.close();
		return;
	}

	QString streamDir;
	for (const auto& path : paths) {
		QString subPath(path.absoluteFilePath().replace("/", "\\"));
		subPath.replace( formatInDir, "");
		qsizetype splitPos = subPath.lastIndexOf('\\');
		QString filename = subPath.mid(splitPos, subPath.length() - splitPos);
		QString subDir = subPath.mid(0, splitPos);

		// write to outPackage
		if (subDir != streamDir) {
			IndexSizeType subDirSize = subDir.toUtf8().length();
			IndexSizeType subDirFlag = subDirSize | dirFlag;
			outPackage.write(reinterpret_cast<const char*>(&subDirFlag), sizeof(IndexSizeType));
			outPackage.write(subDir.toUtf8());
			streamDir = subDir;
		}
		IndexSizeType filenameSize = filename.toUtf8().length();
		outPackage.write(reinterpret_cast<const char*>(&filenameSize), sizeof(IndexSizeType));
		outPackage.write(filename.toUtf8());
		writeFile(path.absoluteFilePath().replace("/", "\\"), outPackage);
	}
	outPackage.close();
}

CrystalPackage::CrystalPackage(const QString& packagePath) :
	data{ new Pimpl } {
	QString formatPath = formatDirectory(packagePath);
	data->package.setFileName(formatPath);
	data->package.open(QIODevice::ReadOnly);
	if (!data->package.isOpen()) {
		throw exception::pathAccessError;
	}
	DataSizeType packageSize = data->package.size();
	data->dirs.push_back(QFileInfo(""));
	while (data->package.pos() < packageSize) {
		IndexSizeType indexSize = 0;
		data->package.read(reinterpret_cast<char*>(&indexSize), sizeof(IndexSizeType));

		// if indexSize have dirFlag
		bool isDir = false;
		if ((indexSize & dirFlag) != 0) {
			indexSize ^= dirFlag;
			isDir = true;
		}
		Stream indexStream(indexSize);
		indexStream.setData(data->package, indexSize);

		// if indexStream is a dir
		if (isDir) {
			data->dirs.push_back(QFileInfo(indexStream.toString()));
			continue;
		}
		// if indexStream is a filename
		DataSizeType dataSize = 0;
		data->package.read(reinterpret_cast<char*>(&dataSize), sizeof(DataSizeType));
		data->indexes.push_back({ indexStream.toString(), static_cast<IndexSizeType>(data->dirs.size() - 1), static_cast<DataSizeType>(data->package.pos()), dataSize });
		data->package.seek(dataSize + data->package.pos());
	}
}

CrystalPackage::~CrystalPackage() {
	data->package.close();
}

void CrystalPackage::unpack(const QString& outDir) {
	QString formatOutDir = formatDirectory(outDir);
	while (formatOutDir.front() == '\\') {
		formatOutDir = formatOutDir.mid(1, formatOutDir.length() - 1);
	}
	QDir newDir;
	for (const auto& dir : data->dirs) {
		newDir.mkdir(formatOutDir + dir.absoluteFilePath().replace("/", "\\"));
	}

	for (Index index(this, 0); index._indexOffset < indexLength(); ++index) {
		unpackFile(formatOutDir, index);
	}
}

void CrystalPackage::unpack(const QString& outDir, const Index& index) {
	QString formatOutDir = formatDirectory(outDir);
	while (formatOutDir.front() == '\\') {
		formatOutDir = formatOutDir.mid(1, formatOutDir.length() - 1);
	}
	QDir newDir;
	newDir.mkdir(formatOutDir + index.dir());
	unpackFile(formatOutDir, index);
}

CrystalPackage::Index CrystalPackage::firstIndex() {
	return Index(this, 0);
}

CrystalPackage::Index CrystalPackage::lastIndex() {
	return Index(this, data->indexes.size() - 1);
}

CrystalPackage::Index CrystalPackage::findName(const QString& filename, const Index& offsetIndex) {
	Index currentIndex = offsetIndex;
	while (currentIndex._indexOffset < indexLength()) {
		if (currentIndex.name() == filename) {
			break;
		}
		++currentIndex;
	}
	return currentIndex;
}

CrystalPackage::Index CrystalPackage::findName(const QString& filename) {
	return findName(filename, Index(this, 0));
}

CrystalPackage::Index CrystalPackage::findPath(const QString& path) {
	Index currentIndex(this, 0);
	while (currentIndex._indexOffset < indexLength()) {
		if (currentIndex.path() == path) {
			break;
		}
		++currentIndex;
	}
	return currentIndex;
}

qsizetype CrystalPackage::indexLength() const {
	return data->indexes.size();
}

void CrystalPackage::loadFilePaths(const QString& inDir, DirsType& container) {
	QDir dir(inDir);
	container.append(dir.entryInfoList(QDir::Files));
	DirsType subDirs;
	subDirs.append(dir.entryInfoList(QDir::Dirs | QDir::NoDot | QDir::NoDotDot));
	for (const auto& dir : subDirs) {
		loadFilePaths(dir.absoluteFilePath().replace("/", "\\"), container);
	}
}

QString CrystalPackage::formatDirectory(const QString& dir) {
	QString formatDir = dir;
	formatDir.replace("//", "\\");
	formatDir.replace( "/", "\\");
	while (*(formatDir.cend() - 1) == '\\') {
		formatDir = formatDir.left(formatDir.length() - 1);
	}
	return formatDir;
}

inline void CrystalPackage::writeFile(const QString& src, QFile& dest) {
	QFile	srcFile(src);
	srcFile.open(QIODevice::ReadOnly);
	if (!srcFile.isOpen()) {
		throw exception::pathAccessError;
	}
	DataSizeType fileSize = srcFile.size();
	dest.write(reinterpret_cast<const char*>(&fileSize), sizeof(DataSizeType));
	writeData(srcFile, dest, fileSize);
	srcFile.close();
}

inline void CrystalPackage::writeData(QFile& src, QFile& dest, DataSizeType size) {
	Stream buffer(size > bufferSizeMax ? bufferSizeMax : size);
	while (size > bufferSizeMax) {
		buffer.setData(src, bufferSizeMax);
		dest.write(buffer.data(), bufferSizeMax);
		size -= bufferSizeMax;
	}
	buffer.setData(src, size);
	dest.write(buffer.data(), size);
}

inline void CrystalPackage::unpackFile(const QString& outDir, const Index& index) {
	QFile outFile(outDir + index.path());
	outFile.open(QIODevice::WriteOnly);
	if (!outFile.isOpen()) {
		throw exception::pathAccessError;
	}
	data->package.seek(index.offset());
	writeData(data->package, outFile, index.size());
	outFile.close();
}

