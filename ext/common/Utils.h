/*
 *  Phusion Passenger - http://www.modrails.com/
 *  Copyright (c) 2008, 2009 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */
#ifndef _PASSENGER_UTILS_H_
#define _PASSENGER_UTILS_H_

#include <boost/shared_ptr.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <cstdio>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include "StaticString.h"
#include "Exceptions.h"

namespace Passenger {

using namespace std;
using namespace boost;

typedef struct CachedFileStat CachedFileStat;

/** Enumeration which indicates what kind of file a file is. */
typedef enum {
	/** The file doesn't exist. */
	FT_NONEXISTANT,
	/** A regular file or a symlink to a regular file. */
	FT_REGULAR,
	/** A directory. */
	FT_DIRECTORY,
	/** Something else, e.g. a pipe or a socket. */
	FT_OTHER
} FileType;

/**
 * Convenience shortcut for creating a <tt>shared_ptr</tt>.
 * Instead of:
 * @code
 *    shared_ptr<Foo> foo;
 *    ...
 *    foo = shared_ptr<Foo>(new Foo());
 * @endcode
 * one can write:
 * @code
 *    shared_ptr<Foo> foo;
 *    ...
 *    foo = ptr(new Foo());
 * @endcode
 *
 * @param pointer The item to put in the shared_ptr object.
 * @ingroup Support
 */
template<typename T> shared_ptr<T>
ptr(T *pointer) {
	return shared_ptr<T>(pointer);
}

/**
 * Used internally by toString(). Do not use directly.
 *
 * @internal
 */
template<typename T>
struct AnythingToString {
	string operator()(T something) {
		stringstream s;
		s << something;
		return s.str();
	}
};

/**
 * Used internally by toString(). Do not use directly.
 *
 * @internal
 */
template<>
struct AnythingToString< vector<string> > {
	string operator()(const vector<string> &v) {
		string result("[");
		vector<string>::const_iterator it;
		unsigned int i;
		for (it = v.begin(), i = 0; it != v.end(); it++, i++) {
			result.append("'");
			result.append(*it);
			if (i == v.size() - 1) {
				result.append("'");
			} else {
				result.append("', ");
			}
		}
		result.append("]");
		return result;
	}
};

/**
 * Convert anything to a string.
 *
 * @param something The thing to convert.
 * @ingroup Support
 */
template<typename T> string
toString(T something) {
	return AnythingToString<T>()(something);
}

/**
 * Converts the given string to an integer.
 * @ingroup Support
 */
int atoi(const string &s);

/**
 * Converts the given string to a long integer.
 * @ingroup Support
 */
long atol(const string &s);

/**
 * Split the given string using the given separator.
 *
 * @param str The string to split.
 * @param sep The separator to use.
 * @param output The vector to write the output to.
 * @ingroup Support
 */
void split(const string &str, char sep, vector<string> &output);

/**
 * Check whether the specified file exists.
 *
 * @param filename The filename to check.
 * @param cstat A CachedFileStat object, if you want to use cached statting.
 * @param throttleRate A throttle rate for cstat. Only applicable if cstat is not NULL.
 * @return Whether the file exists.
 * @throws FileSystemException Unable to check because of a filesystem error.
 * @throws TimeRetrievalException
 * @throws boost::thread_interrupted
 * @ingroup Support
 */
bool fileExists(const char *filename, CachedFileStat *cstat = 0,
                unsigned int throttleRate = 0);

/**
 * Check whether 'filename' exists and what kind of file it is.
 *
 * @param filename The filename to check.
 * @param mstat A CachedFileStat object, if you want to use cached statting.
 * @param throttleRate A throttle rate for cstat. Only applicable if cstat is not NULL.
 * @return The file type.
 * @throws FileSystemException Unable to check because of a filesystem error.
 * @throws TimeRetrievalException
 * @throws boost::thread_interrupted
 * @ingroup Support
 */
FileType getFileType(const StaticString &filename, CachedFileStat *cstat = 0,
                     unsigned int throttleRate = 0);

/**
 * Create the given file with the given contents, permissions and ownership.
 * This function does not leave behind junk files: if the ownership cannot be set
 * or if not all data can be written then then the file will be deleted.
 *
 * @param filename The file to create.
 * @param contents The contents to write to the file.
 * @param permissions The desired file permissions.
 * @param owner The desired file owner. Specify -1 if you want to use the current
 *              process's owner as the file owner.
 * @param group The desired file group. Specify -1 if you want to use the current
 *              process's group as the file group.
 * @throws FileSystemException Something went wrong.
 * @ingroup Support
 */
void createFile(const string &filename, const StaticString &contents,
                mode_t permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
                uid_t owner = (uid_t) -1, gid_t group = (gid_t) -1);

/**
 * Find the location of the Passenger spawn server script.
 *
 * @param passengerRoot The Passenger root folder. If NULL is given, then
 *      the spawn server is found by scanning $PATH. For security reasons,
 *      only absolute paths are scanned.
 * @return An absolute path to the spawn server script, or
 *         an empty string on error.
 * @throws FileSystemException Unable to access parts of the filesystem.
 * @ingroup Support
 */
string findSpawnServer(const char *passengerRoot = NULL);

/**
 * Returns a canonical version of the specified path. All symbolic links
 * and relative path elements are resolved.
 *
 * @throws FileSystemException Something went wrong.
 * @ingroup Support
 */
string canonicalizePath(const string &path);

/**
 * If <em>path</em> refers to a symlink, then this function resolves the
 * symlink for 1 level. That is, if the symlink points to another symlink,
 * then the other symlink will not be resolved. The resolved path is returned.
 *
 * If the symlink doesn't point to an absolute path, then this function will
 * prepend <em>path</em>'s directory to the result.
 *
 * If <em>path</em> doesn't refer to a symlink then this method will return
 * <em>path</em>.
 *
 * @throws FileSystemException Something went wrong.
 * @ingroup Support
 */
string resolveSymlink(const string &path);

/**
 * Given a path, extracts its directory name.
 *
 * @ingroup Support
 */
string extractDirName(const string &path);

/**
 * Escape the given raw string into an XML value.
 *
 * @throws std::bad_alloc Something went wrong.
 * @ingroup Support
 */
string escapeForXml(const string &input);

/**
 * Returns the username of the user that the current process is running as.
 * If the user has no associated username, then "UID xxxx" is returned,
 * where xxxx is the current UID.
 */
string getProcessUsername();

/**
 * Given a username that's supposed to be the "lowest user" in the user switching mechanism,
 * checks whether this username exists. If so, this users's UID and GID will be stored into
 * the arguments of the same names. If not, <em>uid</em> and <em>gid</em> will be set to
 * the UID and GID of the "nobody" user. If that user doesn't exist either, then <em>uid</em>
 * and <em>gid</em> will be set to -1.
 */
void determineLowestUserAndGroup(const string &user, uid_t &uid, gid_t &gid);

/**
 * Return the path name for the directory in which the system stores general
 * temporary files. This is usually "/tmp", but might be something else depending
 * on some environment variables.
 *
 * @ensure result != NULL
 * @ingroup Support
 */
const char *getSystemTempDir();

/* Create a temporary directory for storing Phusion Passenger instance-specific
 * temp files, such as temporarily buffered uploads, sockets for backend
 * processes, etc.
 * The directory that will be created is the one returned by
 * <tt>getPassengerTempDir(false, parentDir)</tt>. This call stores the path to
 * this temp directory in an internal variable, so that subsequent calls to
 * getPassengerTempDir() will return the same path.
 *
 * The created temp directory will have several subdirectories:
 * - webserver_private - for storing the web server's buffered uploads.
 * - info - for storing files that allow external tools to query information
 *          about a running Phusion Passenger instance.
 * - backends - for storing Unix sockets created by backend processes.
 * - master - for storing files such as the Passenger HelperServer socket.
 *
 * If a (sub)directory already exists, then it will not result in an error.
 *
 * The <em>userSwitching</em> and <em>lowestUser</em> arguments passed to
 * this method are used for determining the optimal permissions for the
 * (sub)directories. The permissions will be set as tightly as possible based
 * on the values. The <em>workerUid</em> and <em>workerGid</em> arguments
 * will be used for determining the owner of certain subdirectories.
 *
 * @note You should only call this method inside the web server's master
 *       process. In case of Apache, this is the Apache control process,
 *       the one that tends to run as root. This is because this function
 *       will set directory permissions and owners/groups, which may require
 *       root privileges.
 *
 * @param parentDir The directory under which the Phusion Passenger-specific
 *                  temp directory should be created. This argument may be the
 *                  empty string, in which case getSystemTempDir() will be used
 *                  as the parent directory.
 * @param userSwitching Whether user switching is turned on.
 * @param lowestUser The user that the spawn manager and the pool server will
 *                   run as, if user switching is turned off.
 * @param workerUid The UID that the web server's worker processes are running
 *                  as. On Apache, this is the UID that's associated with the
 *                  'User' directive.
 * @param workerGid The GID that the web server's worker processes are running
 *                  as. On Apache, this is the GID that's associated with the
 *                  'Group' directive.
 * @throws IOException Something went wrong.
 * @throws SystemException Something went wrong.
 * @throws FileSystemException Something went wrong.
 */
/* void createPassengerTempDir(const string &parentDir, bool userSwitching,
                            const string &lowestUser,
                            uid_t workerUid, gid_t workerGid); */

/**
 * Create the directory at the given path, creating intermediate directories
 * if necessary. The created directories' permissions are exactly as specified
 * by the 'mode' parameter (i.e. the umask will be ignored). You can specify
 * this directory's owner and group through the 'owner' and 'group' parameters.
 * A value of -1 for 'owner' or 'group' means that the owner/group should not
 * be changed.
 *
 * If 'path' already exists, then nothing will happen.
 *
 * @throws IOException Something went wrong.
 * @throws SystemException Something went wrong.
 * @throws FileSystemException Something went wrong.
 */
void makeDirTree(const string &path, const char *mode = "u=rwx,g=,o=", uid_t owner = (uid_t) -1, gid_t group = (gid_t) -1);

/**
 * Remove an entire directory tree recursively. If the directory doesn't exist then this
 * function does nothing.
 *
 * @throws FileSystemException Something went wrong.
 */
void removeDirTree(const string &path);

/**
 * Check whether the specified directory is a valid Ruby on Rails
 * application root directory.
 *
 * @param cstat A CachedFileStat object, if you want to use cached statting.
 * @param throttleRate A throttle rate for cstat. Only applicable if cstat is not NULL.
 * @throws FileSystemException Unable to check because of a system error.
 * @throws TimeRetrievalException
 * @throws boost::thread_interrupted
 * @ingroup Support
 */
bool verifyRailsDir(const string &dir, CachedFileStat *cstat = 0,
                    unsigned int throttleRate = 0);

/**
 * Check whether the specified directory is a valid Rack application
 * root directory.
 *
 * @param cstat A CachedFileStat object, if you want to use cached statting.
 * @param throttleRate A throttle rate for cstat. Only applicable if cstat is not NULL.
 * @throws FileSystemException Unable to check because of a filesystem error.
 * @throws TimeRetrievalException
 * @throws boost::thread_interrupted
 * @ingroup Support
 */
bool verifyRackDir(const string &dir, CachedFileStat *cstat = 0,
                   unsigned int throttleRate = 0);

/**
 * Check whether the specified directory is a valid WSGI application
 * root directory.
 *
 * @param cstat A CachedFileStat object, if you want to use cached statting.
 * @param throttleRate A throttle rate for cstat. Only applicable if cstat is not NULL.
 * @throws FileSystemException Unable to check because of a filesystem error.
 * @throws TimeRetrievalException
 * @throws boost::thread_interrupted
 * @ingroup Support
 */
bool verifyWSGIDir(const string &dir, CachedFileStat *cstat = 0,
                   unsigned int throttleRate = 0);

/**
 * Generate a secure, random token of the <tt>size</tt> bytes and put
 * the result into <tt>buf</tt>.
 *
 * This method is thread-safe.
 *
 * @throws FileSystemException
 * @throws IOException
 * @throws boost::thread_interrupted A system call has been interrupted.
 * @ingroup Support
 */
void generateSecureToken(void *buf, unsigned int size);

/**
 * Given a prefix string, a middle string and a postfix string, try to build a string
 * that looks like <tt>prefix + middle + postfix</tt>, with as many characters from
 * <tt>midle</tt> preserved as possible.
 *
 * If <tt>prefix + middle + postfix</tt> does not fit in <tt>max</tt> characters,
 * then <tt>middle</tt> will be truncated so that it fits. If <tt>max</tt> is too
 * small to contain even 1 character from <tt>middle</tt>, then an ArgumentException
 * will be thrown.
 *
 * @code
 *   fillInMiddle(18, "server.", "1234", ".socket");    // "server.1234.socket"
 *   fillInMiddle(16, "server.", "1234", ".socket");    // "server.12.socket"
 *   fillInMiddle(14, "server.", "1234", ".socket");    // ArgumentException
 * @endcode
 *
 * @returns The resulting string, with <tt>middle</tt> possibly truncated.
 * @throws ArgumentException <tt>max</tt> is too small to contain even 1
 *         character from <tt>middle</tt>.
 * @post result.size() <= max
 */
string fillInMiddle(unsigned int max, const string &prefix, const string &middle, const string &postfix = "");

/**
 * Convert the given binary data to hexadecimal.
 */
string toHex(const StaticString &data);

/**
 * Convert a signal number to its associated name.
 */
string getSignalName(int sig);

/**
 * Create a new Unix server socket which is bounded to <tt>filename</tt>.
 *
 * @param filename The filename to bind the socket to.
 * @param backlogSize The size of the socket's backlog. Specify 0 to use the
 *                    platform's maximum allowed backlog size.
 * @param autoDelete Whether <tt>filename</tt> should be deleted, if it already exists.
 * @return The file descriptor of the newly created Unix server socket.
 * @throws RuntimeException Something went wrong.
 * @throws SystemException Something went wrong while creating the Unix server socket.
 * @throws boost::thread_interrupted A system call has been interrupted.
 * @ingroup Support
 */
int createUnixServer(const char *filename, unsigned int backlogSize = 0, bool autoDelete = true);

/**
 * Connect to a Unix server socket at <tt>filename</tt>.
 *
 * @param filename The filename of the socket to connect to.
 * @return The file descriptor of the connected client socket.
 * @throws RuntimeException Something went wrong.
 * @throws SystemException Something went wrong while connecting to the Unix server.
 * @throws boost::thread_interrupted A system call has been interrupted.
 * @ingroup Support
 */
int connectToUnixServer(const char *filename);

/**
 * Connect to a TCP server socket at the given host name and port.
 *
 * @param hostname The host name of the TCP server.
 * @param port The port number of the TCP server.
 * @return The file descriptor of the connected client socket.
 * @throws IOException Something went wrong while connecting to the Unix server.
 * @throws SystemException Something went wrong while connecting to the Unix server.
 * @throws boost::thread_interrupted A system call has been interrupted.
 * @ingroup Support
 */
int connectToTcpServer(const char *hostname, unsigned int port);



/**
 * Represents a buffered upload file.
 *
 * @ingroup Support
 */
class BufferedUpload {
public:
	/** The file handle. */
	FILE *handle;
	
