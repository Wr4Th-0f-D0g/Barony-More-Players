/*-------------------------------------------------------------------------------

	BARONY
	File: files.hpp
	Desc: prototypes for file.cpp, all file access should be mediated
		  through this interface

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/
#pragma once

#include <list>
#include <string>
#include <vector>
#include <cstdio>
#include <dirent.h>

#include "main.hpp"

//This class provides a common platform-independent interface for file accesses. Deriving classes must provide an implementation for all of these methods, but may make use of any common routines or common portions of routines.
//Don't create a FileBase or derivative class directly, use FileIO::open to get one...
class FileBase {
	friend class FileIO;

public:
	// write data to the file
	// @param src where the data is coming from
	// @param size the size of each data element in bytes
	// @param count the number of data elements to write
	// @return the number of bytes written
	// The base class only contains the common implementation between FileNX and FilePC (currently just input validation), not a default implementation, so deriving implementation is mandatory.
	virtual size_t write(const void* src, size_t size, size_t count) = 0;

	// read data into the given buffer
	// @param buffer the buffer to read into
	// @param size the size of each data element in bytes
	// @param count the number of data elements to read
	// @return the number of bytes read
	// The base class only contains the common implementation between FileNX and FilePC (currently just input validation), not a default implementation, so deriving implementation is mandatory.
	virtual size_t read(void* buffer, size_t size, size_t count) = 0;

	// get the size of the file
	// @return the size in bytes
	virtual size_t size() = 0;

	// determine whether we have reached the end of the file or not
	// @return true if we are at the end of the file, otherwise false
	virtual bool eof() = 0;

	// read a string from the file, culling newlines
	// @param buf the buffer to contain the read string
	// @param size the maximum size of the string to read
	// @return buf if successfully read a string, otherwise nullptr
	char* gets2(char* buf, int size)
	{
		auto result = gets(buf, size);
		for (int c = 0; c < size; ++c)
		{
			if (buf[c] == '\n' || buf[c] == '\r')
			{
				buf[c] = '\0';
				return result;
			}
		}
		return result;
	}

	// read a string from the file
	// @param buf the buffer to contain the read string
	// @param size the maximum size of the string to read
	// @return buf if successfully read a string, otherwise nullptr
	char* gets(char* buf, int size)
	{
		char* result = buf;
	    if (!buf) {
		    return nullptr;
	    }
		for (int c = 0; c < size - 1; ++c) {
			size_t bytesRead = read(buf, sizeof(char), 1);
			if (bytesRead > 0U) {
				if (*buf == '\0' || *buf == '\n') {
					buf += bytesRead;
					break;
				}
				buf += bytesRead;
			} else {
				*buf = '\0';
				if (c == 0) {
					return nullptr;
				} else {
					return result;
				}
			}
		}
		*(buf) = '\0';
		return result;
	}

	// read an integer from the stream
	// @return the read integer or possibly 0 if we failed to read one
	int geti()
	{
		char field[64];
		gets(field, 64);
		long result = strtol(field, nullptr, 10);
		return (int)result;
	}

	// read 1 char from the stream
	// @return the read char or possibly '\0' if we failed to read one
	char getc()
	{
		char result = '\0';
		if (read(&result, sizeof(char), 1) != 1)
		{
			return '\0';
		}
		return result;
	}

	// write a formatted string to the file, printf style
	// @param fmt the string to format
	// @param ... variadic string format arguments
	// @return the number of characters written to the file
	int printf(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		char buf[1024];
		int result = vsnprintf(buf, 1024, fmt, args);
		buf[1023] = '\0';

		write(buf, sizeof(char), result);

		va_end(args);

		return result;
	}

	// write the given string to the file
	// @param str the string to write
	// @return 0 on success, -1 on error
	int puts(const char* str)
	{
		size_t size = strlen(str);
		return write(str, sizeof(char), size) == size ? 0 : -1;
	}

	// write char to file
	// @param c the char to write
	// @return 0 on success, -1 on error
	int putc(char c)
	{
		return write(&c, sizeof(char), 1) == 1 ? 0 : -1;
	}

	// seek mode associated with seek()
	enum class SeekMode : Uint8
	{
		SET,		// set the offset into the stream
		ADD,		// change the offset based on the current offset
		SETEND		// set the offset into the stream, starting from the end of the file
	};

	// moves the current offset in the file stream without reading or writing anything
	// @param offset how much to move the stream, or where
	// @param mode The seek mode
	// @return 0 on success, non-zero on error
	virtual int seek(ptrdiff_t offset, SeekMode mode) = 0;

	// get the current offset into the stream
	// @return the offset into the stream, in bytes
	virtual long int tell() = 0;

	// sets the file position back to the start of the file.
	void rewind()
	{
		seek(0, FileBase::SeekMode::SET);
	}

	// file mode
	enum class FileMode : Uint8
	{
		INVALID,	// not valid
		READ,		// file set for read mode
		WRITE		// file set for write mode
	};

protected:
	FileMode mode = FileMode::INVALID;
	std::string path;

	FileBase(FileMode mode, const char* path) :
		mode(mode),
		path(path)
	{
	}
	virtual ~FileBase() { }

private:
	// close the file, after this point no ops are valid
	virtual void close() = 0;
};

#ifdef NINTENDO
 #include "nintendo/filenx.hpp"
 typedef FileNX File;
#else
 #include "engine/filepc.hpp"
 typedef FilePC File;
#endif

//Don't create a FileBase or derivative class directly, use this to get one...
class FileIO {
private:
	FileIO() {}
	~FileIO() {}

public:
	// open a new file
	// @param path complete path to the file to open
	// @param mode access mode (see fopen in stdio)
	static File* open(const char* path, const char* mode)
	{
		if (!path || !mode)
		{
			return nullptr;
		}

		FileBase::FileMode fileMode;
		switch (mode[0])
		{
		case 'r': fileMode = FileBase::FileMode::READ; break;
		case 'w': fileMode = FileBase::FileMode::WRITE; break;
		default: fileMode = FileBase::FileMode::INVALID; break;
		}

#ifdef NINTENDO
		return FileNX::FileIO_NintendoOpen(path, mode, fileMode);
#else
        // note: on PC, files are ALWAYS opened in binary mode
		FILE* fp;
		switch (fileMode) {
		default: assert(0 && "invalid file open mode");
		case FileBase::FileMode::READ: fp = fopen(path, "rb"); break;
		case FileBase::FileMode::WRITE: fp = fopen(path, "wb"); break;
		}
		if (fp) {
			return new File(fp, fileMode, path);
		} else {
			return nullptr;
		}
#endif
	}

	// close the given file
	// @param file the file to close
	static void close(File* file)
	{
		if (!file)
		{
			return;
		}
		file->close();
		delete file;
	}
};

enum HolidayTheme {
    THEME_NONE,
    THEME_HALLOWEEN,
    THEME_XMAS,
    THEME_MAX
};
extern const char* holidayThemeDirs[HolidayTheme::THEME_MAX];
HolidayTheme getCurrentHoliday(bool force = false);
bool isCurrentHoliday(bool force = false);

#ifndef EDITOR
#include "interface/consolecommand.hpp"
extern ConsoleVariable<int> cvar_forceHoliday;
extern ConsoleVariable<bool> cvar_disableHoliday;
#endif

extern char datadir[PATH_MAX]; //PATH_MAX as defined in main.hpp -- maybe define in Config.hpp?
extern char outputdir[PATH_MAX];
void glLoadTexture(SDL_Surface* image, int texnum);
SDL_Surface* loadImage(char const * const filename);
voxel_t* loadVoxel(char* filename2);
bool verifyMapHash(const char* filename, int hash, bool* fileExistsInTable = nullptr);
int loadMap(const char* filename, map_t* destmap, list_t* entlist, list_t* creatureList, int *checkMapHash = nullptr);
int loadConfig(char* filename);
int loadDefaultConfig();
int saveMap(const char* filename);
char* readFile(char* filename);
std::list<std::string> directoryContents(const char* directory, bool includeSubdirectory, bool includeFiles, const char* base = datadir);
File *openDataFile(const char *const filename, const char * const mode);
DIR * openDataDir(const char *const);
bool dataPathExists(const char *const, bool complete = true);
bool completePath(char *dest, const char * const path, const char *base = datadir);
void openLogFile();
std::vector<std::string> getLinesFromDataFile(std::string filename);
int loadMainMenuMap(bool blessedAdditionMaps, bool forceVictoryMap, int forcemap = -1);
int physfsLoadMapFile(int levelToLoad, Uint32 seed, bool useRandSeed, int *checkMapHash = nullptr);
std::list<std::string> physfsGetFileNamesInDirectory(const char* dir);
std::string physfsFormatMapName(char const * const levelfilename);
bool physfsModelIndexUpdate(int &start, int &end);
bool physfsSearchModelsToUpdate();
bool physfsSearchSoundsToUpdate();
void physfsReloadSounds(bool reloadAll);
void physfsReloadBooks();
bool physfsSearchBooksToUpdate();
bool physfsSearchMusicToUpdate();
void physfsReloadMusic(bool &introMusicChanged, bool reloadAll);
void physfsReloadTiles(bool reloadAll);
bool physfsSearchTilesToUpdate();
void physfsReloadSprites(bool reloadAll);
bool physfsSearchSpritesToUpdate();
bool physfsIsMapLevelListModded();
bool physfsSearchItemSpritesToUpdate();
void physfsReloadItemSprites(bool reloadAll);
bool physfsSearchItemsTxtToUpdate();
bool physfsSearchItemsGlobalTxtToUpdate();
void physfsReloadItemsTxt();
bool physfsSearchMonsterLimbFilesToUpdate();
void physfsReloadMonsterLimbFiles();
void physfsReloadSystemImages();
bool physfsSearchSystemImagesToUpdate();
void gamemodsUnloadCustomThemeMusic();

enum MapParameterIndices : int
{
	LEVELPARAM_CHANCE_SECRET,
	LEVELPARAM_CHANCE_DARKNESS,
	LEVELPARAM_CHANCE_MINOTAUR,
	LEVELPARAM_DISABLE_NORMAL_EXIT
};
