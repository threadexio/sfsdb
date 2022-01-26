#pragma once

#include "handlers/del.hpp"
#include "handlers/desc.hpp"
#include "handlers/get.hpp"
#include "handlers/invalid.hpp"
#include "handlers/list.hpp"
#include "handlers/put.hpp"
#include "handlers/quit.hpp"

inline protocol::cmd_table commands = {
	{protocol::commands::INVALID, handlers::invalid},
	{1, handlers::get},
	{2, handlers::put},
	{3, handlers::desc},
	{4, handlers::del},
	{5, handlers::list},
	{6, handlers::quit}};