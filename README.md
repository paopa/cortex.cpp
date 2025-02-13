# Cortex.cpp

<p align="center">
<img width="1280" alt="Cortex cpp's Readme Banner" src="https://github.com/user-attachments/assets/a27c0435-b3b4-406f-b575-96ac4f12244c">

</p>

<p align="center">
  <!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
  <img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/m/janhq/cortex.cpp"/>
  <img alt="Github Last Commit" src="https://img.shields.io/github/last-commit/janhq/cortex.cpp"/>
  <img alt="Github Contributors" src="https://img.shields.io/github/contributors/janhq/cortex.cpp"/>
  <img alt="GitHub closed issues" src="https://img.shields.io/github/issues-closed/janhq/cortex.cpp"/>
  <img alt="Discord" src="https://img.shields.io/discord/1107178041848909847?label=discord"/>
</p>

<p align="center">
  <a href="https://cortex.so/docs/">Documentation</a> - <a href="https://cortex.so/api-reference">API Reference</a> 
  - <a href="https://github.com/janhq/cortex.cpp/releases">Changelog</a> - <a href="https://github.com/janhq/cortex.cpp/issues">Bug reports</a> - <a href="https://discord.gg/AsJ8krTT3N">Discord</a>
</p>

> **Cortex.cpp is currently in active development.**

## Overview

Cortex is a Local AI API Platform that is used to run and customize LLMs. 

