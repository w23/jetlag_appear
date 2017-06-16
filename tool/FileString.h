#pragma once

#include "atto/app.h"

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class FileChangePoller {
	const std::string filename_;
	timespec mtime_;

public:
	FileChangePoller(const char *filename) : filename_(filename) {
		mtime_.tv_sec = 0;
		mtime_.tv_nsec = 0;
	}
	~FileChangePoller() {}

	std::string poll() {
		std::string content;
		struct stat st;
		stat(filename_.c_str(), &st);
		if (st.st_mtim.tv_sec == mtime_.tv_sec &&
				st.st_mtim.tv_nsec == mtime_.tv_nsec)
			return content;

		mtime_ = st.st_mtim;

		int fd = open(filename_.c_str(), O_RDONLY);
		if (fd < 0) {
			aAppDebugPrintf("Cannot open file %s", filename_.c_str());
			return content;
		}

		content.resize(st.st_size + 1, 0);

		if (read(fd, &content[0], st.st_size) != st.st_size) {
			aAppDebugPrintf("Cannot read file\n");
			content.resize(0);
			return content;
		}

		aAppDebugPrintf("Reread file %s", filename_.c_str());

		close(fd);
		return content;
	}
};

class String {
protected:
	std::string value_;

public:
	String(const std::string &str) : value_(str) {}
	const std::string &string() const { return value_; }
	virtual bool update() { return false; }
};

class FileString : public String {
	FileChangePoller poller_;

public:
	FileString(const char *filename) : String(""), poller_(filename) {}

	bool update() {
		std::string new_content = poller_.poll();
		if (!new_content.empty()) {
			value_ = std::move(new_content);
			return true;
		}
		return false;
	}
};
