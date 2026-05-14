# bridge

## Build (Linux)

### Prerequisites (Debian / Ubuntu)

```bash
sudo apt install build-essential cmake ninja-build \
  qt6-base-dev qt6-base-dev-tools \
  libprotobuf-dev protobuf-compiler \
  libgrpc++-dev protobuf-compiler-grpc
```

For Fedora / RHEL:

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel \
  protobuf-devel protobuf-compiler \
  grpc-devel grpc-plugins
```

### Configure and build

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Output: `build/bridge`. Runs directly — no extra DLL/runtime deployment needed.

## Build (Windows / MSVC)

### Prerequisites

- Visual Studio 2022 with MSVC toolset **14.44.35207** installed
- Qt 6 with the **MSVC 2022 64-bit** kit at `C:\Qt\6.11.1\msvc2022_64`
- vcpkg at `G:\vcpkg` with `protobuf` and `grpc` installed for the `x64-windows` triplet:
  ```powershell
  git clone https://github.com/microsoft/vcpkg.git G:\vcpkg
  G:\vcpkg\bootstrap-vcpkg.bat
  G:\vcpkg\vcpkg.exe install protobuf grpc --triplet x64-windows
  ```

### Configure and build

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -T version=14.44.35207 `
  -DCMAKE_TOOLCHAIN_FILE=G:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64
cmake --build . --config Release
```

Output: `build\Release\bridge.exe`.

### Deploy runtime DLLs

The freshly built `bridge.exe` won't run yet — it needs Qt and vcpkg DLLs next to it.

```powershell
# Qt DLLs + plugins (Qt6Core/Gui/Network/Widgets, platform plugin, image formats, etc.)
C:\Qt\6.11.1\msvc2022_64\bin\windeployqt.exe --release --no-translations build\Release\bridge.exe

# vcpkg runtime DLLs (protobuf, abseil, re2, zlib, c-ares, openssl)
foreach ($d in 'libprotobuf','abseil_dll','re2','z','cares','libssl-3-x64','libcrypto-3-x64') {
  Copy-Item "G:\vcpkg\installed\x64-windows\bin\$d.dll" build\Release\
}
```

### Notes

- The MSVC toolset version passed to `-T version=` must match the version vcpkg used to build grpc/abseil. If you see unresolved external symbols like `__std_find_last_of_trivial_pos_1`, `__std_remove_8`, or `__std_search_1` at link time, check `G:\vcpkg\buildtrees\grpc\config-x64-windows-rel-CMakeCache.txt.log` for the MSVC version vcpkg used and update `-T version=` to match.
- Building gRPC from source via vcpkg needs roughly 30 GB of free disk space at peak.
