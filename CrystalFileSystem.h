/*
Create by Celuna in Spring of 2024.
QString is used for resolve encode problem in different device.
*/

// TODO: CFS functions return a handle use implcit cast of operator Type();

#pragma once
#include <vector>
#include <list>
#include <type_traits>

#include <QString>
#include <QCryptographicHash>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>


// Binrary for Inheritance relationship mark.
enum class SynopsisType : uint8_t {
	Base =						0b0000'0000,
	Reference =				0b0000'0001,
	Folder =						0b0000'0010,
	File =							0b0000'0011,
	MetaBase =				0b0000'0100, // Useless
	MetaReference =	0b0000'0101,
	MetaFolder =			0b0000'0110,
	MetaFile =					0b0000'0111
};

enum class ForEachCondition {
	Continue,
	Break
};

class CrystalSynopsisStream {
public:
	using SizeType = uint32_t;
	CrystalSynopsisStream(SizeType size);
	CrystalSynopsisStream(CrystalSynopsisStream&& rval) noexcept;
	~CrystalSynopsisStream();

	// append success return true
	bool appendData(SizeType dataSize, const char* data);
	bool appendData(const CrystalSynopsisStream& stream);
	uint32_t size() const;
	const char* data() const;

	CrystalSynopsisStream(const CrystalSynopsisStream&) = delete;
	const CrystalSynopsisStream& operator=(const CrystalSynopsisStream&) = delete;

private:
	uint32_t _offset;
	uint32_t _size;
	char* _data;
};

// DEV: use 1 bit sign the syno encoded.
struct CrystalSynopsisHeader {
	char _magic[4] = { 0 };
	uint16_t _version = 0;
	uint16_t _noUsed = 0;
};

class CrystalSynopsisDatabase {
public:
	enum class Status {
		InsufficientCapacity,
		DescriptionOverflow,
		DescriptionNotFound,
		DescriptionExists,
		VerificationFailed,
		Succeeded
	};
	using CrypterType = QCryptographicHash;
	using AccountIDType = uint64_t;
	struct PasswordSHAType {
		char _data[32] = { 0 };
	};

	struct AccountDescriptionType {
		char _data[64] = { 0 };
	};

	CrystalSynopsisDatabase();
	CrystalSynopsisDatabase(uint32_t capacity);

	void load(const char* address);
	uint32_t byteSize() const;
	CrystalSynopsisStream dump() const;

	Status addAccount(const QString& name, const QString& password);
	Status delAccount(const QString& name, const QString& password);

	Status verification(const QString& name, const QString& password) const;

	static uint64_t RandomID();
	static Status WriteDescription(const QString& srcString, AccountDescriptionType* destDesc);

private:
	PasswordSHAType encryptPassword(AccountIDType id, const QString& password) const;

	uint64_t _databaseID;
	uint32_t _accountCount;
	uint32_t _accountCapacity;
	std::vector<AccountIDType> _accountIDs;
	std::vector<PasswordSHAType> _passwordSHAs;
	std::vector<AccountDescriptionType> _accountDescriptions;
};

class CrystalSynopsisMetaInfo {
public:
	// DEV: check bytesize, add to 256
	enum class Security : int8_t {
		System = -1, 
		Public = 0, 
		SynopsisPublic = 1, 
		Verification = 2, 
		Priviate = 3, 
		PrivateVerification = 4, 
		HideVerification = 5
	};

	// DEV: expand...
	enum  class MetaType : uint16_t {
		Unknown = 0
	};

	void load(const char* address);
	uint32_t byteSize() const;
	CrystalSynopsisStream dump() const;


	uint64_t thumbnailOffset() const;
	uint32_t thumbnailSize() const;

	uint64_t createdTime() const;
	uint64_t lastAccessTime() const;

	MetaType metaType() const;
	uint16_t version() const;

	Security securityLevel() const;

	const char* fileMD5() const;
	const char* accountMD5() const;
	const char* description() const;

	uint32_t descriptionSize() const;

	const QString& mappedThumbnailPath() const;


	void setThumbnailOffset(uint64_t thumbnailOffset);
	void setThumbnailSize(uint32_t thumbnailSize);

	void setCreatedTime(uint64_t createdTime);
	void setLastAccessTime(uint64_t lastAccessTime);

	void setMetaType(MetaType metaType);
	void setVersion(uint16_t version);

	void setSecurityLevel(Security securityLevel);

	void updateFileMD5();
	void setAccountMD5(const CrystalSynopsisDatabase& database, const QString& accountName);

	// DEV: split its func, rename, desc, tag, style...
	void setDescription(const char* description);

	void setMappedThumbnailPath(const QString& mappedThumbnailPath);

