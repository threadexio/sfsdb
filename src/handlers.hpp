#pragma once

#include "handlers/get.hpp"
#include "handlers/invalid.hpp"
#include "handlers/put.hpp"

inline protocol::cmd_table commands = {
	{protocol::commands::INVALID, handlers::invalid},
	{1, handlers::get},
	{2, handlers::put}};