#include "engine_list_cmd.h"
#include <json/reader.h>
#include <json/value.h>
#include "server_start_cmd.h"
#include "services/engine_service.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"
// clang-format off
#include <tabulate/table.hpp>
#include <unordered_map>
// clang-format on

namespace commands {

bool EngineListCmd::Exec(const std::string& host, int port) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return false;
    }
  }

  tabulate::Table table;
  table.add_row({"#", "Name", "Version", "Variant", "Status"});

  auto url = url_parser::Url{
      .protocol = "http",
      .host = host + ":" + std::to_string(port),
      .pathParams = {"v1", "engines"},
  };
  auto result = curl_utils::SimpleGetJson(url.ToFullPath());
  if (result.has_error()) {
    CTL_ERR(result.error());
    return false;
  }

  std::vector<std::string> engines = {
      kLlamaEngine,
      kOnnxEngine,
      kTrtLlmEngine,
  };

  std::unordered_map<std::string, std::vector<EngineVariantResponse>>
      engine_map;

  for (const auto& engine : engines) {
    auto installed_variants = result.value()[engine];
    for (const auto& variant : installed_variants) {
      engine_map[engine].push_back(EngineVariantResponse{
          .name = variant["name"].asString(),
          .version = variant["version"].asString(),
          .engine = engine,
      });
    }
  }

  std::vector<EngineVariantResponse> output;
  for (const auto& [key, value] : engine_map) {
    output.insert(output.end(), value.begin(), value.end());
  }

  int count = 0;
  for (auto const& v : output) {
    count += 1;
    table.add_row(
        {std::to_string(count), v.engine, v.version, v.name, "Installed"});
  }

  std::cout << table << std::endl;
  return true;
}
};  // namespace commands