	QString toString() const;

private:
	uint64_t _thumbnailOffset = 0;
	uint32_t _thumbnailSize = 0;

	// offset second since 1970
	// manage in file system
	int64_t _createdTime = 0;
	int64_t _lastAccessTime = 0;

	MetaType _metaType = MetaType::Unknown;
	uint16_t _version = 0;

	Security _securityLevel = Security::Public;

	char _fileMD5[16] = {0};
	// encrypt by account, created time and database info
	char _accountMD5[16] = {0};

	// for dynamic modify...rename, display style, description, tag...
	// DEV: change as a class with some user define interface
	char _description[184] = {0};

	char _noUsed[3] = { 0 };

	QString _mappedthumbnailPath;
};

class CrystalSynopsisBase {
public:
	CrystalSynopsisBase(const CrystalSynopsisBase&) = delete;
	const CrystalSynopsisBase& operator=(const CrystalSynopsisBase&) = delete;


	virtual uint8_t typeID() const;
	virtual void load(const char* address);
	virtual uint32_t byteSize() const;
	virtual CrystalSynopsisStream dump() const;

	// not ret a rval beacuse of a temp val would cause string operator ERROR
	QString path() const;

	uint16_t nameLength() const;
	const QString& name() const;
	const QString& mappedPath() const;

	void setName(const QString& name);
	void setMappedPath(const QString& mappedPath);

	// only manage as parent things.
	const CrystalSynopsisBase* findChild(const QString& name) const;
	const CrystalSynopsisBase* findChild(uint32_t index) const;


	// input must be a new synopsis without parent, sibling and child
	// and this func will release input synopsis
	bool addChild(std::unique_ptr<CrystalSynopsisBase>&& synopsis);

	bool delChild(const CrystalSynopsisBase* child);
	void delAllChildren();

	bool moveChild(CrystalSynopsisBase* destSynopsis, const CrystalSynopsisBase* child);
	void moveAllChildren(CrystalSynopsisBase* destSynopsis);

	// if has sibling's sibling belong to the node, replace old and relink sibling's sibling

	const CrystalSynopsisBase* parent() const;
	const CrystalSynopsisBase* firstChild() const;
	const CrystalSynopsisBase* sibling() const;
	const CrystalSynopsisBase* lastChild() const;

	CrystalSynopsisBase* firstChild();
	CrystalSynopsisBase* lastChild();

	const CrystalSynopsisBase* findChild(const QString& name, const CrystalSynopsisBase* begin = nullptr) const;
	CrystalSynopsisBase* findChild(const QString& name, const CrystalSynopsisBase* begin = nullptr);

	virtual QString toString() const;

	template<typename FuncType, typename ...ParamsTypes>
	void forEachChild(FuncType callback, ParamsTypes... params) const;



protected:
	CrystalSynopsisBase();

	// For serialization
	uint16_t _nameLength;
	QString _name;

	// For data process
	// node manage firstChild and smaller sibling memory
	const CrystalSynopsisBase* _parent;
	std::unique_ptr<CrystalSynopsisBase> _firstChild;
	std::unique_ptr<CrystalSynopsisBase> _sibling;

	// DEV: add load from path func
	// for runtime package create
	QString _mappedPath;
};

class CrystalSynopsisReference : public CrystalSynopsisBase {
public:
	CrystalSynopsisReference();

	virtual uint8_t typeID() const override;
	virtual QString toString() const override;
};

class CrystalSynopsisFolder : public CrystalSynopsisBase {
public:
	CrystalSynopsisFolder();

	virtual uint8_t typeID() const override;
	virtual void load(const char* address) override;
	virtual uint32_t byteSize() const override;
	virtual CrystalSynopsisStream dump() const override;

	// this func will update child count automatic
	uint32_t childCount();

	// for data load
	uint32_t childCountRaw() const;

	virtual QString toString() const override;

protected:
	// do it in dump() and childCount()
	void updateChildCount() const;

	mutable uint32_t _childCount;
};

class CrystalSynopsisFile : public CrystalSynopsisBase {
public:
	CrystalSynopsisFile();

	virtual uint8_t typeID() const override;
	virtual void load(const char* address) override;
	virtual uint32_t byteSize() const override;
	virtual CrystalSynopsisStream dump() const override;

	uint64_t dataOffset() const;
	uint64_t dataSize() const;

	void setDataOffset(uint64_t dataOffset);
	void setDataSize(uint64_t dataSize);

	virtual QString toString() const override;

protected:
	// absolute offset in .rys, beginfrom 0
	uint64_t _dataOffset;
	uint64_t _dataSize;
};

class CrystalSynopsisMetaFolder : public CrystalSynopsisFolder {
public:
	CrystalSynopsisMetaFolder();

