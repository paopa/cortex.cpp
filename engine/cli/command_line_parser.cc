#include "command_line_parser.h"
#include <memory>
#include <optional>
#include <string>
#include "commands/cortex_upd_cmd.h"
#include "commands/engine_get_cmd.h"
#include "commands/engine_install_cmd.h"
#include "commands/engine_list_cmd.h"
#include "commands/engine_uninstall_cmd.h"
#include "commands/engine_update_cmd.h"
#include "commands/engine_use_cmd.h"
#include "commands/model_del_cmd.h"
#include "commands/model_get_cmd.h"
#include "commands/model_import_cmd.h"
#include "commands/model_list_cmd.h"
#include "commands/model_pull_cmd.h"
#include "commands/model_start_cmd.h"
#include "commands/model_stop_cmd.h"
#include "commands/model_upd_cmd.h"
#include "commands/ps_cmd.h"
#include "commands/run_cmd.h"
#include "commands/server_start_cmd.h"
#include "commands/server_stop_cmd.h"
#include "services/engine_service.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

namespace {
constexpr const auto kCommonCommandsGroup = "Common Commands";
constexpr const auto kInferenceGroup = "Inference";
constexpr const auto kModelsGroup = "Models";
constexpr const auto kEngineGroup = "Engines";
constexpr const auto kSystemGroup = "Server";
constexpr const auto kSubcommands = "Subcommands";
}  // namespace

CommandLineParser::CommandLineParser()
    : app_("\nCortex.cpp CLI\n"),
      download_service_{std::make_shared<DownloadService>()},
      model_service_{ModelService(download_service_)},
      engine_service_{EngineService(download_service_)} {}

bool CommandLineParser::SetupCommand(int argc, char** argv) {
  app_.usage("Usage:\n" + commands::GetCortexBinary() +
             " [options] [subcommand]");
  cml_data_.config = file_manager_utils::GetCortexConfig();
  std::string model_id;
  std::string msg;

  SetupCommonCommands();

  SetupInferenceCommands();

  SetupModelCommands();

  SetupEngineCommands();

  SetupSystemCommands();

  app_.add_flag("--verbose", log_verbose, "Get verbose logs");

  // Logic is handled in main.cc, just for cli helper command
  std::string path;
  app_.add_option("--config_file_path", path, "Configure .rc file path");
  app_.add_option("--data_folder_path", path, "Configure data folder path");

  // cortex version
  auto cb = [&](int c) {
#ifdef CORTEX_CPP_VERSION
    CLI_LOG(CORTEX_CPP_VERSION);
#else
    CLI_LOG("default");
#endif
  };
  app_.add_flag_function("-v,--version", cb, "Get Cortex version");

  CLI11_PARSE(app_, argc, argv);
  if (argc == 1) {
    CLI_LOG(app_.help());
    return true;
  }

  // Check new update
#ifdef CORTEX_CPP_VERSION
  if (cml_data_.check_upd) {
    if (strcmp(CORTEX_CPP_VERSION, "default_version") != 0) {
      // TODO(sang) find a better way to handle
      // This is an extremely ugly way to deal with connection
      // hang when network down
      std::atomic<bool> done = false;
      std::thread t([&]() {
        if (auto latest_version =
                commands::CheckNewUpdate(commands::kTimeoutCheckUpdate);
            latest_version.has_value() &&
            *latest_version != CORTEX_CPP_VERSION) {
          CLI_LOG("\nNew Cortex release available: "
                  << CORTEX_CPP_VERSION << " -> " << *latest_version);
          CLI_LOG("To update, run: " << commands::GetRole()
                                     << commands::GetCortexBinary()
                                     << " update");
        }
        done = true;
      });
      // Do not wait for http connection timeout
      t.detach();
      int retry = 10;
      while (!done && retry--) {
        std::this_thread::sleep_for(commands::kTimeoutCheckUpdate / 10);
      }
    }
  }
#endif

  return true;
}

