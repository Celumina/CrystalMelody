#include "CrystalFileSystem.h"
#include <filesystem>
#include <exception>
#include <random>

#include "DebugLibrary.h"

CrystalSynopsisBase::CrystalSynopsisBase() :
	_nameLength(0), 
	_parent(nullptr), 
	_firstChild(nullptr), 
	_sibling(nullptr) {
	
}

uint8_t CrystalSynopsisBase::typeID() const {
	return static_cast<uint8_t>(SynopsisType::Base);
}

void CrystalSynopsisBase::load(const char* address) {
	// ignore TypeID
	address += sizeof(uint8_t);
	
	_nameLength = *(reinterpret_cast<const decltype(_nameLength)*>(address));
	address += sizeof(_nameLength);
	
	_name = QString::fromUtf8(address, _nameLength);
	address += _nameLength;
}

uint32_t CrystalSynopsisBase::byteSize() const {
	return sizeof(typeID()) + sizeof(_nameLength) + _nameLength;
}

CrystalSynopsisStream CrystalSynopsisBase::dump() const {
	//	be careful! must explicit specify bytesize (otherwise polymophic would cause expection)
	CrystalSynopsisStream stream(CrystalSynopsisBase::byteSize());
	uint8_t id = typeID();
	stream.appendData(sizeof(id), reinterpret_cast<const char*>(&id));
	stream.appendData(sizeof(_nameLength), reinterpret_cast<const char*>(&_nameLength));
	stream.appendData(_nameLength, _name.toUtf8().data());

	return stream;
}

QString CrystalSynopsisBase::path() const {
	QString path;
	const CrystalSynopsisBase* cursor = this;
	while (cursor) {
		// DEV: custom spliter
		// check if is last elem, dont add spliter
		if (!path.isEmpty()) {
			path.push_front('\\');
		}
		path = cursor->_name + path;
		cursor = cursor->_parent;
	}
	return path;
}

uint16_t CrystalSynopsisBase::nameLength() const {
	return _nameLength;
}

const QString& CrystalSynopsisBase::name() const {
	return _name;
}

const QString& CrystalSynopsisBase::mappedPath() const {
	return _mappedPath;
}

void CrystalSynopsisBase::setName(const QString& name) {
	_name = name;
	_nameLength = name.length();
}

void CrystalSynopsisBase::setMappedPath(const QString& mappedPath) {
	_mappedPath = mappedPath;
}


const CrystalSynopsisBase* CrystalSynopsisBase::findChild(const QString& name) const {
	CrystalSynopsisBase* current = _firstChild.get();
	while (current) {
		if (current->_name == name) {
			return current;
		}
		current = current->_sibling.get();
	}
	return nullptr;
}

const CrystalSynopsisBase* CrystalSynopsisBase::findChild(uint32_t index) const {
	CrystalSynopsisBase* current = _firstChild.get();
	for (int i = 0; i < index; ++i) {
		if (current == nullptr) {
			break;
		}
		current = current->_sibling.get();
	}
	return current;
}

bool CrystalSynopsisBase::addChild(std::unique_ptr<CrystalSynopsisBase>&& synopsis) {
	if (!synopsis) {
		printf("<CrystalSynopsisBase::addChild> synopsis is a nullptr");
		return false;
	}
	if (synopsis->_parent != nullptr ||
		synopsis->_firstChild != nullptr ||
		synopsis->_sibling != nullptr) {
		printf("<CrystalSynopsisBase::addChild> synopsis's parent, firstChild and sibling must be nullptr.");
		return false;
	}

	synopsis->_parent = this;

	CrystalSynopsisBase* last = lastChild();
	// reset unque nullptr
	if (last) {
		last->_sibling.reset(synopsis.release());
	}
	else {
		_firstChild.reset(synopsis.release());
	}
	return true;
}

bool CrystalSynopsisBase::delChild(const CrystalSynopsisBase* child) {
	CrystalSynopsisBase* current = _firstChild.get();
	while (current) {
		if (current->_sibling.get() == child) {
			// release sibling's sibling before sibling reset (reset will del all managing data)
			// this action will delete all subchild.
			current->_sibling.reset(current->_sibling->_sibling.release());
			return true;
		}
		current = current->_sibling.get();
	}
	return false;
}

void CrystalSynopsisBase::delAllChildren() {
	_firstChild = nullptr;
}

// DEV: return a bool func
bool CrystalSynopsisBase::moveChild(CrystalSynopsisBase* destSynopsis, const CrystalSynopsisBase* child) {
	std::unique_ptr<CrystalSynopsisBase>* current = &_firstChild;
	CrystalSynopsisBase* prev = nullptr;
	while (current) {
		if ((*current).get() == child) {
			CrystalSynopsisBase* nextRaw = (*current)->_sibling.release();

			(*current)->_parent = destSynopsis;

			if (!destSynopsis->_firstChild) {
				destSynopsis->_firstChild.reset(current->release());
			}
			else {
				destSynopsis->lastChild()->_sibling.reset(current->release());
			}


			if (prev) {
				prev->_sibling.reset(nextRaw);
			}
			else {
				// when current is first and it moved out 
				_firstChild.reset(nextRaw);
			}
			return true;
		}

		prev = (*current).get();
		current = &(*current)->_sibling;
	}
	return false;
}

void CrystalSynopsisBase::moveAllChildren(CrystalSynopsisBase* destSynopsis) {
	CrystalSynopsisBase* current = _firstChild.get();
	while (current) {
		current->_parent = destSynopsis;
		current = current->_sibling.get();
	}
	
	if (!destSynopsis->_firstChild) {
		// if dest syno have not child, reset it first child
		destSynopsis->_firstChild.reset(_firstChild.release());
	}
	else {
		destSynopsis->lastChild()->_sibling.reset(_firstChild.release());
	}
}

QString CrystalSynopsisBase::toString() const {
	QString out;
	out += "Class: CrystalSynopsisBase\n";
	out += "Name Length: " + QString::number(_nameLength) + '\n';
	out += "Name: " + _name + '\n';
	return out;
}

CrystalSynopsisBase* CrystalSynopsisBase::firstChild() {
	return _firstChild.get();
}

CrystalSynopsisBase* CrystalSynopsisBase::lastChild() {
	CrystalSynopsisBase* current = _firstChild.get();
	while (current) {
		if (current->_sibling == nullptr) {
			return current;
		}
		current = current->_sibling.get();
	}
	return nullptr;
}

const CrystalSynopsisBase* CrystalSynopsisBase::findChild(const QString& name, const CrystalSynopsisBase* begin) const {
	return const_cast<CrystalSynopsisBase*>(this)->findChild(name, begin);
}

CrystalSynopsisBase* CrystalSynopsisBase::findChild(const QString& name, const CrystalSynopsisBase* begin) {
	if (begin && this != begin->parent()) {
		return nullptr;
	}

	CrystalSynopsisBase* current = _firstChild.get();
	if (begin) {
		current = const_cast<CrystalSynopsisBase*>(begin);
	}

	while (current) {
		if (current->name() == name) {
			break;
		}
		current = current->_sibling.get();
	}

	return current;
}

CrystalSynopsisStream::CrystalSynopsisStream(SizeType size) :
_offset(0), _size(size) {
	_data = new char[_size];
}