	virtual uint8_t typeID() const override;
	virtual void load(const char* address) override;
	virtual uint32_t byteSize() const override;
	virtual CrystalSynopsisStream dump() const override;

	const CrystalSynopsisMetaInfo& metaInfo() const;

	CrystalSynopsisMetaInfo& metaInfo();
	void setMetaInfo(const CrystalSynopsisMetaInfo& metaInfo);

	virtual QString toString() const override;

protected:
	CrystalSynopsisMetaInfo _metaInfo;
};

class CrystalSynopsisMetaFile : public CrystalSynopsisFile {
public:
	CrystalSynopsisMetaFile();

	virtual uint8_t typeID() const override;
	virtual void load(const char* address) override;
	virtual uint32_t byteSize() const override;
	virtual CrystalSynopsisStream dump() const override;

	const CrystalSynopsisMetaInfo& metaInfo() const;

	CrystalSynopsisMetaInfo& metaInfo();
	void setMetaInfo(const CrystalSynopsisMetaInfo& metaInfo);

	virtual QString toString() const override;

protected:
	CrystalSynopsisMetaInfo _metaInfo;
	// DEV: no needed, accout and data encrypt by created time
	uint64_t _encryptionCode;
};


class CrystalFileSystem {
public:
	enum class Mode {
		Null, 
		Write, 
		WriteDatabase, 
		Read // in the mode you can move synos but can't add/del
	};

	enum class State {
		Succeeded, 
		ModeNotSupported, 
		PathNotExists, 
		PathInaccessible, 
		SynopsisTypeCannotHoldsChild, 
		SynopsisNotExists, 
	};

	CrystalFileSystem(CrystalFileSystem&& rval) noexcept;
	~CrystalFileSystem();

	CrystalFileSystem(const CrystalFileSystem&) = delete;
	const CrystalFileSystem& operator=(const CrystalFileSystem&) = delete;

	// LNA, Layered Node Archive
	static CrystalFileSystem Create(SynopsisType rootType, const QString& rootName = "NewRoot");
	// CEL, Credential Encryption Library
	static CrystalFileSystem CreateDatabase();
	static CrystalFileSystem Open(const QString& path);

	static std::vector<QString> PathLayeredNames(const QString& path);

	void reset(CrystalFileSystem&& rval);


	//////////////////////////////////////////////////
	// Write mode only
	// this func can append a folder also.
	State appendFile(const QString& path);
	State appendMetaFile(const QString& path, const CrystalSynopsisMetaInfo& metaInfo);
	State appendReference(const QString& path);
	State appendAllFiles(const QString& dir);

	// append file and keep it soruce path structure, if struct not in current, it will create syno automaic.
	// throw disk flag like C:  
	State appendLayeredFile(const QString& filePath, const QString& relativePath);

	// any create func will not change the position of _currentSynopsis
	const CrystalSynopsisFolder* createFolder(const QString& name);
	const CrystalSynopsisMetaFolder* createMetaFolder(const QString& name, const CrystalSynopsisMetaInfo& metaInfo);
	//////////////////////////////////////////////////


	//////////////////////////////////////////////////
	// open mode only
	const char* data(const CrystalSynopsisFile* synopsis) const;
	const char* currentData() const;

	State exportCurrent(const QString& destDir) const;
	State exportFile(const CrystalSynopsisBase* synopsis, const QString& destDir) const;
	State exportDirectoryFiles(const CrystalSynopsisBase* synopsis, const QString& destDir) const;
	State exportAllFiles(const QString& destDir) const;

	// if maxDepth == -1, find and export all reference with infinite depth. DANGEROUS!
	State exportAllFilesWithReference(const QString& destDir, int maxDepth = 1) const;

	QString path() const;
	//////////////////////////////////////////////////
	

	//////////////////////////////////////////////////
	// database only
	State accountRegister(const QString& name, const QString& password);
	//////////////////////////////////////////////////



	//////////////////////////////////////////////////
	// user interface
	// DEV: permission level 
	// DEV: user interface consider MetaFolder and MetaFile account permission..
	State accountLogin(const QString& name, const QString& password);
	State userExportAllFiles(const QString& destDir) const;
	//////////////////////////////////////////////////



	//////////////////////////////////////////////////
	// General and replace raw pointer access.
	// interface synopsis must be base, for easier call.
	const CrystalSynopsisBase* child(const CrystalSynopsisBase* synopsis, int index) const;
	int childCount(const CrystalSynopsisBase* synopsis) const ;
	SynopsisType type(const CrystalSynopsisBase* synopsis) const;
	QString typeName(const CrystalSynopsisBase* synopsis) const;

