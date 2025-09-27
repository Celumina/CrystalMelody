#pragma once

namespace msg {
	using Text = const char*;

	static Text unknownFileType = "Unknown File Type";
	static Text failedOpenPackage = "Failed to Open Package";
	static Text noFolderOpened = "No Folder was Opened";
	static Text noPackageOpened = "No Package was Opened";
	static Text packagedSuccessfully = "Packed Successfully";
	static Text unpackagedSuccessfully = "Unpacked Successfully";

	static Text outputDirectory = "Output Directory";

	static Text thumbnailEmpty = "Thumbnail Image not Found";
	static Text accessError = "Access Permission Error";
	static Text saveSuccessfully = "Save Successfully";
	static Text saveThumbnailAs = "Save Thumbnail Image As";


	static Text explorerName = "Name";
	static Text explorerType = "Type";
	static Text explorerSize = "Size";


	static Text sizeUnits[9] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB" };
}