Key Features:
- Pull from Huggingface, or Cortex Built-in Models
- Models stored in universal file formats (vs blobs)
- Swappable Engines (default: [`llamacpp`](https://github.com/janhq/cortex.llamacpp), future: [`ONNXRuntime`](https://github.com/janhq/cortex.onnx), [`TensorRT-LLM`](https://github.com/janhq/cortex.tensorrt-llm))
- Cortex can be deployed as a standalone API server, or integrated into apps like [Jan.ai](https://jan.ai/)

Coming soon; now available on [cortex-nightly](#beta--nightly-versions):
- Engines Management (install specific llama-cpp version and variants)
- Nvidia Hardware detection & activation (current: Nvidia, future: AMD, Intel, Qualcomm)
- Cortex's roadmap is to implement the full OpenAI API including Tools, Runs, Multi-modal and Realtime APIs.

## Local Installation

Cortex has an Local Installer that packages all required dependencies, so that no internet connection is required during the installation process.

Cortex also has a [Network Installer](#network-installer) which downloads the necessary dependencies from the internet during the installation.

<h4> 
  <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:15px; width: 15px" />
  Windows: 
  <a href='https://app.cortexcpp.com/download/latest/windows-amd64-local'><b>cortex.exe</b></a>
</h4>

<h4> 
  <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
  MacOS (Silicon/Intel): 
 <a href='https://app.cortexcpp.com/download/latest/mac-universal-local'><b>cortex.pkg</b></a>
</h4>

<h4> 
  <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:15px; width: 15px" />
  Linux debian based distros: 
 <a href='https://app.cortexcpp.com/download/latest/linux-amd64-local'><b>cortex-linux-local-installer.deb</b></a>
</h4>

- For Linux: Download the installer and run the following command in terminal:

```bash
    # Linux debian based distros
    curl -s https://raw.githubusercontent.com/janhq/cortex/main/engine/templates/linux/install.sh | sudo bash -s -- --deb_local

    # Other Linux distros
    curl -s https://raw.githubusercontent.com/janhq/cortex/main/engine/templates/linux/install.sh | sudo bash -s
```

- The binary will be installed in the `/usr/bin/` directory.

## Usage

### CLI

After installation, you can run Cortex.cpp from the command line by typing `cortex --help`.

```
# Run a Model
cortex pull llama3.2                                    
cortex pull bartowski/Meta-Llama-3.1-8B-Instruct-GGUF
cortex run llama3.2                          

# Resource Management
cortex ps                               (view active models & RAM/VRAM used)       
cortex models stop llama3.2                

# Available on cortex-nightly:
cortex engines install llama-cpp -m     (lists versions and variants)
cortex hardware list                    (hardware detection)
cortex hardware activate   

cortex stop
```

Refer to our [Quickstart](https://cortex.so/docs/quickstart/) and 
[CLI documentation](https://cortex.so/docs/cli) for more details.

### API:
Cortex.cpp includes a REST API accessible at `localhost:39281`.

Refer to our [API documentation](https://cortex.so/api-reference) for more details.

## Models

Cortex.cpp allows users to pull models from multiple Model Hubs, offering flexibility and extensive model access:
- [Hugging Face](https://huggingface.co): GGUF models eg `author/Model-GGUF`
- Cortex Built-in Models 

Once downloaded, the model `.gguf` and `model.yml` files are stored in `~\cortexcpp\models`.

> **Note**:
> You should have at least 8 GB of RAM available to run the 7B models, 16 GB to run the 14B models, and 32 GB to run the 32B models.

### Cortex Built-in Models & Quantizations

| Model /Engine  | llama.cpp             | Command                       |
| -------------- | --------------------- | ----------------------------- |
| phi-3.5        | ✅                    | cortex run phi3.5             |
| llama3.2       | ✅                    | cortex run llama3.2           |
| llama3.1       | ✅                    | cortex run llama3.1           |
| codestral      | ✅                    | cortex run codestral          |
| gemma2         | ✅                    | cortex run gemma2             |
| mistral        | ✅                    | cortex run mistral            |
| ministral      | ✅                    | cortex run ministral          |
| qwen2          | ✅                    | cortex run qwen2.5            |
| openhermes-2.5 | ✅                    | cortex run openhermes-2.5     |
| tinyllama      | ✅                    | cortex run tinyllama          |

View all [Cortex Built-in Models](https://cortex.so/models).

Cortex supports multiple quantizations for each model.
```
❯ cortex-nightly pull llama3.2
Downloaded models:
    llama3.2:3b-gguf-q2-k

Available to download:
    1. llama3.2:3b-gguf-q3-kl
    2. llama3.2:3b-gguf-q3-km
    3. llama3.2:3b-gguf-q3-ks
    4. llama3.2:3b-gguf-q4-km (default)
    5. llama3.2:3b-gguf-q4-ks
    6. llama3.2:3b-gguf-q5-km
    7. llama3.2:3b-gguf-q5-ks
    8. llama3.2:3b-gguf-q6-k
    9. llama3.2:3b-gguf-q8-0

Select a model (1-9): 
```

## Advanced Installation

### Network Installer (Stable)

Cortex.cpp is available with a Network Installer, which is a smaller installer but requires internet connection during installation to download the necessary dependencies.

<h4> 
  <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
  Windows: 
  <a href='https://app.cortexcpp.com/download/latest/windows-amd64-local'><b>cortex-windows-network-installer.exe</b></a>
</h4>

<h4> 
  <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
  MacOS (Universal): 
 <a href='https://app.cortexcpp.com/download/latest/mac-universal-network'><b>cortex-mac-network-installer.pkg</b></a>
</h4>

<h4> 
  <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 15px" />
  Linux debian based distros: 
 <a href='https://app.cortexcpp.com/download/latest/linux-amd64-network'><b>cortex-linux-network-installer.deb</b></a>
</h4>
  
### Beta & Nightly Versions (Local Installer)

Cortex releases Beta and Nightly versions for advanced users to try new features (we appreciate your feedback!)
- Beta (early preview): CLI command: `cortex-beta`
- Nightly (released every night): CLI Command: `cortex-nightly`
  - Nightly automatically pulls the latest changes from upstream [llama.cpp](https://github.com/ggerganov/llama.cpp/) repo, creates a PR and runs tests. 
  - If all test pass, the PR is automatically merged into our repo, with the latest llama.cpp version.

<table>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Version</b></td>
    <td style="text-align:center"><b>Windows</b></td>
    <td style="text-align:center"><b>MacOS</b></td>
    <td style="text-align:center"><b>Linux debian based distros</b></td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Beta (Preview)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/windows-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
        cortex.exe
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/mac-universal-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        cortex.pkg
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/linux-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 14px" />
        cortex.deb
      </a>
    </td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Nightly (Experimental)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/windows-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:14px; width: 14px" />
        cortex.exe
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/mac-universal-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        cortex.pkg
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/linux-amd64-local'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:14px; width: 14px" />
        cortex.deb
      </a>
    </td>
  </tr>
</table>

### Network Installer

Cortex.cpp is available with a Network Installer, which is a smaller installer but requires internet connection during installation to download the necessary dependencies.

<table>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Version Type</b></td>
    <td style="text-align:center"><b>Windows</b></td>
    <td style="text-align:center"><b>MacOS</b></td>
    <td style="text-align:center"><b>Linux debian based distros</b></td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Stable (Recommended)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/latest/windows-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:15px; width: 15px" />
        cortex.exe
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/latest/mac-universal-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        cortex.pkg
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/latest/linux-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:15px; width: 15px" />
        cortex.deb
      </a>
    </td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Beta (Preview)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/windows-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:15px; width: 15px" />
        cortex.exe
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/mac-universal-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        cortex.pkg
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/beta/linux-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:15px; width: 15px" />
        cortex.deb
      </a>
    </td>
  </tr>
  <tr style="text-align:center">
    <td style="text-align:center"><b>Nightly (Experimental)</b></td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/windows-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/windows.png' style="height:15px; width: 15px" />
        cortex.exe
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/mac-universal-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/mac.png' style="height:15px; width: 15px" />
        cortex.pkg
      </a>
    </td>
    <td style="text-align:center">
      <a href='https://app.cortexcpp.com/download/nightly/linux-amd64-network'>
        <img src='https://github.com/janhq/docs/blob/main/static/img/linux.png' style="height:15px; width: 15px" />
        cortex.deb
      </a>
    </td>
  </tr>
</table>

### Build from Source

#### Windows

1. Clone the Cortex.cpp repository [here](https://github.com/janhq/cortex.cpp).
2. Navigate to the `engine` folder.
3. Configure the vpkg:

```bash
cd vcpkg
./bootstrap-vcpkg.bat
vcpkg install
```

4. Build the Cortex.cpp inside the `engine/build` folder:

```bash
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg_folder_in_cortex_repo/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build . --config Release
```

5. Verify that Cortex.cpp is installed correctly by getting help information.

```sh
cortex -h
```

#### MacOS

1. Clone the Cortex.cpp repository [here](https://github.com/janhq/cortex.cpp).
2. Navigate to the `engine` folder.
3. Configure the vpkg:

```bash
cd vcpkg
./bootstrap-vcpkg.sh
vcpkg install
```

4. Build the Cortex.cpp inside the `engine/build` folder:

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg_folder_in_cortex_repo/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j4
```

5. Verify that Cortex.cpp is installed correctly by getting help information.

```sh
cortex -h
```

#### Linux

1. Clone the Cortex.cpp repository [here](https://github.com/janhq/cortex.cpp).
2. Navigate to the `engine` folder.
3. Configure the vpkg:

```bash
cd vcpkg
./bootstrap-vcpkg.sh
vcpkg install
```

4. Build the Cortex.cpp inside the `engine/build` folder:

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg_folder_in_cortex_repo/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j4
```

5. Verify that Cortex.cpp is installed correctly by getting help information.

```sh
cortex -h
```

#### Devcontainer / Codespaces

1. Open Cortex.cpp repository in Codespaces or local devcontainer

    [![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/janhq/cortex.cpp?quickstart=1)

    ```sh
    devcontainer up --workspace-folder .
    ```

2. Configure vpkg in `engine/vcpkg`:

```bash {"tag": "devcontainer"}
cd engine/vcpkg
export VCPKG_FORCE_SYSTEM_BINARIES="$([[ $(uname -m) == 'arm64' ]] && echo '1' || echo '0')"
./bootstrap-vcpkg.sh
```

3. Build the Cortex.cpp inside the `engine/build` folder:

```bash {"tag": "devcontainer"}
cd engine
mkdir -p build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$(realpath ..)/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j$(grep -c ^processor /proc/cpuinfo)
```

4. Verify that Cortex.cpp is installed correctly by getting help information.

```sh {"tag": "devcontainer"}
cd engine/build
./cortex -h
```

5. Everytime a rebuild is needed, just run the commands above using oneliner

```sh
npx -y runme run --filename README.md -t devcontainer -y
```

## Uninstallation

### Windows

1. Open the Windows Control Panel.
2. Navigate to `Add or Remove Programs`.
3. Search for `cortexcpp` and double click to uninstall. (for beta and nightly builds, search for `cortexcpp-beta` and `cortexcpp-nightly` respectively)

### MacOs

Run the uninstaller script:

```bash
sudo sh cortex-uninstall.sh
```

For MacOS, there is a uninstaller script comes with the binary and added to the `/usr/local/bin/` directory. The script is named `cortex-uninstall.sh` for stable builds, `cortex-beta-uninstall.sh` for beta builds and `cortex-nightly-uninstall.sh` for nightly builds.

### Linux

```bash
sudo apt remove cortexcpp
```

## Contact Support

- For support, please file a [GitHub ticket](https://github.com/janhq/cortex.cpp/issues/new/choose).
- For questions, join our Discord [here](https://discord.gg/FTk2MvZwJH).
- For long-form inquiries, please email [hello@jan.ai](mailto:hello@jan.ai).