CrystalSynopsisStream::CrystalSynopsisStream(CrystalSynopsisStream&& old) noexcept :
_offset(old._offset), _size(old._size), _data(old._data) {
	// let old one dont release memory when it is destructs.
	old._data = nullptr;
}

CrystalSynopsisStream::~CrystalSynopsisStream() {
	delete[] _data;
	_data = nullptr;
}

bool CrystalSynopsisStream::appendData(SizeType dataSize, const char* data) {
	if (dataSize + _offset > _size) {
		return false;
	}
	memcpy(reinterpret_cast<void*>(_data + _offset), data, dataSize);
	_offset += dataSize;
	return true;
}

bool CrystalSynopsisStream::appendData(const CrystalSynopsisStream& stream) {
	return appendData(stream.size(), stream.data());
}

uint32_t CrystalSynopsisStream::size() const {
	return _size;
}

const char* CrystalSynopsisStream::data() const {
	return _data;
}

CrystalSynopsisFolder::CrystalSynopsisFolder() : 
CrystalSynopsisBase(), 
_childCount(0) {
}

uint8_t CrystalSynopsisFolder::typeID() const {
	return static_cast<uint8_t>(SynopsisType::Folder);
}

void CrystalSynopsisFolder::load(const char* address) {
	CrystalSynopsisBase::load(address); 
	address += CrystalSynopsisBase::byteSize();
	
	_childCount = *(reinterpret_cast<const decltype(_childCount)*>(address));
	address += sizeof(_childCount);
}

uint32_t CrystalSynopsisFolder::byteSize() const {
	return CrystalSynopsisBase::byteSize() + sizeof(_childCount);
}

CrystalSynopsisStream CrystalSynopsisFolder::dump() const {
	updateChildCount();
	CrystalSynopsisStream stream(CrystalSynopsisFolder::byteSize());
	stream.appendData(CrystalSynopsisBase::dump());
	stream.appendData(sizeof(_childCount), reinterpret_cast<const char*>(&_childCount));
	return stream;
}

uint32_t CrystalSynopsisFolder::childCount() {
	updateChildCount();
	return _childCount;
}

uint32_t CrystalSynopsisFolder::childCountRaw() const {
	return _childCount;
}

QString CrystalSynopsisFolder::toString() const {
	QString out = CrystalSynopsisBase::toString();
	out += "Derived Class Name: CrystalSynopsisFolder\n";
	out += "Child Count: " + QString::number(_childCount) + '\n';
	return out;
}

void CrystalSynopsisFolder::updateChildCount() const {
	const CrystalSynopsisBase* current = firstChild();
	uint32_t count = 0;
	while (current) {
		++count;
		current = current->sibling();
	}
	_childCount = count;
}

CrystalSynopsisFile::CrystalSynopsisFile() : 
CrystalSynopsisBase(), 
_dataOffset(0), 
_dataSize(0) {
}

uint8_t CrystalSynopsisFile::typeID() const {
	return static_cast<uint8_t>(SynopsisType::File);
}

void CrystalSynopsisFile::load(const char* address) {
	CrystalSynopsisBase::load(address);
	address += CrystalSynopsisBase::byteSize();

	_dataOffset = *(reinterpret_cast<const decltype(_dataOffset)*>(address));
	address += sizeof(_dataOffset);

	_dataSize = *(reinterpret_cast<const decltype(_dataSize)*>(address));
	address += sizeof(_dataSize);
}

uint32_t CrystalSynopsisFile::byteSize() const {
	return CrystalSynopsisBase::byteSize() + sizeof(_dataOffset) + sizeof(_dataSize);
}

CrystalSynopsisStream CrystalSynopsisFile::dump() const {
	CrystalSynopsisStream stream(CrystalSynopsisFile::byteSize());
	stream.appendData(CrystalSynopsisBase::dump());
	stream.appendData(sizeof(_dataOffset), reinterpret_cast<const char*>(&_dataOffset));
	stream.appendData(sizeof(_dataSize), reinterpret_cast<const char*>(&_dataSize));
	return stream;
}

uint64_t CrystalSynopsisFile::dataOffset() const {
	return _dataOffset;
}

uint64_t CrystalSynopsisFile::dataSize() const {
	return _dataSize;
}

void CrystalSynopsisFile::setDataOffset(uint64_t dataOffset) {
	_dataOffset = dataOffset;
}

void CrystalSynopsisFile::setDataSize(uint64_t dataSize) {
	_dataSize = dataSize;
}

QString CrystalSynopsisFile::toString() const {
	QString out = CrystalSynopsisBase::toString();
	out += "Derived Class Name: CrystalSynopsisFile\n";
	out += "Offset: " + QString::number(_dataOffset) + '\n';
	out += "Size: " + QString::number(_dataSize) + '\n';
	return out;
}

CrystalSynopsisMetaFolder::CrystalSynopsisMetaFolder() : 
CrystalSynopsisFolder() {
}

uint8_t CrystalSynopsisMetaFolder::typeID() const {
	return static_cast<uint8_t>(SynopsisType::MetaFolder);
}

void CrystalSynopsisMetaFolder::load(const char* address) {
	CrystalSynopsisFolder::load(address);
	address += CrystalSynopsisFolder::byteSize();

	_metaInfo.load(address);
	address += _metaInfo.byteSize();
}

uint32_t CrystalSynopsisMetaFolder::byteSize() const {
	return CrystalSynopsisFolder::byteSize() + _metaInfo.byteSize();
}

CrystalSynopsisStream CrystalSynopsisMetaFolder::dump() const {
	CrystalSynopsisStream stream(CrystalSynopsisMetaFolder::byteSize());
	stream.appendData(CrystalSynopsisFolder::dump());
	stream.appendData(_metaInfo.dump());
	return stream;
}

const CrystalSynopsisMetaInfo& CrystalSynopsisMetaFolder::metaInfo() const {
	return _metaInfo;
}

CrystalSynopsisMetaInfo& CrystalSynopsisMetaFolder::metaInfo() {
	return _metaInfo;
}

void CrystalSynopsisMetaFolder::setMetaInfo(const CrystalSynopsisMetaInfo& metaInfo) {
	_metaInfo = metaInfo;
}

QString CrystalSynopsisMetaFolder::toString() const {
	QString out = CrystalSynopsisFolder::toString();
	out += "Derived Class Name: CrystalSynopsisMetaFolder\n";
	out += _metaInfo.toString();
	return out;
}

CrystalSynopsisMetaFile::CrystalSynopsisMetaFile() : 
_encryptionCode(0) {
}

uint8_t CrystalSynopsisMetaFile::typeID() const {
	return static_cast<uint8_t>(SynopsisType::MetaFile);
}

void CrystalSynopsisMetaFile::load(const char* address) {
	CrystalSynopsisFile::load(address);
	address += CrystalSynopsisFile::byteSize();

	_metaInfo.load(address);
	address += _metaInfo.byteSize();

	_encryptionCode = *(reinterpret_cast<const decltype(_encryptionCode)*>(address));
	address += sizeof(_encryptionCode);
}

uint32_t CrystalSynopsisMetaFile::byteSize() const {
	return CrystalSynopsisFile::byteSize() + _metaInfo.byteSize() + sizeof(_encryptionCode);
}