void CommandLineParser::SetupCommonCommands() {
  auto model_pull_cmd = app_.add_subcommand(
      "pull",
      "Download models by HuggingFace Repo/ModelID"
      "See built-in models: https://huggingface.co/cortexso");
  model_pull_cmd->group(kCommonCommandsGroup);
  model_pull_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " pull [options] [model_id]");
  model_pull_cmd->add_option("model_id", cml_data_.model_id, "");
  model_pull_cmd->callback([this, model_pull_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_pull_cmd->help());
      return;
    }
    try {
      commands::ModelPullCmd(download_service_)
          .Exec(cml_data_.config.apiServerHost,
                std::stoi(cml_data_.config.apiServerPort), cml_data_.model_id);
    } catch (const std::exception& e) {
      CLI_LOG(e.what());
    }
  });

  auto run_cmd =
      app_.add_subcommand("run", "Shortcut: pull, start & chat with a model");
  run_cmd->group(kCommonCommandsGroup);
  run_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                 " run [options] [model_id]");
  run_cmd->add_option("model_id", cml_data_.model_id, "");
  run_cmd->add_flag("-d,--detach", cml_data_.run_detach, "Detached mode");
  run_cmd->callback([this, run_cmd] {
    if (std::exchange(executed_, true))
      return;
    commands::RunCmd rc(cml_data_.config.apiServerHost,
                        std::stoi(cml_data_.config.apiServerPort),
                        cml_data_.model_id, download_service_);
    rc.Exec(cml_data_.run_detach);
  });
}

void CommandLineParser::SetupInferenceCommands() {
  // auto embeddings_cmd = app_.add_subcommand(
  //     "embeddings", "Creates an embedding vector representing the input text");
  // embeddings_cmd->group(kInferenceGroup);
}

void CommandLineParser::SetupModelCommands() {
  // Models group commands
  auto models_cmd =
      app_.add_subcommand("models", "Subcommands for managing models");
  models_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                    " models [options] [subcommand]");
  models_cmd->group(kModelsGroup);

  models_cmd->callback([this, models_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (models_cmd->get_subcommands().empty()) {
      CLI_LOG(models_cmd->help());
    }
  });

  auto model_start_cmd =
      models_cmd->add_subcommand("start", "Start a model by ID");
  model_start_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                         " models start [model_id]");
  model_start_cmd->add_option("model_id", cml_data_.model_id, "");
  model_start_cmd->group(kSubcommands);
  model_start_cmd->callback([this, model_start_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_start_cmd->help());
      return;
    };
    commands::ModelStartCmd(model_service_)
        .Exec(cml_data_.config.apiServerHost,
              std::stoi(cml_data_.config.apiServerPort), cml_data_.model_id);
  });

  auto stop_model_cmd =
      models_cmd->add_subcommand("stop", "Stop a model by ID");
  stop_model_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " models stop [model_id]");
  stop_model_cmd->group(kSubcommands);
  stop_model_cmd->add_option("model_id", cml_data_.model_id, "");
  stop_model_cmd->callback([this, stop_model_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(stop_model_cmd->help());
      return;
    };
    commands::ModelStopCmd(model_service_)
        .Exec(cml_data_.config.apiServerHost,
              std::stoi(cml_data_.config.apiServerPort), cml_data_.model_id);
  });

  auto list_models_cmd =
      models_cmd->add_subcommand("list", "List all local models");
  list_models_cmd->add_option("filter", cml_data_.filter, "Filter model id");
  list_models_cmd->add_flag("-e,--engine", cml_data_.display_engine,
                            "Display engine");
  list_models_cmd->add_flag("-v,--version", cml_data_.display_version,
                            "Display version");
  list_models_cmd->group(kSubcommands);
  list_models_cmd->callback([this]() {
    if (std::exchange(executed_, true))
      return;
    commands::ModelListCmd().Exec(cml_data_.config.apiServerHost,
                                  std::stoi(cml_data_.config.apiServerPort),
                                  cml_data_.filter, cml_data_.display_engine,
                                  cml_data_.display_version);
  });

  auto get_models_cmd =
      models_cmd->add_subcommand("get", "Get a local model info by ID");
  get_models_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " models get [model_id]");
  get_models_cmd->group(kSubcommands);
  get_models_cmd->add_option("model_id", cml_data_.model_id, "");
  get_models_cmd->callback([this, get_models_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(get_models_cmd->help());
      return;
    };
    commands::ModelGetCmd().Exec(cml_data_.config.apiServerHost,
                                 std::stoi(cml_data_.config.apiServerPort),
                                 cml_data_.model_id);
  });

  auto model_del_cmd =
      models_cmd->add_subcommand("delete", "Delete a local model by ID");
  model_del_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                       " models delete [model_id]");
  model_del_cmd->group(kSubcommands);
  model_del_cmd->add_option("model_id", cml_data_.model_id, "");
  model_del_cmd->callback([&]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_del_cmd->help());
      return;
    };

    commands::ModelDelCmd().Exec(cml_data_.config.apiServerHost,
                                 std::stoi(cml_data_.config.apiServerPort),
                                 cml_data_.model_id);
  });

  // Model update parameters comment
  ModelUpdate(models_cmd);

  std::string model_path;
  auto model_import_cmd = models_cmd->add_subcommand(
      "import", "Import a model from a local filepath");
  model_import_cmd->usage(
      "Usage:\n" + commands::GetCortexBinary() +
      " models import --model_id [model_id] --model_path [model_path]");
  model_import_cmd->group(kSubcommands);
  model_import_cmd->add_option("--model_id", cml_data_.model_id, "");
  model_import_cmd->add_option("--model_path", cml_data_.model_path,
                               "Absolute path to .gguf model, the path should "
                               "include the gguf file name");
  model_import_cmd->callback([this, model_import_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty() || cml_data_.model_path.empty()) {
      CLI_LOG("[model_id] and [model_path] are required\n");
      CLI_LOG(model_import_cmd->help());
      return;
    }
    commands::ModelImportCmd().Exec(cml_data_.config.apiServerHost,
                                    std::stoi(cml_data_.config.apiServerPort),
                                    cml_data_.model_id, cml_data_.model_path);
  });
}