	/**
	 * Create an empty upload bufer file, and open it for reading and writing.
	 *
	 * @throws SystemException Something went wrong.
	 */
	BufferedUpload(const string &dir, const char *identifier = "temp") {
		char templ[PATH_MAX];
		int fd;
		
		snprintf(templ, sizeof(templ), "%s/%s.XXXXXX", dir.c_str(), identifier);
		templ[sizeof(templ) - 1] = '\0';
		fd = mkstemp(templ);
		if (fd == -1) {
			char message[1024];
			int e = errno;
			
			snprintf(message, sizeof(message), "Cannot create a temporary file '%s'", templ);
			message[sizeof(message) - 1] = '\0';
			throw SystemException(message, e);
		}
		
		/* We use a POSIX trick here: the file's permissions are set to "u=,g=,o="
		 * and the file is deleted immediately from the filesystem, while we
		 * keep its file handle open. The result is that no other processes
		 * will be able to access this file's contents anymore, except us.
		 * We now have an anonymous disk-backed buffer.
		 */
		fchmod(fd, 0000);
		unlink(templ);
		
		handle = fdopen(fd, "w+");
	}
	
	~BufferedUpload() {
		fclose(handle);
	}
};

/**
 * Fills the given memory space or string with zeroes when a MemoryZeroGuard object
 * is destroyed. Useful for ensuring that buffers containing password data or
 * other sensitive information is cleared when it goes out of scope.
 */
class MemZeroGuard {
private:
	void *data;
	unsigned int size;
	string *str;
	