	static bool isFolder(const CrystalSynopsisBase* synopsis);
	static bool isFile(const CrystalSynopsisBase* synopsis);
	static bool isMeta(const CrystalSynopsisBase* synopsis);
	//////////////////////////////////////////////////



	//////////////////////////////////////////////////
	// General

	Mode mode() const;
	const CrystalSynopsisBase* root() const;
	// if not success, return nullptr
	const CrystalSynopsisBase* current() const;
	const CrystalSynopsisFolder* currentFolder() const;

	// :: Synopsis Access
	bool isValid(const CrystalSynopsisBase* synopsis) const;

	// if not success, current will not change. 
	bool setCurrent(const CrystalSynopsisBase* current);
	bool setCurrentToParent();

	State saveAs(const QString& path);

	const CrystalSynopsisBase* find(const QString& path) const;
	const CrystalSynopsisBase* find(const QString& name, const CrystalSynopsisBase* begin) const;

	// makes src as dest child
	State move(const QString& srcPath, const QString& destDir);
	State move(const CrystalSynopsisBase* srcSynopsis, CrystalSynopsisBase* destSynopsis);
	//////////////////////////////////////////////////
	


	// No necessary, header info is generated inside the class
	// void setHeader(char magicChars[4], uint16_t version);

private:
	CrystalFileSystem();

	// file mapping ptr
	const char* mappedData(uint64_t offset = 0) const;
	char* mappedData(uint64_t offset = 0);

	// input stream
	// return created syno, if create failed, return nullptr
	CrystalSynopsisBase* createSynopsisChild(SynopsisType type, CrystalSynopsisBase* destParent = nullptr);
	void loadSynopses(const char* address);

	void updateSynopses();
	void writeSynopses();
	void writeThumbnails();
	void writeData();

	// output
	static State exportFileImplement(const CrystalSynopsisBase* synopsis, const CrystalFileSystem* cfs, const QString& dir);

	// verification of function usage
	inline void modeVerification(Mode mode) const;


	// DEV: support BFS mode...
	// DEV: support return in loops in new foreach func
	template<typename FuncType, typename ...ParamsTypes>
	void forEachSynopsisFrom(const CrystalSynopsisBase* root, FuncType callback, ParamsTypes... params) const;

	template<typename FuncType, typename ...ParamsTypes>
	void forEachSynopsis(FuncType callback, ParamsTypes... params) const;

	Mode _mode;

	std::unique_ptr<CrystalSynopsisBase> _root;

	std::unique_ptr<CrystalSynopsisHeader> _header;
	std::unique_ptr<CrystalSynopsisDatabase> _database;

	FILE* _outFile;

	boost::interprocess::file_mapping _file;
	boost::interprocess::mapped_region _region;

	uint64_t _synopsesSize;
	uint64_t _thumbnailsSize;
	uint64_t _dataSize;

	CrystalSynopsisBase* _currentSynopsis;
	const char* _currentAccount;
};

template<typename FuncType, typename ...ParamsTypes>
inline void CrystalFileSystem::forEachSynopsisFrom(const CrystalSynopsisBase* synopsis, FuncType callback, ParamsTypes ...params) const {
	// Depth First Search
	std::list<const CrystalSynopsisBase*> midNodes;
	const CrystalSynopsisBase* current = synopsis;
	while (current) {
		// if callback func return true, it means break synopsis loop immediately.
		if constexpr (std::is_same_v<std::invoke_result_t<FuncType, CrystalSynopsisBase*, ParamsTypes...>, ForEachCondition>) {
			if (callback(current, params...) == ForEachCondition::Break) {
				break;
			};
		}
		else {
			callback(current, params...);
		}
		
		if (current->sibling()) {
			// if has sibling, record it.
			midNodes.push_back(current->sibling());
		}

		if (current->firstChild()) {
			current = current->firstChild();
		}
		else if (!midNodes.empty()) {
			current = midNodes.back();
			midNodes.pop_back();
		}
		else {
			current = nullptr;
		}
	}
}

template<typename FuncType, typename ...ParamsTypes>
inline void CrystalFileSystem::forEachSynopsis(FuncType callback, ParamsTypes... params) const {
	forEachSynopsisFrom(_root.get(), callback, params...);
}


template<typename FuncType, typename ...ParamsTypes>
inline void CrystalSynopsisBase::forEachChild(FuncType callback, ParamsTypes ...params) const {
	const CrystalSynopsisBase* current = _firstChild.get();
	while (current) {
		if constexpr (std::is_same_v<std::invoke_result_t<FuncType, CrystalSynopsisBase*, ParamsTypes...>, ForEachCondition>) {
			if (callback(current, params...) == ForEachCondition::Break) {
				break;
			};
		}
		else {
			callback(current, params...);
		}
		current = current->sibling();
	}
}