void CommandLineParser::SetupEngineCommands() {
  auto engines_cmd =
      app_.add_subcommand("engines", "Subcommands for managing engines");
  engines_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                     " engines [options] [subcommand]");
  engines_cmd->group(kEngineGroup);
  engines_cmd->callback([this, engines_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (engines_cmd->get_subcommands().empty()) {
      CLI_LOG("A subcommand is required\n");
      CLI_LOG(engines_cmd->help());
    }
  });

  auto list_engines_cmd =
      engines_cmd->add_subcommand("list", "List all cortex engines");
  list_engines_cmd->group(kSubcommands);
  list_engines_cmd->callback([this]() {
    if (std::exchange(executed_, true))
      return;
    commands::EngineListCmd command;
    command.Exec(cml_data_.config.apiServerHost,
                 std::stoi(cml_data_.config.apiServerPort));
  });

  auto install_cmd = engines_cmd->add_subcommand("install", "Install engine");
  install_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                     " engines install [engine_name] [options]");
  install_cmd->group(kSubcommands);
  install_cmd->callback([this, install_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (install_cmd->get_subcommands().empty()) {
      CLI_LOG("[engine_name] is required\n");
      CLI_LOG(install_cmd->help());
    }
  });
  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    EngineInstall(install_cmd, engine_name, cml_data_.engine_version,
                  cml_data_.engine_src, cml_data_.show_menu);
  }

  auto uninstall_cmd =
      engines_cmd->add_subcommand("uninstall", "Uninstall engine");
  uninstall_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                       " engines uninstall [engine_name] [options]");
  uninstall_cmd->callback([this, uninstall_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (uninstall_cmd->get_subcommands().empty()) {
      CLI_LOG("[engine_name] is required\n");
      CLI_LOG(uninstall_cmd->help());
    }
  });
  uninstall_cmd->group(kSubcommands);
  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    EngineUninstall(uninstall_cmd, engine_name);
  }

  auto engine_upd_cmd = engines_cmd->add_subcommand("update", "Update engine");
  engine_upd_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " engines update [engine_name]");
  engine_upd_cmd->callback([this, engine_upd_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (engine_upd_cmd->get_subcommands().empty()) {
      CLI_LOG("[engine_name] is required\n");
      CLI_LOG(engine_upd_cmd->help());
    }
  });
  engine_upd_cmd->group(kSubcommands);
  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    EngineUpdate(engine_upd_cmd, engine_name);
  }

  auto engine_use_cmd =
      engines_cmd->add_subcommand("use", "Set engine as default");
  engine_use_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " engines use [engine_name]");
  engine_use_cmd->callback([this, engine_use_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (engine_use_cmd->get_subcommands().empty()) {
      CLI_LOG("[engine_name] is required\n");
      CLI_LOG(engine_use_cmd->help());
    }
  });
  engine_use_cmd->group(kSubcommands);
  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    EngineUse(engine_use_cmd, engine_name);
  }

  EngineGet(engines_cmd);
}

