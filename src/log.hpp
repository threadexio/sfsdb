#pragma once

#include <cstdio>
#include <ctime>
#include <string>

// source: https://gist.github.com/JBlond/2fea43a3049b38287e5e9cefc87b2124
#define LOG_COLOR_BLACK	 "\e[0;30m"
#define LOG_COLOR_RED	 "\e[0;31m"
#define LOG_COLOR_GREEN	 "\e[0;32m"
#define LOG_COLOR_YELLOW "\e[0;33m"
#define LOG_COLOR_BLUE	 "\e[0;34m"
#define LOG_COLOR_PURPLE "\e[0;35m"
#define LOG_COLOR_CYAN	 "\e[0;36m"
#define LOG_COLOR_WHITE	 "\e[0;37m"
#define LOG_COLOR_RESET	 "\e[0m"

#define LOG_COLOR_BOLD_BLACK  "\e[1;30m"
#define LOG_COLOR_BOLD_RED	  "\e[1;31m"
#define LOG_COLOR_BOLD_GREEN  "\e[1;32m"
#define LOG_COLOR_BOLD_YELLOW "\e[1;33m"
#define LOG_COLOR_BOLD_BLUE	  "\e[1;34m"
#define LOG_COLOR_BOLD_PURPLE "\e[1;35m"
#define LOG_COLOR_BOLD_CYAN	  "\e[1;36m"
#define LOG_COLOR_BOLD_WHITE  "\e[1;37m"

#define LOG_INFO	LOG_COLOR_BOLD_WHITE
#define LOG_NOTICE	LOG_COLOR_CYAN
#define LOG_WARNING LOG_COLOR_YELLOW
#define LOG_ERROR	LOG_COLOR_BOLD_RED

// This logger does NOT have JNDI lookups :)
namespace plog {
	/**
	 * @brief Log a message to console.
	 *
	 * @param _id Id of the caller
	 * @param _msg Message to log
	 */
	void v(const std::string& _id, const std::string& _msg);
} // namespace plog