CrystalSynopsisStream CrystalSynopsisMetaFile::dump() const {
	CrystalSynopsisStream stream(CrystalSynopsisMetaFile::byteSize());
	stream.appendData(CrystalSynopsisFile::dump());
	stream.appendData(_metaInfo.dump());
	stream.appendData(sizeof(_encryptionCode), reinterpret_cast<const char*>(&_encryptionCode));
	return stream;
}

const CrystalSynopsisMetaInfo& CrystalSynopsisMetaFile::metaInfo() const {
	return _metaInfo;
}

CrystalSynopsisMetaInfo& CrystalSynopsisMetaFile::metaInfo() {
	return _metaInfo;
}

void CrystalSynopsisMetaFile::setMetaInfo(const CrystalSynopsisMetaInfo& metaInfo) {
	_metaInfo = metaInfo;
}

QString CrystalSynopsisMetaFile::toString() const {
	QString out = CrystalSynopsisFile::toString();
	out += "Derived Class Name: CrystalSynopsisMetaFile\n";
	out += _metaInfo.toString();
	out += "Encryption Code: " + QString::number(_encryptionCode) + '\n';
	return out;
}


CrystalFileSystem::CrystalFileSystem(CrystalFileSystem&& rval) noexcept :
	_mode(rval._mode),
	_root(rval._root.release()),
	_header(rval._header.release()),
	_database(rval._database.release()), 
	_outFile(rval._outFile), 
	_synopsesSize(rval._synopsesSize), 
	_thumbnailsSize(rval._thumbnailsSize), 
	_dataSize(rval._dataSize), 
	_currentSynopsis(rval._currentSynopsis), 
	_currentAccount(rval._currentAccount) {
	_file.swap(rval._file);
	_region.swap(rval._region);
}

CrystalFileSystem::~CrystalFileSystem() {
}

CrystalFileSystem CrystalFileSystem::Create(SynopsisType rootType, const QString& rootName) {
	CrystalFileSystem cfs;
	cfs._mode = Mode::Write;
	cfs.createSynopsisChild(rootType);
	cfs._root->setName(rootName);
	cfs._header.reset(new CrystalSynopsisHeader{"LNA", 0, 0}); // "Layered Node Archive"
	cfs._currentSynopsis = cfs._root.get();
	return cfs;
}

CrystalFileSystem CrystalFileSystem::Open(const QString& path) {
	if (!std::filesystem::exists(path.toUtf8().data())) {
		// DEV: Log it to file
		printf("<CrystalFileSystem::Open> Path not exists.");
		return CrystalFileSystem();
	}
	CrystalFileSystem cfs;
	cfs._mode = Mode::Read;
	boost::interprocess::file_mapping inFile(path.toUtf8().data(), boost::interprocess::read_write);
	boost::interprocess::mapped_region inRegion(inFile, boost::interprocess::read_write);
	cfs._file.swap(inFile);
	cfs._region.swap(inRegion);

	uint64_t cursor = 0;

	cfs._header.reset(new CrystalSynopsisHeader);
	memcpy(cfs._header.get(), cfs.mappedData(cursor), sizeof(CrystalSynopsisHeader));
	cursor += sizeof(CrystalSynopsisHeader);

	QString magic = cfs._header.get()->_magic;
	
	if (magic == "CEL") {
		cfs._database.reset(new CrystalSynopsisDatabase);
		cfs._database->load(cfs.mappedData(cursor));
		cursor += cfs._database->byteSize();
	}
	else if (magic == "LNA") {
		// Nothing needs to be done here.
	}
	else {
		// Undefined Magic, meaning it's not a valid crystal file.
		return CrystalFileSystem();
	}

	cfs.loadSynopses(cfs.mappedData(cursor));
		
	return cfs;
	
}

std::vector<QString> CrystalFileSystem::PathLayeredNames(const QString& path) {
	QString standardizedPath = path.right(path.length() - path.indexOf(':') - 1);
	standardizedPath = standardizedPath.replace('/', '\\');

	if (standardizedPath[0] == '\\') {
		standardizedPath = standardizedPath.right(standardizedPath.length() - 1);
	}
	if (standardizedPath.last(1) != '\\') {
		standardizedPath.push_back('\\');
	}

	std::vector<QString> names;

	int64_t cursor = 0;
	int64_t nameLength = 0;
	while (true) {
		nameLength = standardizedPath.indexOf('\\', cursor) - cursor;
		if (nameLength <= 0) {
			break;
		}
		names.push_back(standardizedPath.mid(cursor, nameLength));
		cursor += nameLength + 1;
	}

	return names;
}

void CrystalFileSystem::reset(CrystalFileSystem&& rval) {
	// DEV: Modification is necessary when a new attribute is appended.
	_mode = rval._mode;
	_root.reset(rval._root.release());
	_header.reset(rval._header.release());
	_database.reset(rval._database.release());

	_outFile = rval._outFile;

	_synopsesSize = rval._synopsesSize;
	_thumbnailsSize = rval._thumbnailsSize;
	_dataSize = rval._dataSize;
	_currentSynopsis = rval._currentSynopsis;
	_currentAccount = rval._currentAccount;

	_file.swap(rval._file);
	_region.swap(rval._region);
}

CrystalFileSystem::State CrystalFileSystem::appendFile(const QString& path) {
	modeVerification(Mode::Write);

	if (!isFolder(_currentSynopsis)) {
		return State::SynopsisTypeCannotHoldsChild;
	}

	QByteArray pathByte(std::move(path.toUtf8()));
	if (!std::filesystem::exists(pathByte.data())) {
		return State::PathNotExists;
	}

	QString name = std::filesystem::path(pathByte.data()).filename().string().data();

	// you can append a folder also
	if (std::filesystem::is_directory(pathByte.data())) {
		std::unique_ptr<CrystalSynopsisFolder> synopsis(new CrystalSynopsisFolder);
		synopsis->setName(name);
		synopsis->setMappedPath(path);
		_currentSynopsis->addChild(std::move(synopsis));
	}
	else {
		std::unique_ptr<CrystalSynopsisFile> synopsis(new CrystalSynopsisFile);
		// damn it, is so hard.
		synopsis->setName(name);
		synopsis->setMappedPath(path);
		_currentSynopsis->addChild(std::move(synopsis));
		// check file exists in writeData
	}

	return State::Succeeded;
}

CrystalFileSystem::State CrystalFileSystem::appendAllFiles(const QString& dir) {
	modeVerification(Mode::Write);

	QString standardDir = dir;
	if (standardDir[standardDir.length() - 1] != '\\' && standardDir[standardDir.length() - 1] != '/') {
		standardDir += '\\';
	}

	QByteArray dirByte(std::move(dir.toUtf8()));

	if (!std::filesystem::exists(dirByte.data())) {
		return State::PathNotExists;
	}

	for (const auto& entry : std::filesystem::recursive_directory_iterator(dirByte.data())) {
		QString absolutePath = entry.path().string().data();
		auto relativePath = absolutePath.mid(standardDir.length(), absolutePath.length() - standardDir.length());
		State status = appendLayeredFile(absolutePath, relativePath);
		if (status != State::Succeeded) {
			printf("<CrystalFileSystem::appendAllFiles> Expection Status Code: %d", static_cast<int>(status));
		}
	}
	return State::Succeeded;
}