void CommandLineParser::SetupSystemCommands() {
  auto start_cmd = app_.add_subcommand("start", "Start the API server");
  start_cmd->group(kSystemGroup);
  cml_data_.port = std::stoi(cml_data_.config.apiServerPort);
  start_cmd->add_option("-p, --port", cml_data_.port, "Server port to listen");
  start_cmd->callback([this] {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.port != stoi(cml_data_.config.apiServerPort)) {
      CTL_INF("apiServerPort changed from " << cml_data_.config.apiServerPort
                                            << " to " << cml_data_.port);
      auto config_path = file_manager_utils::GetConfigurationPath();
      cml_data_.config.apiServerPort = std::to_string(cml_data_.port);
      auto result = config_yaml_utils::DumpYamlConfig(cml_data_.config,
                                                      config_path.string());
      if (result.has_error()) {
        CLI_LOG("Error update " << config_path.string() << result.error());
      }
    }
    commands::ServerStartCmd ssc;
    ssc.Exec(cml_data_.config.apiServerHost,
             std::stoi(cml_data_.config.apiServerPort));
  });

  auto stop_cmd = app_.add_subcommand("stop", "Stop the API server");
  stop_cmd->group(kSystemGroup);
  stop_cmd->callback([this] {
    if (std::exchange(executed_, true))
      return;
    commands::ServerStopCmd ssc(cml_data_.config.apiServerHost,
                                std::stoi(cml_data_.config.apiServerPort));
    ssc.Exec();
  });

  auto ps_cmd = app_.add_subcommand("ps", "Show active model statuses");
  ps_cmd->group(kSystemGroup);
  ps_cmd->usage("Usage:\n" + commands::GetCortexBinary() + "ps");
  ps_cmd->callback([&]() {
    if (std::exchange(executed_, true))
      return;
    commands::PsCmd().Exec(cml_data_.config.apiServerHost,
                           std::stoi(cml_data_.config.apiServerPort));
  });

  auto update_cmd = app_.add_subcommand("update", "Update cortex version");
  update_cmd->group(kSystemGroup);
  update_cmd->add_option("-v", cml_data_.cortex_version, "");
  update_cmd->callback([this] {
    if (std::exchange(executed_, true))
      return;
#if !defined(_WIN32)
    if (getuid()) {
      CLI_LOG("Error: Not root user. Please run with sudo.");
      return;
    }
#endif
    auto cuc = commands::CortexUpdCmd(download_service_);
    cuc.Exec(cml_data_.cortex_version);
    cml_data_.check_upd = false;
  });
}

void CommandLineParser::EngineInstall(CLI::App* parent,
                                      const std::string& engine_name,
                                      std::string& version, std::string& src,
                                      bool show_menu) {
  auto install_engine_cmd = parent->add_subcommand(engine_name, "");
  install_engine_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                            " engines install " + engine_name + " [options]");
  install_engine_cmd->group(kEngineGroup);

  install_engine_cmd->add_option("-v, --version", version,
                                 "Engine version to download");

  install_engine_cmd->add_option("-s, --source", src,
                                 "Install engine by local path");

  install_engine_cmd->add_option("-m, --menu", show_menu,
                                 "Display menu for engine variant selection");

  install_engine_cmd->callback([this, engine_name, &version, &src, show_menu] {
    if (std::exchange(executed_, true))
      return;
    try {
      commands::EngineInstallCmd(
          download_service_, cml_data_.config.apiServerHost,
          std::stoi(cml_data_.config.apiServerPort), show_menu)
          .Exec(engine_name, version, src);
    } catch (const std::exception& e) {
      CTL_ERR(e.what());
    }
  });
}

void CommandLineParser::EngineUninstall(CLI::App* parent,
                                        const std::string& engine_name) {
  auto uninstall_engine_cmd = parent->add_subcommand(engine_name, "");
  uninstall_engine_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                              " engines install " + engine_name + " [options]");
  uninstall_engine_cmd->group(kEngineGroup);

  uninstall_engine_cmd->callback([this, engine_name] {
    if (std::exchange(executed_, true))
      return;
    try {
      commands::EngineUninstallCmd().Exec(
          cml_data_.config.apiServerHost,
          std::stoi(cml_data_.config.apiServerPort), engine_name);
    } catch (const std::exception& e) {
      CTL_ERR(e.what());
    }
  });
}

