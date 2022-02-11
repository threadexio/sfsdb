#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include "common.hpp"
#include "log.hpp"
#include "storage.hpp"

namespace hooks {

	enum class type { PRE, POST };

	inline auto run_hooks(type _hook_type) {
		Result<void*, Error> ret;

		std::string hook_path;
		switch (_hook_type) {
			case type::PRE:
				hook_path = storage::HOOK_PRE_DIR;
				break;

			case type::POST:
				hook_path = storage::HOOK_POST_DIR;
				break;
		}

		std::vector<std::string> hooks;
		for (const auto& hook : std::filesystem::directory_iterator(hook_path))
			hooks.push_back(hook.path());

		std::sort(hooks.begin(),
				  hooks.end(),
				  [](const std::string& x, const std::string& y) -> bool {
					  return x < y;
				  });

		for (const auto& hook : hooks) {
			auto hret = std::system(("./" + hook).c_str());

			auto status = WEXITSTATUS(hret);

			if (hret < 0)
				return std::move(ret.Err(Error(errno)));

			if (status != 0) {
				// If hooks return this magic number then it means something
				// went terribly bad and we shouldn't continue
				if (status == 164) {
					plog::v(
						LOG_ERROR "hook", "%s fatal: %d", hook.c_str(), status);
					return std::move(
						ret.Err(Error(1, "Hook returned fatal status code")));
				}

				plog::v(
					LOG_WARNING "hook", "%s error: %d", hook.c_str(), status);
				continue;
			}
		}

		return std::move(ret.Ok(nullptr));
	}

} // namespace hooks