CrystalFileSystem::State CrystalFileSystem::appendLayeredFile(const QString& absolutePath, const QString& relativePath) {
	modeVerification(Mode::Write);

	if (!std::filesystem::exists(absolutePath.toUtf8().data())) {
		return State::PathNotExists;
	}

	auto names = PathLayeredNames(relativePath);

	// one-layer file or folder
	if (names.size() <= 1) {
		appendFile(absolutePath);
		return State::Succeeded;
	}

	CrystalSynopsisBase* destSynopsis = _currentSynopsis;
	// name which is last one could be a file, ignore it.
	for (auto iter = names.begin(); iter < --names.end(); ++iter) {
		CrystalSynopsisBase* child = destSynopsis->findChild(*iter);
		if (child) {
			destSynopsis = child;
		}
		else {
			destSynopsis = createSynopsisChild(SynopsisType::Folder, destSynopsis);
			destSynopsis->setName(*iter);
		}
	}

	CrystalSynopsisBase* currentTemp = _currentSynopsis;
	_currentSynopsis = destSynopsis;
	// status already check in front of func
	appendFile(absolutePath);
	_currentSynopsis = currentTemp;

	return State::Succeeded;
}


const CrystalSynopsisFolder* CrystalFileSystem::createFolder(const QString& name) {
	modeVerification(Mode::Write);

	if (!isFolder( _currentSynopsis)) {
		return nullptr;
	}
	CrystalSynopsisFolder* folder = reinterpret_cast<CrystalSynopsisFolder*>(createSynopsisChild(SynopsisType::Folder, _currentSynopsis));
	folder->setName(name);

	return folder;
}

CrystalSynopsisBase* CrystalFileSystem::createSynopsisChild(SynopsisType type, CrystalSynopsisBase* destParent) {
	std::unique_ptr<CrystalSynopsisBase> temp;
	switch (type) {
	case SynopsisType::Base:
		// Can't not create a SynopsisBase
		// temp = std::make_unique<CrystalSynopsisBase>();
		break;
	case SynopsisType::Reference:
		temp = std::make_unique<CrystalSynopsisReference>();
		break;
	case SynopsisType::Folder:
		temp = std::make_unique<CrystalSynopsisFolder>();
		break;
	case SynopsisType::File:
		temp = std::make_unique<CrystalSynopsisFile>();
		break;
	case SynopsisType::MetaFolder:
		temp = std::make_unique<CrystalSynopsisMetaFolder>();
		break;
	case SynopsisType::MetaFile:
		temp = std::make_unique<CrystalSynopsisMetaFile>();
		break;
	default:
		return nullptr;
	}

	if (destParent) {
		destParent->addChild(std::move(temp));
		return destParent->lastChild();
	}
	// if root is empty and dest empty, create as root
	else if (!_root) {
		_root.reset(temp.release());
		return _root.get();
	}
	else {
		return nullptr;
	}
}

void CrystalFileSystem::loadSynopses(const char* address) {
	std::vector<uint32_t> childCounts;
	do {
		_currentSynopsis = createSynopsisChild(SynopsisType(*address), _currentSynopsis);
		_currentSynopsis->load(address);
		address += _currentSynopsis->byteSize();

		if (isFolder(_currentSynopsis)) {
			if (childCounts.size()) {
				--childCounts[childCounts.size() - 1];
			}
			childCounts.push_back(reinterpret_cast<const CrystalSynopsisFolder*>(_currentSynopsis)->childCountRaw());
		}
		else {
			--childCounts[childCounts.size() - 1];
			_currentSynopsis = const_cast<CrystalSynopsisBase*>(_currentSynopsis->parent());
		}		

		// there [while] is used for jump out of multi empty folder like:
		// folder_count: 0
		//	......		|__folder_count: 0
		while (childCounts.size() && childCounts[childCounts.size() - 1] <= 0) {
			childCounts.pop_back();
			_currentSynopsis = const_cast<CrystalSynopsisBase*>(_currentSynopsis->parent());
		}
	} while (_currentSynopsis);
	_currentSynopsis = _root.get();
}

// DEV: set meta's accountSHA, encript with other info to defend modify(this func define in database)
// so meta's accountSHA will change after set info, need to update before write
void CrystalFileSystem::updateSynopses() {
	// update synopsis size
	_synopsesSize = 0;
	_synopsesSize += sizeof(CrystalSynopsisHeader);
	if (_header->_magic == "CDB") {
		_synopsesSize += _database->byteSize();
	}

	auto updateSynopsesSizeLambda = [](const CrystalSynopsisBase* synopsis, CrystalFileSystem* cfs) {
		cfs->_synopsesSize += synopsis->byteSize();
	};

	forEachSynopsis(updateSynopsesSizeLambda, this);

	// update thumbnail synoInfo and size
	_thumbnailsSize = 0;
	auto updateThumbnailsSizeLambda = [](const CrystalSynopsisBase* synopsis, CrystalFileSystem* cfs) {
		// CONST CAST MARK
		CrystalSynopsisBase* synopsisNonConst = const_cast<CrystalSynopsisBase*>(synopsis);

		CrystalSynopsisMetaInfo* metaInfoHandle = nullptr;
		if (SynopsisType(synopsis->typeID()) == SynopsisType::MetaFolder) {
			CrystalSynopsisMetaFolder* synopsisMetaFolder = reinterpret_cast<CrystalSynopsisMetaFolder*>(synopsisNonConst);
			metaInfoHandle = &synopsisMetaFolder->metaInfo();
		}
		else if (SynopsisType(synopsis->typeID()) == SynopsisType::MetaFile) {
			CrystalSynopsisMetaFile* synopsisMetaFile = reinterpret_cast<CrystalSynopsisMetaFile*>(synopsisNonConst);
			metaInfoHandle = &synopsisMetaFile->metaInfo();
		}
		else {
			return;
		}
		auto thumbnailPath = std::filesystem::path(metaInfoHandle->mappedThumbnailPath().toUtf8().data());
		// open mode dont update from mappedPath
		if (cfs->_mode == Mode::Read) {
			cfs->_thumbnailsSize += metaInfoHandle->thumbnailSize();
		}
		else if (!std::filesystem::exists(thumbnailPath)) {
			metaInfoHandle->setThumbnailOffset(0);
			metaInfoHandle->setThumbnailSize(0);
			metaInfoHandle->setMappedThumbnailPath("");
		}
		else {
			// there _thumbnailsSize not a final value, is a current value 
			metaInfoHandle->setThumbnailOffset(cfs->_synopsesSize + cfs->_thumbnailsSize);
			metaInfoHandle->setThumbnailSize(std::filesystem::file_size(thumbnailPath));
			cfs->_thumbnailsSize += metaInfoHandle->thumbnailSize();
		}
	};

	forEachSynopsis(updateThumbnailsSizeLambda, this);

	_dataSize = 0;
	auto updateSynopsisDataLambda = [](const CrystalSynopsisBase* synopsis, CrystalFileSystem* cfs) {
		if (!isFile(synopsis)) {
			return;
		}
		CrystalSynopsisBase* synopsisNonConst = const_cast<CrystalSynopsisBase*>(synopsis);
		CrystalSynopsisFile* synopsisFile = reinterpret_cast<CrystalSynopsisFile*>(synopsisNonConst);

		auto path = std::filesystem::path(synopsis->mappedPath().toUtf8().data());

		if (cfs->_mode == Mode::Read) {
			cfs->_dataSize += synopsisFile->dataSize();
		}
		else if (!std::filesystem::exists(path)) {
			synopsisFile->setDataOffset(0);
			synopsisFile->setDataSize(0);
			synopsisFile->setMappedPath("");
		}
		else {
			synopsisFile->setDataOffset(cfs->_synopsesSize + cfs->_thumbnailsSize + cfs->_dataSize);
			synopsisFile->setDataSize(std::filesystem::file_size(path));
			cfs->_dataSize += synopsisFile->dataSize();
		}
	};

	forEachSynopsis(updateSynopsisDataLambda, this);
}

