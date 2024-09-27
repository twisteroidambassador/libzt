# libzt fork for Python

This is my personal fork of `libzt`,
which focuses pretty much exclusively on the Python bindings.
Other bindings may not build or work correctly.

This is developed and tested on Python 3.11 on Linux.
Other Python versions / OSes are not guaranteed to work.

## Notable changes from upstream

- Fixed the build process
- Implemented many methods on the socket object (IPv6, `sendto`/`recvfrom`, etc.) and the select call
- Made the event callback return everything, and made it possible to read the included information from Python
- Added DNS push information to the callback
- Enabled loopback on included lwIP
- Many more small fixes

## Download prebuilt wheels

Check the [Releases](https://github.com/twisteroidambassador/libzt/releases) section.
Sometimes, the [Actions](https://github.com/twisteroidambassador/libzt/actions) section may have newer builds.

## How to build

- Install `cmake` and `swig`.
- Prepare a Python virtualenv and install `poetry` in it.
- Navigate to `pkg/pypi`.
- If not from a freshly cloned repository, run `clean.sh` to delete any prior artifacts.
- Run `poetry build`.

You should have a `.whl` file inside `pkg/pypi/dist`.
(Ignore the `.tar.gz` file in the same directory. It is empty.)

# Remarks

I started [just wanting to use the Python bindings on newer Python versions.](https://twisteroidambassador.github.io/2024/08/15/build-libzt-python-bindings.html).
As I dug into it, I found out that using `libzt` is somewhat tricky.

`libzt` uses `lwIP` for its userspace networking, and `lwIP` is not thread safe.
Technically, you're not supposed to receive on one thread and send on another with the same socket!
This makes duplex traffic difficult to implement unless you use `select`.

Many other things in `libzt` are not thread safe either, like the implementation of `zts_errno`.
Fortunately it seems possible to just use the native `errno`, since functions just sets both.


## License

My modifications found in this repository are released under The GNU General Public License v3.0.


Original README contents follows.

---

<div align="center">

<h1>ZeroTier Sockets</h1>
Part of the ZeroTier SDK
<img alt="" src="https://i.imgur.com/BwSHwE3.png" class="doxyhidden"> </img>

P2P cross-platform encrypted sockets library using ZeroTier

<br>

<a href="./examples">Examples</a> |
<a href="https://docs.zerotier.com/sockets/tutorial.html">Docs</a> |
<a href="https://github.com/zerotier/libzt/issues">Report an issue</a>

<a href="https://www.twitter.com/zerotier"><img alt="@zerotier" src="https://img.shields.io/twitter/follow/zerotier?style=social"/></a>
<a href="https://old.reddit.com/r/zerotier"><img alt="r/zerotier" src="https://img.shields.io/reddit/subreddit-subscribers/zerotier?style=social"/></a>

<img alt="latest libzt version" src="https://img.shields.io/github/v/tag/zerotier/libzt?label=latest"/></a>
<a href="https://github.com/zerotier/libzt/commits/main"><img alt="Last Commit" src="https://img.shields.io/github/last-commit/zerotier/libzt"/></a>
<a href="https://github.com/zerotier/libzt/actions"><img alt="Build Status (master branch)" src="https://img.shields.io/github/actions/workflow/status/zerotier/libzt/selftest.yml?branch=main"/></a>
</div>

| Language/Platform | Install | Version | Example |
|:----------|:---------|:---|:---|
| C/C++  | [Build from source](#build-from-source) | <img alt="version" src="https://img.shields.io/github/v/tag/zerotier/libzt?label="/></a>| [C/C++](./examples/c)  |
| C#  | `Install-Package ZeroTier.Sockets` |<a href="https://www.nuget.org/packages/ZeroTier.Sockets/"><img src="https://img.shields.io/github/v/tag/zerotier/libzt?label=NuGet"/></a> |[C#](./examples/csharp)  |
| Python  | `pip install libzt`|<a href="https://pypi.org/project/libzt/"><img src="https://img.shields.io/pypi/v/libzt?label=PyPI"/></a> |[Python](./examples/python)  |
| Rust  | See: [crates.io/crates/libzt](https://crates.io/crates/libzt) | <img alt="version" src="https://img.shields.io/crates/v/libzt?color=blue"/>|[Rust](./examples/rust)  |
| Java  | Install JDK, then: `./build.sh host-jar` |<img src="https://img.shields.io/github/v/tag/zerotier/libzt?label="/> |[Java](./examples/java)  |
| Linux  | `brew install zerotier/tap/libzt` | <img alt="version" src="https://img.shields.io/github/v/tag/zerotier/libzt?label="/></a>| [C/C++](./examples/c)  |
| macOS  | `brew install zerotier/tap/libzt`| <img alt="version" src="https://img.shields.io/github/v/tag/zerotier/libzt?label=Homebrew"/></a>| [C/C++](./examples/c)  |
| iOS / iPadOS  | `./build.sh iphoneos-framework` | <img src="https://img.shields.io/github/v/tag/zerotier/libzt?label="/>| [Objective-C](./attic/objective-c), [Swift](./attic/swift)  |
| Android  |`./build.sh android-aar` | <img src="https://img.shields.io/github/v/tag/zerotier/libzt?label="/> | [Java](./examples/java)  |

<br>

<div align="left">

```
#include "ZeroTierSockets.h"

int main()
{
    zts_node_start();
    zts_net_join(net_id);
    int fd = zts_bsd_socket(ZTS_AF_INET, ZTS_SOCK_STREAM, 0);
    zts_bsd_connect(fd, ...);
    ...
}
```

# Build from source (optional)

```
git submodule update --init
```

This project uses [CMake](https://cmake.org/download/) as a build system generator. The scripts `build.*` simplify building and packaging for various targets. There are many targets and configurations not mentioned here.

|Platform| Build instructions | Notes |
|:---|:---|:---|
|Linux | `./build.sh host "release"`| [build.sh](./build.sh) |
|macOS | `./build.sh host "release"`| [build.sh](./build.sh) |
|Windows | `. .\build.ps1; Build-Host -BuildType "Release" -Arch "x64"` | [build.ps1](./build.ps1), *Requires [PowerShell](https://github.com/powershell/powershell)*|

 Using the `host` keyword will automatically detect the current machine type and build standard libzt for use in C/C++ (no additional language bindings.) See `./build.sh list` for additional target options. `libzt` depends on [cURL](https://github.com/curl/curl) for the optional portion of the API that interfaces with our hosted web offering ([my.zerotier.com](my.zerotier.com)). If you do not need this functionality you can omit it by passing `-DZTS_DISABLE_CENTRAL_API=1` to CMake.

Example output:

```
~/libzt/dist/macos-x64-host-release
├── bin
│   ├── client
│   ├── server
│   └── ...
└── lib
    ├── libzt.a
    └── libzt.dylib
```

Important directories:

|Directory| Purpose|
|:---|:---|
|`dist`| Contains finished targets (libraries, binaries, packages, etc.)|
|`cache`| Contains build system caches that can safely be deleted after use.|
|`pkg`| Contains project, script and spec files to generate packages.|

# Self-hosting

If you'd like to use your own infrastructure we make it easy to [set up your own controllers and roots](https://docs.zerotier.com/self-hosting/introduction).

# Help

 - Documentation: [docs.zerotier.com](https://docs.zerotier.com/sockets/tutorial.html)
 - Examples: [examples/](./examples)
 - Bug reports: [Open a github issue](https://github.com/zerotier/libzt/issues).
 - General ZeroTier troubleshooting: [Knowledgebase](https://zerotier.atlassian.net/wiki/spaces/SD/overview).
 - Chat with us: [discuss.zerotier.com](https://discuss.zerotier.com)

# Licensing

ZeroTier and the ZeroTier SDK (libzt and libztcore) are licensed under the [BSL version 1.1](./LICENSE.txt). ZeroTier is free to use internally in businesses and academic institutions and for non-commercial purposes. Certain types of commercial use such as building closed-source apps and devices based on ZeroTier or offering ZeroTier network controllers and network management as a SaaS service require a commercial license. A small amount of third party code is also included in ZeroTier and is not subject to our BSL license. See [AUTHORS.md](ext/ZeroTierOne/AUTHORS.md) for a list of third party code, where it is included, and the licenses that apply to it. All of the third party code in ZeroTier is liberally licensed (MIT, BSD, Apache, public domain, etc.). If you want a commercial license to use the ZeroTier SDK in your product contact us directly via [contact@zerotier.com](mailto:contact@zerotier.com)
