#pragma once
#pragma once
#include <memory>
#include <QFile>
#include <QFileInfo>

// duplicated soon...


class CrystalPackage {
public:
	struct IndexData;
	using DataSizeType = uint64_t;
	using IndexSizeType = uint16_t;
	using IndexesType = std::vector<IndexData>;
	using DirsType = QFileInfoList;

	enum class exception {
		outDirError,
		inDirError,
		pathAccessError,
		packagePathAccessError
	};

	enum limit : DataSizeType {
		bufferSizeMax = 131072	// memory usage when write buffer: 128 KB
	};

	enum flag : IndexSizeType {
		dirFlag = 0b1000000000000000 // hightest bit as directory flag
	};

	class Index {
		friend class CrystalPackage;
	public:
		explicit Index(const CrystalPackage* packagePtr, IndexSizeType indexOffset);
		QString name() const;
		QString dir() const;
		QString path() const;	// inline �������ܵ��� inline ����, ��˴˴� path() ����Ϊ inline
		DataSizeType offset() const;
		DataSizeType size() const;
		bool empty() const noexcept;
		Index& operator++() noexcept;
		Index& operator--() noexcept;

	private:
		const CrystalPackage* _thisPackage;
		IndexSizeType _indexOffset;
	};

	// inline must be inClass use! (like private mfunc)
	class Stream {
	public:
		explicit Stream(DataSizeType dataSize);
		explicit Stream(Index index);
		~Stream();

		void setData(QFile& src, DataSizeType writeSize);
		const char* data();
		DataSizeType size();
		QString toString();

		// devlog: ����ʹ��Ĭ���ƶ�����

		Stream(const Stream&) = delete;
		const Stream& operator=(const Stream&) = delete;

	private:
		char* _data;
		DataSizeType _size;
	};

	static void pack(const QString& inDir, const QString& outDir);

	explicit CrystalPackage(const QString& packagePath);
	~CrystalPackage();

	void unpack(const QString& outDir);
	void unpack(const QString& outDir, const Index& index);

	// Index interface
	Index firstIndex();
	Index lastIndex();
	Index findName(const QString& filename, const Index& offsetIndex);
	Index findName(const QString& filename);
	Index findPath(const QString& path);

	// Member attributes
	qsizetype indexLength() const;

	CrystalPackage(const CrystalPackage&) = delete;
	const CrystalPackage& operator=(const CrystalPackage&) = delete;

private:
	struct Pimpl;
	struct IndexData;

	static void loadFilePaths(const QString& inDir, DirsType& container);
	static QString formatDirectory(const QString& dir);
	inline static void writeFile(const QString& src, QFile& dest);
	inline static void writeData(QFile& src, QFile& dest, DataSizeType size);

	inline void unpackFile(const QString& outDir, const Index& index);

	std::unique_ptr<Pimpl> data;
};