void CrystalFileSystem::writeSynopses() {
	fwrite(_header.get(), sizeof(char), sizeof(CrystalSynopsisHeader), _outFile);

	if (_header->_magic == "CDB") {
		uint32_t databaseSize = _database->byteSize();
		fwrite(_database->dump().data(), sizeof(char), _database->byteSize(), _outFile);
	}

	auto writeSynopsisLambda = [](const CrystalSynopsisBase* synopsis, CrystalFileSystem* cfs) {
		fwrite(synopsis->dump().data(), sizeof(char), synopsis->byteSize(), cfs->_outFile);
	};

	forEachSynopsis(writeSynopsisLambda, this);
}

void CrystalFileSystem::writeThumbnails() {
	auto writeThumbnailLambda = [](const CrystalSynopsisBase* synopsis, CrystalFileSystem* cfs) {
		const CrystalSynopsisMetaInfo* metaInfoHandle = nullptr;
		if (SynopsisType(synopsis->typeID()) == SynopsisType::MetaFolder) {
			const CrystalSynopsisMetaFolder* synopsisMetaFolder = reinterpret_cast<const CrystalSynopsisMetaFolder*>(synopsis);
			metaInfoHandle = &synopsisMetaFolder->metaInfo();
		}
		else if (SynopsisType(synopsis->typeID()) == SynopsisType::MetaFile) {
			const CrystalSynopsisMetaFile* synopsisMetaFile = reinterpret_cast<const CrystalSynopsisMetaFile*>(synopsis);
			metaInfoHandle = &synopsisMetaFile->metaInfo();
		}
		else {
			return;
		}

		if (metaInfoHandle->thumbnailSize() <= 0) {
			return;
		}

		boost::interprocess::file_mapping thumbnailFile(metaInfoHandle->mappedThumbnailPath().toUtf8().data(), boost::interprocess::read_write);
		boost::interprocess::mapped_region region(thumbnailFile, boost::interprocess::read_write);
		fwrite(region.get_address(), sizeof(char), metaInfoHandle->thumbnailSize(), cfs->_outFile);
	};

	if (mode() == Mode::Read) {
		// open mode read from lna dont for each
		fwrite(mappedData(_synopsesSize), sizeof(char), _thumbnailsSize, _outFile);
	}
	else {
		forEachSynopsis(writeThumbnailLambda, this);
	}

}

void CrystalFileSystem::writeData() {
	auto writeDataLambda = [](const CrystalSynopsisBase* synopsis, CrystalFileSystem* cfs) {
		if (!isFile(synopsis)) {
			return;
		}

		const CrystalSynopsisFile* synopsisFile = reinterpret_cast<const CrystalSynopsisFile*>(synopsis);
		if (synopsisFile->dataSize() <= 0) {
			return;
		}

		boost::interprocess::file_mapping dataFile(synopsisFile->mappedPath().toUtf8().data(), boost::interprocess::read_write);
		boost::interprocess::mapped_region region(dataFile, boost::interprocess::read_write);
		fwrite(region.get_address(), sizeof(char), synopsisFile->dataSize(), cfs->_outFile);
	};

	if (mode() == Mode::Read) {
		// open mode read from lna dont for each
		fwrite(mappedData(_synopsesSize + _thumbnailsSize), sizeof(char), _dataSize, _outFile);
	}
	else {
		forEachSynopsis(writeDataLambda, this);
	}
	
}

CrystalFileSystem::State CrystalFileSystem::exportFileImplement(const CrystalSynopsisBase* synopsis, const CrystalFileSystem* cfs, const QString& dir) {
	if (!std::filesystem::exists(dir.toUtf8().data())) {
		return State::PathNotExists;
	}

	if (isFolder(synopsis)) {
		QByteArray currentDirByte(std::move((dir + '\\' + synopsis->path()).toUtf8()));

		if (!std::filesystem::exists(currentDirByte.data())) {
			if (!std::filesystem::create_directory(currentDirByte.data())) {
				return State::PathInaccessible;
			}
		}
		return State::Succeeded;
	}

	const CrystalSynopsisFile* synopsisFile = reinterpret_cast<const CrystalSynopsisFile*>(synopsis);
	QString standardPath = dir + '\\' + synopsisFile->path();

	// if same name syno in the same layer, only one of them can be export
	// but you can find another one in cfs and export it in other ways.
	// DEV: make overwrite log

	FILE* out = nullptr;
	if (fopen_s(&out, standardPath.toUtf8().data(), "wb")) {
		return State::PathInaccessible;
	};
	// DEV: different offset, but why?
	fwrite(cfs->mappedData(synopsisFile->dataOffset()), sizeof(char), synopsisFile->dataSize(), out);
	fclose(out);
	return State::Succeeded;
}

inline void CrystalFileSystem::modeVerification(Mode mode) const {
	if (mode != _mode) {
			throw State::ModeNotSupported; 
	}
}