void CommandLineParser::EngineUpdate(CLI::App* parent,
                                     const std::string& engine_name) {
  auto engine_update_cmd = parent->add_subcommand(engine_name, "");
  engine_update_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                           " engines update " + engine_name);
  engine_update_cmd->group(kEngineGroup);

  engine_update_cmd->callback([this, engine_name] {
    if (std::exchange(executed_, true))
      return;
    try {
      commands::EngineUpdateCmd().Exec(
          cml_data_.config.apiServerHost,
          std::stoi(cml_data_.config.apiServerPort), engine_name);
    } catch (const std::exception& e) {
      CTL_ERR(e.what());
    }
  });
}

void CommandLineParser::EngineUse(CLI::App* parent,
                                  const std::string& engine_name) {
  auto engine_use_cmd = parent->add_subcommand(engine_name, "");
  engine_use_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " engines use " + engine_name);
  engine_use_cmd->group(kEngineGroup);

  engine_use_cmd->callback([this, engine_name] {
    if (std::exchange(executed_, true))
      return;
    auto result = commands::EngineUseCmd().Exec(
        cml_data_.config.apiServerHost,
        std::stoi(cml_data_.config.apiServerPort), engine_name);
    if (result.has_error()) {
      CTL_ERR(result.error());
    } else {
      CTL_INF("Engine " << engine_name << " is set as default");
    }
  });
}

void CommandLineParser::EngineGet(CLI::App* parent) {
  auto get_cmd = parent->add_subcommand("get", "Get engine info");
  get_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                 " engines get [engine_name] [options]");
  get_cmd->group(kSubcommands);
  get_cmd->callback([this, get_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (get_cmd->get_subcommands().empty()) {
      CLI_LOG("[engine_name] is required\n");
      CLI_LOG(get_cmd->help());
    }
  });

  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    std::string desc = "Get " + engine_name + " status";

    auto engine_get_cmd = get_cmd->add_subcommand(engine_name, desc);
    engine_get_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                          " engines get " + engine_name + " [options]");
    engine_get_cmd->group(kEngineGroup);
    engine_get_cmd->callback([this, engine_name] {
      if (std::exchange(executed_, true))
        return;
      commands::EngineGetCmd().Exec(cml_data_.config.apiServerHost,
                                    std::stoi(cml_data_.config.apiServerPort),
                                    engine_name);
    });
  }
}

void CommandLineParser::ModelUpdate(CLI::App* parent) {
  auto model_update_cmd =
      parent->add_subcommand("update", "Update model configurations");
  model_update_cmd->group(kSubcommands);
  model_update_cmd->add_option("--model_id", cml_data_.model_id, "Model ID")
      ->required();

  // Add options dynamically
  std::vector<std::string> option_names = {"name",
                                           "model",
                                           "version",
                                           "stop",
                                           "top_p",
                                           "temperature",
                                           "frequency_penalty",
                                           "presence_penalty",
                                           "max_tokens",
                                           "stream",
                                           "ngl",
                                           "ctx_len",
                                           "n_parallel",
                                           "engine",
                                           "prompt_template",
                                           "system_template",
                                           "user_template",
                                           "ai_template",
                                           "os",
                                           "gpu_arch",
                                           "quantization_method",
                                           "precision",
                                           "tp",
                                           "trtllm_version",
                                           "text_model",
                                           "files",
                                           "created",
                                           "object",
                                           "owned_by",
                                           "seed",
                                           "dynatemp_range",
                                           "dynatemp_exponent",
                                           "top_k",
                                           "min_p",
                                           "tfs_z",
                                           "typ_p",
                                           "repeat_last_n",
                                           "repeat_penalty",
                                           "mirostat",
                                           "mirostat_tau",
                                           "mirostat_eta",
                                           "penalize_nl",
                                           "ignore_eos",
                                           "n_probs",
                                           "min_keep",
                                           "grammar"};

  for (const auto& option_name : option_names) {
    model_update_cmd->add_option("--" + option_name,
                                 cml_data_.model_update_options[option_name],
                                 option_name);
  }

  model_update_cmd->callback([this]() {
    if (std::exchange(executed_, true))
      return;
    commands::ModelUpdCmd command(cml_data_.model_id);
    command.Exec(cml_data_.config.apiServerHost,
                 std::stoi(cml_data_.config.apiServerPort),
                 cml_data_.model_update_options);
  });
}
