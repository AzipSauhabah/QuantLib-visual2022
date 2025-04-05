# QuantLib: Building with Visual Studio 2022 and MSVC 2017

![QuantLib Logo](path/to/your/logo.png)

[![Download](https://img.shields.io/github/v/release/yourusername/QuantLib-visual2022?label=Download&sort=semver)](https://github.com/yourusername/QuantLib-visual2022/releases/latest)
[![License](https://img.shields.io/badge/License-BSD--3--Clause-blue.svg)](https://github.com/yourusername/QuantLib-visual2022/blob/main/LICENSE)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1440997.svg)](https://doi.org/10.5281/zenodo.1440997)

QuantLib is a free/open-source library for quantitative finance, providing a comprehensive framework for modeling, trading, and risk management in real-life.

## Requirements

Before building QuantLib, ensure you have the following installed:

- **Visual Studio 2022**: Install the Desktop development with C++ workload, including the MSVC v143 – VS 2022 C++ x64/x86 build tools.
- **Boost Libraries**: QuantLib requires Boost 1.48 or later.
- **CMake**: A minimum version of 3.15.0 is required for configuring the build process.

## Installation Steps

1. **Install Visual Studio 2022**:
   - Download and install Visual Studio 2022 from the [official website](https://visualstudio.microsoft.com/).
   - During installation, select the "Desktop development with C++" workload.
   - Ensure the following optional components are installed:
     - Windows 10 SDK (10.0)
     - C++ CMake tools for Windows
     - MSVC v143 – VS 2022 C++ x64/x86 build tools
     - C++ Modules for v143 build tools (x64/x86)

2. **Install Boost Libraries**:
   - Download the Boost installer matching your MSVC version from the [Boost Binaries](https://sourceforge.net/projects/boost/files/boost-binaries/) page. For Visual Studio 2022 with MSVC v143, download `boost_1_81_0-msvc-14.3-64.exe`.
   - Run the installer and note the installation directory (e.g., `C:\local\boost_1_81_0`).

3. **Install CMake**:
   - Download and install CMake from the [CMake Download Page](https://cmake.org/download/).

4. **Obtain QuantLib Source Code**:
   - Clone the QuantLib repository from GitHub:
     ```bash
     git clone https://github.com/lballabio/QuantLib.git
     ```
   - Alternatively, download the latest release from the [QuantLib Download Page](https://www.quantlib.org/download.shtml).

5. **Configure Boost for QuantLib**:
   - Open the "Developer Command Prompt for VS 2022" as an administrator.
   - Navigate to the Boost installation directory:
     ```bash
     cd C:\local\boost_1_81_0
     ```
   - Bootstrap Boost to prepare it for building:
     ```bash
     .\bootstrap.bat
     ```
   - Build Boost with the MSVC v143 toolset:
     ```bash
     .\b2 toolset=msvc-14.3 address-model=64 --build-type=complete stage
     ```

6. **Modify QuantLib Source Code**:
   - In the QuantLib source code, adjust the `#include` directives:
     - Remove the `ql/` prefix where necessary to ensure correct path resolution.

7. **first technique to build the solution: Configure QuantLib with CMake**:
   - Create a build directory within the QuantLib source directory:
     ```bash
     mkdir build
     cd build
     ```
   - Run CMake to configure the project, specifying the generator for Visual Studio 2022:
     ```bash
     cmake -G "Visual Studio 17 2022" ..
     ```
   - Ensure that CMake detects the correct Boost installation. If not, specify the Boost root directory:
     ```bash
     cmake -G "Visual Studio 17 2022" -DBOOST_ROOT="C:/local/boost_1_81_0" ..
     ```

8. **Other Technique to build the solution if Cmake failed to cpompile: Build QuantLib**:
   - Open the generated `QuantLib.sln` solution file in Visual Studio 2022.
   - Set the build configuration to "Release" and the target platform to "x64".
   - Build the solution by selecting "Build" > "Build Solution" from the menu.

9. **Set Up QuantLib in Your Projects**:
   - In your Visual Studio projects that use QuantLib:
     - Add the QuantLib include directory (e.g., `C:\QuantLib\include`) to the "Include Directories" in the project properties.
     - Add the QuantLib library directory (e.g., `C:\QuantLib\lib`) to the "Library Directories".
     - Link against the appropriate QuantLib library (e.g., `QuantLib-vc143-mt.lib`).

## Additional Notes

- Ensure that both QuantLib and your projects are built using the same configuration (Release/Debug) and platform (x64/x86) to avoid compatibility issues.
- For more detailed instructions and troubleshooting, refer to [Benjamin Whiteside's guide](https://benjaminwhiteside.com/2023/09/26/building-quantlib-in-vs2022-64-bit/).

By following these steps, you should be able to successfully build and integrate QuantLib into your projects.