	static void securelyZeroMemory(volatile void *data, unsigned int size) {
		/* We do not use memset() here because the compiler may
		 * optimize out memset() calls. Instead, the following
		 * code is guaranteed to zero the memory.
		 * http://www.dwheeler.com/secure-programs/Secure-Programs-HOWTO/protect-secrets.html
		 */
		volatile char *p = (volatile char *) data;
		while (size--) {
			*p++ = 0;
		}
	}
	
public:
	/**
	 * Creates a new MemZeroGuard object with a memory region to zero.
	 *
	 * @param data The data to zero after destruction.
	 * @param size The size of the data.
	 * @pre data != NULL
	 */
	MemZeroGuard(void *data, unsigned int size) {
		this->data = data;
		this->size = size;
		this->str  = NULL;
	}
	
	/**
	 * Creates a new MemoryZeroGuard object with a string to zero.
	 *
	 * @param str The string to zero after destruction.
	 */
	MemZeroGuard(string &str) {
		this->data = NULL;
		this->size = NULL;
		this->str  = &str;
	}
	
	/**
	 * Zero the data immediately. The data will still be zeroed after
	 * destruction of this object.
	 */
	void zeroNow() {
		if (str == NULL) {
			securelyZeroMemory(data, size);
		} else {
			securelyZeroMemory((volatile void *) str->c_str(), str->size());
		}
	}
	
	~MemZeroGuard() {
		zeroNow();
	}
};

} // namespace Passenger

#endif /* _PASSENGER_UTILS_H_ */