bool CrystalFileSystem::isFolder(const CrystalSynopsisBase* synopsis) {
	// 0b0000'0011 is BaseType bit flag.
	// == has higher precedent than &.
	return (synopsis->typeID() & 0b0000'0011) == static_cast<uint8_t>(SynopsisType::Folder);
}

bool CrystalFileSystem::isFile(const CrystalSynopsisBase* synopsis) {
	return (synopsis->typeID() & 0b0000'0011) == static_cast<uint8_t>(SynopsisType::File);
}

bool CrystalFileSystem::isMeta(const CrystalSynopsisBase* synopsis) {
	return synopsis->typeID() & 0b0000'0100;
}

CrystalFileSystem::CrystalFileSystem() :
	_mode(Mode::Null), 
	_root(nullptr), 
	_header(nullptr),
	_database(nullptr), 
	_outFile(nullptr), 
	_file(boost::interprocess::file_mapping()),
	_region(boost::interprocess::mapped_region()),
	_synopsesSize(0), 
	_thumbnailsSize(0), 
	_dataSize(0), 
	_currentSynopsis(nullptr), 
	_currentAccount(nullptr) {
	// DEV: Load Synopsis
	// static_cast<char*>(_region.get_address())
}

const char* CrystalFileSystem::mappedData(uint64_t offset) const {
	return reinterpret_cast<const char*>(_region.get_address()) + offset;
}

char* CrystalFileSystem::mappedData(uint64_t offset) {
	return reinterpret_cast<char*>(_region.get_address()) + offset;
}

CrystalSynopsisReference::CrystalSynopsisReference() : 
	CrystalSynopsisBase() {}

uint8_t CrystalSynopsisReference::typeID() const {
	return static_cast<uint8_t>(SynopsisType::Reference);
}

QString CrystalSynopsisReference::toString() const {
	QString out = CrystalSynopsisBase::toString();
	out += "Derived Class Name: CrystalSynopsisReference\n";
	return out;
}

void CrystalSynopsisMetaInfo::load(const char* address) {
	_thumbnailOffset = *(reinterpret_cast<const decltype(_thumbnailOffset)*>(address));
	address += sizeof(_thumbnailOffset);

	_thumbnailSize = *(reinterpret_cast<const decltype(_thumbnailSize)*>(address));
	address += sizeof(_thumbnailSize);

	_createdTime = *(reinterpret_cast<const decltype(_createdTime)*>(address));
	address += sizeof(_createdTime);

	_lastAccessTime = *(reinterpret_cast<const decltype(_lastAccessTime)*>(address));
	address += sizeof(_lastAccessTime);

	_metaType = *(reinterpret_cast<const decltype(_metaType)*>(address));
	address += sizeof(_metaType);

	_version = *(reinterpret_cast<const decltype(_version)*>(address));
	address += sizeof(_version);

	_securityLevel = *(reinterpret_cast<const decltype(_securityLevel)*>(address));
	address += sizeof(_securityLevel);

	memcpy(&_fileMD5, address, sizeof(_fileMD5));
	address += sizeof(_fileMD5);

	memcpy(&_accountMD5, address, sizeof(_accountMD5));
	address += sizeof(_accountMD5);

	memcpy(&_description, address, sizeof(_description));
	address += sizeof(_description);

	memcpy(&_noUsed, address, sizeof(_noUsed));
	address += sizeof(_noUsed);
}

uint32_t CrystalSynopsisMetaInfo::byteSize() const {
	return sizeof(_thumbnailOffset) + 
		sizeof(_thumbnailSize) +
		sizeof(_createdTime) +
		sizeof(_lastAccessTime) +
		sizeof(_metaType) +
		sizeof(_version) +
		sizeof(_securityLevel) +
		sizeof(_fileMD5) +
		sizeof(_accountMD5) +
		sizeof(_description) +
		sizeof(_noUsed);
}

CrystalSynopsisStream CrystalSynopsisMetaInfo::dump() const {
	CrystalSynopsisStream stream(byteSize());
	stream.appendData(sizeof(_thumbnailOffset), reinterpret_cast<const char*>(&_thumbnailOffset));
	stream.appendData(sizeof(_thumbnailSize), reinterpret_cast<const char*>(&_thumbnailSize));
	stream.appendData(sizeof(_createdTime), reinterpret_cast<const char*>(&_createdTime));
	stream.appendData(sizeof(_lastAccessTime), reinterpret_cast<const char*>(&_lastAccessTime));
	stream.appendData(sizeof(_metaType), reinterpret_cast<const char*>(&_metaType));
	stream.appendData(sizeof(_version), reinterpret_cast<const char*>(&_version));
	stream.appendData(sizeof(_fileMD5), reinterpret_cast<const char*>(&_fileMD5));
	stream.appendData(sizeof(_accountMD5), reinterpret_cast<const char*>(&_accountMD5));
	stream.appendData(sizeof(_description), reinterpret_cast<const char*>(&_description));
	stream.appendData(sizeof(_noUsed), reinterpret_cast<const char*>(&_noUsed));
	return stream;
}

uint64_t CrystalSynopsisMetaInfo::thumbnailOffset() const {
	return _thumbnailOffset;
}

uint32_t CrystalSynopsisMetaInfo::thumbnailSize() const {
	return _thumbnailSize;
}

uint64_t CrystalSynopsisMetaInfo::createdTime() const {
	return _createdTime;
}

uint64_t CrystalSynopsisMetaInfo::lastAccessTime() const {
	return _lastAccessTime;
}

CrystalSynopsisMetaInfo::MetaType CrystalSynopsisMetaInfo::metaType() const {
	return _metaType;
}

uint16_t CrystalSynopsisMetaInfo::version() const {
	return _version;
}

CrystalSynopsisMetaInfo::Security CrystalSynopsisMetaInfo::securityLevel() const {
	return _securityLevel;
}

const char* CrystalSynopsisMetaInfo::fileMD5() const {
	return _fileMD5;
}

const char* CrystalSynopsisMetaInfo::accountMD5() const {
	return _accountMD5;
}

const char* CrystalSynopsisMetaInfo::description() const {
	return _description;
}

uint32_t CrystalSynopsisMetaInfo::descriptionSize() const {
	return sizeof(_description);
}

const QString& CrystalSynopsisMetaInfo::mappedThumbnailPath() const {
	return _mappedthumbnailPath;
}

void CrystalSynopsisMetaInfo::setThumbnailOffset(uint64_t thumbnailOffset) {
	_thumbnailOffset = thumbnailOffset;
}

void CrystalSynopsisMetaInfo::setThumbnailSize(uint32_t thumbnailSize) {
	_thumbnailSize = thumbnailSize;
}

void CrystalSynopsisMetaInfo::setCreatedTime(uint64_t createdTime) {
	_createdTime = createdTime;
}

void CrystalSynopsisMetaInfo::setLastAccessTime(uint64_t lastAccessTime) {
	_lastAccessTime = lastAccessTime;
}

void CrystalSynopsisMetaInfo::setMetaType(MetaType metaType) {
	_metaType = metaType;
}

void CrystalSynopsisMetaInfo::setVersion(uint16_t version) {
	_version = version;
}

void CrystalSynopsisMetaInfo::setSecurityLevel(Security securityLevel) {
	_securityLevel = securityLevel;
}

void CrystalSynopsisMetaInfo::setMappedThumbnailPath(const QString& mappedThumbnailPath) {
	_mappedthumbnailPath = mappedThumbnailPath;
}

QString CrystalSynopsisMetaInfo::toString() const {
	QString out;
	out += "Thumbnail Offset: " + QString::number(_thumbnailOffset) + '\n';
	out += "Thumbnail Size: " + QString::number(_thumbnailSize) + '\n';
	out += "Create Time: " + QString(ctime(&(_createdTime)));
	out += "Last Access Time: " + QString(ctime(&(_lastAccessTime)));
	out += "Meta Type: " + QString::number(static_cast<uint16_t>(_metaType)) + '\n';
	out += "Version Type: " + QString::number(_version) + '\n';
	out += "Security Level: " + QString::number(static_cast<int8_t>(_securityLevel)) + '\n';
	out += "File MD5: " + QString::fromLatin1(_fileMD5, sizeof(_fileMD5)) + '\n';
	out += "Account MD5: " + QString::fromLatin1(_accountMD5, sizeof(_accountMD5)) + '\n';
	out += "Description: " + QString::fromUtf8(_description, sizeof(_description)) + '\n';
	return out;
}

const CrystalSynopsisBase* CrystalSynopsisBase::parent() const {
	return _parent;
}

const CrystalSynopsisBase* CrystalSynopsisBase::firstChild() const {
	return _firstChild.get();
}

const CrystalSynopsisBase* CrystalSynopsisBase::sibling() const {
	return _sibling.get();
}

const CrystalSynopsisBase* CrystalSynopsisBase::lastChild() const {
	return const_cast<CrystalSynopsisBase&>(*this).lastChild();
}

// watting for load constructor.
CrystalSynopsisDatabase::CrystalSynopsisDatabase() : 
_databaseID(0), 
_accountCount(0), 
_accountCapacity(0) {
}

// create a new database constructor
CrystalSynopsisDatabase::CrystalSynopsisDatabase(uint32_t capacity) : 
	_databaseID(RandomID()),
	_accountCount(0),
	_accountCapacity(capacity) {}

void CrystalSynopsisDatabase::load(const char* address) {
	_databaseID = *(reinterpret_cast<const decltype(_databaseID)*>(address));
	address += sizeof(_databaseID);

	_accountCount = *(reinterpret_cast<const decltype(_accountCount)*>(address));
	address += sizeof(_accountCount);

	_accountCapacity = *(reinterpret_cast<const decltype(_accountCapacity)*>(address));
	address += sizeof(_accountCapacity);

	for (int index = 0; index < _accountCount; ++index) {
		_accountIDs.push_back(*(reinterpret_cast<const AccountIDType*>(address)));
		address += sizeof(AccountIDType);

		_passwordSHAs.push_back(*(reinterpret_cast<const PasswordSHAType*>(address)));
		address += sizeof(PasswordSHAType);

		_accountDescriptions.push_back(*(reinterpret_cast<const AccountDescriptionType*>(address)));
		address += sizeof(AccountDescriptionType);
	}
}

uint32_t CrystalSynopsisDatabase::byteSize() const {
	return sizeof(_databaseID) + sizeof(_accountCount) + sizeof(_accountCapacity) + 
		sizeof(AccountIDType) * _accountCapacity +
		sizeof(PasswordSHAType) * _accountCapacity +
		sizeof(AccountDescriptionType) * _accountCapacity;
}

CrystalSynopsisStream CrystalSynopsisDatabase::dump() const {
	CrystalSynopsisStream stream(byteSize());
	stream.appendData(sizeof(_databaseID), reinterpret_cast<const char*>(&_databaseID));
	stream.appendData(sizeof(_accountCount), reinterpret_cast<const char*>(&_accountCount));
	stream.appendData(sizeof(_accountCapacity), reinterpret_cast<const char*>(&_accountCapacity));

	for (int index = 0; index < _accountCount; ++index) {
		stream.appendData(sizeof(AccountIDType), reinterpret_cast<const char*>(&_accountIDs[index]));
		stream.appendData(sizeof(PasswordSHAType), reinterpret_cast<const char*>(&_passwordSHAs[index]));
		stream.appendData(sizeof(AccountDescriptionType), reinterpret_cast<const char*>(&_accountDescriptions[index]));
	}

	char emptyAccount[sizeof(AccountIDType) + sizeof(PasswordSHAType) + sizeof(AccountDescriptionType)] = { 0 };
	for (int index = 0; index < _accountCapacity - _accountCount; ++index) {
		stream.appendData(sizeof(emptyAccount), reinterpret_cast<const char*>(&emptyAccount));
	}
	return stream;
}

// same id: allowed; same desc: disallowed.
CrystalSynopsisDatabase::Status CrystalSynopsisDatabase::addAccount(const QString& name, const QString& password) {
	if (_accountCount >= _accountCapacity) {
		return Status::InsufficientCapacity;
	}

	AccountDescriptionType desc;
	Status writeDescStatus = WriteDescription(name, &desc);
	if (writeDescStatus != Status::Succeeded) {
		return writeDescStatus;
	}

	for (const auto& elem : _accountDescriptions) {
		if (desc._data == elem._data) {
			return Status::DescriptionExists;
		}
	}

	uint64_t id = RandomID();
	_accountIDs.push_back(id);
	_passwordSHAs.push_back(encryptPassword(id, password));
	_accountDescriptions.push_back(desc);
	++_accountCount;
	return Status::Succeeded;
}

CrystalSynopsisDatabase::Status CrystalSynopsisDatabase::delAccount(const QString& name, const QString& password) {
	Status verificationStatus = verification(name, password);
	if (verificationStatus != Status::Succeeded) {
		return verificationStatus;
	}

	AccountDescriptionType desc;
	Status writeDescStatus = WriteDescription(name, &desc);
	if (writeDescStatus != Status::Succeeded) {
		return writeDescStatus;
	}

	for (int index = 0; index < _accountCount; ++index) {
		if (desc._data == _accountDescriptions[index]._data) {
			_accountIDs.erase(_accountIDs.begin() + index);
			_passwordSHAs.erase(_passwordSHAs.begin() + index);
			_accountDescriptions.erase(_accountDescriptions.begin() + index);
			--_accountCount;
			return Status::Succeeded;
		}
	}
	return Status::DescriptionNotFound;
}

CrystalSynopsisDatabase::Status CrystalSynopsisDatabase::verification(const QString& name, const QString& password) const {
	AccountDescriptionType desc;
	Status writeDescStatus = WriteDescription(name, &desc);
	if (writeDescStatus != Status::Succeeded) {
		return writeDescStatus;
	}

	for (int index = 0; index < _accountCount; ++index) {
		if (_accountDescriptions[index]._data == desc._data) {
			if (encryptPassword(_accountIDs[index], password)._data == _passwordSHAs[index]._data) {
				return Status::Succeeded;
			}
			else {
				return Status::VerificationFailed;
			}
		}
	}
	return Status::DescriptionNotFound;
}

uint64_t CrystalSynopsisDatabase::RandomID() {
	static std::mt19937_64 mt(time(0));
	static std::uniform_int_distribution<uint64_t> distributer;
	return distributer(mt);
}

CrystalSynopsisDatabase::Status CrystalSynopsisDatabase::WriteDescription(const QString& srcString, AccountDescriptionType* destDesc) {
	if (srcString.length() > sizeof(AccountDescriptionType)) {
		return Status::DescriptionOverflow;
	}
	memcpy(reinterpret_cast<void*>(&destDesc), srcString.toUtf8().data(), srcString.length());
	return Status::Succeeded;
}

CrystalSynopsisDatabase::PasswordSHAType CrystalSynopsisDatabase::encryptPassword(AccountIDType id, const QString& password) const {
	PasswordSHAType sha;
	QString code = password + QString::number(id) + QString::number(_databaseID);
	memcpy(reinterpret_cast<void*>(&sha._data),  CrypterType::hash(code.toLatin1(), CrypterType::Sha3_256).data(), sizeof(PasswordSHAType));
	return sha;
}

bool CrystalFileSystem::setCurrent(const CrystalSynopsisBase* current) {
	forEachSynopsis(
		[](const CrystalSynopsisBase* synopsis, CrystalFileSystem* cfs, const CrystalSynopsisBase* current) {
			if (synopsis == current) {
				cfs->_currentSynopsis = const_cast<CrystalSynopsisBase*>(synopsis);
				return ForEachCondition::Break;
			}
			return ForEachCondition::Continue;
		},
		this,
		current
	);
	return _currentSynopsis == current;
}

bool CrystalFileSystem::setCurrentToParent() {
	if (_currentSynopsis->parent()) {
		_currentSynopsis = const_cast<CrystalSynopsisBase*>(_currentSynopsis->parent());
		return true;
	}
	return false;
}

CrystalFileSystem::State CrystalFileSystem::saveAs(const QString& path) {
	if (_mode == Mode::Read) {
		// DEV: if path different, save buffer as _file(rem update)
		// DEV: if path same, do nothing because _filemapping already do.
		if (_file.get_name() == path) {
			return State::PathInaccessible;
		}
	}

	if (fopen_s(&_outFile, path.toUtf8().data(), "wb")) {
		return State::PathInaccessible;
	}

	// IMPORTANT TO DO
	updateSynopses();

	writeSynopses();
	writeThumbnails();
	writeData();
	
	fclose(_outFile);
	_outFile = nullptr;
	return State::Succeeded;
}

const CrystalSynopsisBase* CrystalFileSystem::find(const QString& path) const {
	const CrystalSynopsisBase* current = _root.get();
	auto names = PathLayeredNames(path);
	int index = 0;

	if (current->name() != names[index]) {
		return nullptr;
	}

	auto findLambda = [](const CrystalSynopsisBase* synopsis, const QString& name, const CrystalSynopsisBase** outSynopsis) {
		if (name == synopsis->name()) {
			*outSynopsis = synopsis;
			return ForEachCondition::Break;
		}
		return ForEachCondition::Continue;
	};

	while (index < names.size() && current) {
		current->forEachChild(findLambda, names[index], &current);
		++index;
	}

	return current;
}

CrystalFileSystem::State CrystalFileSystem::move(const QString& srcPath, const QString& destDir) {
	CrystalSynopsisBase* srcSynopsis = const_cast<CrystalSynopsisBase*>(find(srcPath));
	CrystalSynopsisBase* destSynopsis = const_cast<CrystalSynopsisBase*>(find(destDir));
	return move(srcSynopsis, destSynopsis);
}

CrystalFileSystem::State CrystalFileSystem::move(const CrystalSynopsisBase* srcSynopsis, CrystalSynopsisBase* destSynopsis) {
	if (!srcSynopsis || !destSynopsis) {
		return State::SynopsisNotExists;
	}
	const_cast<CrystalSynopsisBase*>(srcSynopsis->parent())->moveChild(destSynopsis, srcSynopsis);
	return State::Succeeded;
}

const char* CrystalFileSystem::data(const CrystalSynopsisFile* synopsis) const {
	modeVerification(Mode::Read);

	return mappedData(synopsis->dataOffset());
}

const char* CrystalFileSystem::currentData() const {
	modeVerification(Mode::Read);

	if (!isFile(_currentSynopsis)) {
		return nullptr;
	}
	return mappedData(reinterpret_cast<CrystalSynopsisFile*>(_currentSynopsis)->dataOffset());
}

CrystalFileSystem::State CrystalFileSystem::exportCurrent(const QString& destDir) const {
	modeVerification(Mode::Read);

	return exportFileImplement(_currentSynopsis, this, destDir);
}

CrystalFileSystem::State CrystalFileSystem::exportFile(const CrystalSynopsisBase* synopsis, const QString& destDir) const {
	modeVerification(Mode::Read);

	return exportFileImplement(synopsis, this, destDir);
}

CrystalFileSystem::State CrystalFileSystem::exportDirectoryFiles(const CrystalSynopsisBase* synopsis, const QString& destDir) const {
	modeVerification(Mode::Read);

	// Do this because of foreach can not return a state.
	if (!std::filesystem::exists(destDir.toUtf8().data())) {
		return State::PathNotExists;
	}

	forEachSynopsisFrom(synopsis, exportFileImplement, this, destDir);
	return State::Succeeded;
}

CrystalFileSystem::State CrystalFileSystem::exportAllFiles(const QString& destDir) const {
	modeVerification(Mode::Read);

	// Do this because of foreach can not return a state.
	if (!std::filesystem::exists(destDir.toUtf8().data())) {
		return State::PathNotExists;
	}

	forEachSynopsis(exportFileImplement, this, destDir);
	return State::Succeeded;
}

QString CrystalFileSystem::path() const {
	modeVerification(Mode::Read);

	return _file.get_name();
}


const CrystalSynopsisBase* CrystalFileSystem::child(const CrystalSynopsisBase* synopsis, int index) const {
	if (!isFolder(synopsis)) {
		return nullptr;
	}
	auto* synopsisFolder = reinterpret_cast<const CrystalSynopsisFolder *>(synopsis);
	const CrystalSynopsisBase* outSynopsis = nullptr;
	int currentIndex = 0;
	synopsisFolder->forEachChild(
		[](const CrystalSynopsisBase* synopsis, const CrystalSynopsisBase** outSynopsis, int* currentIndex, int index) {
			if (*currentIndex == index) {
				*outSynopsis = synopsis;
				return ForEachCondition::Break;
			}
			*currentIndex += 1;
			return ForEachCondition::Continue;
		}, 
		&outSynopsis, 
		&currentIndex, 
		index
	);

	return outSynopsis;
}

int CrystalFileSystem::childCount(const CrystalSynopsisBase* synopsis) const {
	if (!synopsis || !isFolder(synopsis)) {
		return -1;
	}
	return reinterpret_cast<CrystalSynopsisFolder*>(const_cast<CrystalSynopsisBase*>(synopsis))->childCount();
}

SynopsisType CrystalFileSystem::type(const CrystalSynopsisBase* synopsis) const {
	return static_cast<SynopsisType>(synopsis->typeID());
}

QString CrystalFileSystem::typeName(const CrystalSynopsisBase* synopsis) const {
	if (!synopsis) {
		return "";
	}
	switch (static_cast<SynopsisType>(synopsis->typeID())) {
	case SynopsisType::Base:
		return "Base";
	case SynopsisType::Reference:
		return "Reference";
	case SynopsisType::Folder:
		return "Folder";
	case SynopsisType::File:
		return "File";
	case SynopsisType::MetaBase:
		return "MetaBase";
	case SynopsisType::MetaReference:
		return "MetaReference";
	case SynopsisType::MetaFolder:
		return "MetaFolder";
	case SynopsisType::MetaFile:
		return "MetaFile";
	default:
		return "Unknown";
	}
}

CrystalFileSystem::Mode CrystalFileSystem::mode() const {
	return _mode;
}

const CrystalSynopsisBase* CrystalFileSystem::root() const {
	return _root.get();
}

const CrystalSynopsisBase* CrystalFileSystem::current() const {
	return _currentSynopsis;
}

const CrystalSynopsisFolder* CrystalFileSystem::currentFolder() const {
	const auto* synopsis = _currentSynopsis;
	while (!isFolder(synopsis)) {
		synopsis = synopsis->parent();
	}
	return reinterpret_cast<const CrystalSynopsisFolder*>(synopsis);
}

bool CrystalFileSystem::isValid(const CrystalSynopsisBase* synopsis) const {
	bool valid = false;
	forEachSynopsis(
		[](const CrystalSynopsisBase* synopsis, const CrystalSynopsisBase* inSynopsis, bool* valid) {
			if (synopsis == inSynopsis) {
				*valid = true;
				return ForEachCondition::Break;
			}
			return ForEachCondition::Continue;
		}, 
		synopsis, 
		&valid
	);
	return valid;
}
