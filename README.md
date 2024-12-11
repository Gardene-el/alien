# 🌐 本地化中文说明 
### 责任和版权 Responsibility and Copyright
- 参照下列Credit信息，ALIEN项目由[Christian Heinemann](mailto:heinemann.christian@gmail.com)发起、主要开发并维护。According to the following credit information, the ALIEN project was initiated, mainly developed, and maintained by [Christian Heinemann](mailto:heinemann.christian@gmail.com).
- 参照下列License信息，ALIEN 项目使用 [BSD 3-Clause](LICENSE) 许可证授权。According to the following license information, the ALIEN project is licensed under the [BSD 3-Clause](LICENSE) license.
- 该[ALIEN项目分叉](https://github.com/Gardene-el/alien/tree/chinese-lang)是ALIEN项目中文本地化的临时解决方案。在ALIEN项目具备中文本地化方案之后，该项目分叉将不再维护。This [ALIEN project branch](https://github.com/Gardene-el/alien/tree/chinese-lang) is a temporary solution for the Chinese localization of the ALIEN project. Once the ALIEN project has an official Chinese localization, this branch will no longer be maintained.
- 该[ALIEN项目分叉](https://github.com/Gardene-el/alien/tree/chinese-lang)中的中文翻译内容由[Garden_eel](https://github.com/Gardene-el)翻译与维护，翻译者的署名权属于Garden_eel，中文翻译内容版权归属于ALIEN项目，并遵循ALIEN项目的 [BSD 3-Clause](LICENSE) 许可证。- The Chinese translation content in this [ALIEN project branch](https://github.com/Gardene-el/alien/tree/chinese-lang) is translated and maintained by [Garden_eel](https://github.com/Gardene-el). The translation credit belongs to Garden_eel, and the copyright of the Chinese translation content belongs to the ALIEN project and follows the [BSD 3-Clause](LICENSE) license of the ALIEN project.
- Garden_eel不具备彻底校对和始终说明与维护该中文本地化方案的绝对责任。Garden_eel does not have the responsibility to thoroughly proofread and continuously maintain this Chinese localization.
- 中文翻译内容的字体额外使用了[得意黑](https://github.com/atelier-anchor/smiley-sans)，下列Credit信息提及的字体仍有使用。The Chinese translation content additionally uses the [Smiley Sans](https://github.com/atelier-anchor/smiley-sans) font, while the fonts mentioned in the following credit information are still in use.

### 翻译风格说明
- 该中文本地化方案的翻译风格追求简单易懂，所翻译的词汇与词汇关系无法与原文严格对照，例如“Simulation（模拟（名词））”被翻译成“模拟器（simulator）”和“世界（world）”，“Mutation（突变体）”翻译成“种群（population,group,species）”。因此，如想知道中文词汇严格对应的具体英文，请联系Garden_eel。The translation style of this Chinese localization scheme aims for simplicity and clarity. The translated terms and their relationships may not strictly correspond to the original text. For example, "Simulation(模拟（名词）)"  is translated as "模拟器(Simulator)"  and "世界（world）" , and "Mutation(突变体)"  is translated as "种群(population,group,species)". Therefore, if you want to know the exact English equivalents of the Chinese terms, please contact Garden_eel.
- 该中文本地化方案在ALIEN项目罕见概念上创造了对应的专有名词，目前仅包括细胞的功能名称。具体是将细胞的功能名称翻译为“XX器”来使其作为等格且精确描述的中文名词，例如“Muscle（肌肉）”翻译成“运动器”，“Transmitter”翻译成“传输器”。如有对额外词汇的专有名词化的建议或者对现有专有词汇的不理解、不满或批评，请联系Garden_eel以反馈。This Chinese localization scheme creates corresponding proprietary terms for rare concepts in the ALIEN project, currently including only the names of cell functions. Specifically, the names of cell functions are translated as "XX器" to make them equivalent and precisely descriptive Chinese nouns. For example, "Muscle(肌肉)"  is translated as "运动器(movement device/organ)" , and "Transmitter" is translated as "传输器(transmission device/organ)". If you have suggestions for additional proprietary terms or do not understand, are dissatisfied with, or have criticisms of the existing proprietary terms, please contact Garden_eel for feedback.
- 在“入门手册”中，翻译内容是借助AI翻译完成的。如果有任何不理解、错误或纰漏，请联系Garden_eel以指出和修正。In the "Getting Started Guide," the translation content is completed with the help of AI translation. If there are any misunderstandings, errors, or omissions, please contact Garden_eel to point them out and correct them.

### 中文ALIEN安装说明
- 完成品的中文ALIEN软件压缩包会在QQ群926751398中分享。在我理解github的Release机制后，将同样在github中的Release中分享中文ALIEN软件压缩包。这个类型的分享仅支持windows系统。The finished Chinese ALIEN software package will be shared in the QQ group 926751398. After I understand the Release in Github, the Chinese ALIEN software package will also be shared in the GitHub Releases. This type of sharing only supports Windows systems.
- 如果想要自己构建中文ALIEN软件，请参详下列的How to build the sources，并在Getting the sources步骤中将git地址从`https://github.com/chrxh/alien.git`变更为`https://github.com/Gardene-el/alien/tree/chinese-lang`。If you want to build the Chinese ALIEN software yourself, please refer to the following "How to build the sources" and change the git address in the "Getting the sources" step from `https://github.com/chrxh/alien.git` to `https://github.com/Gardene-el/alien/tree/chinese-lang`.
- 如果ALIEN项目已更新但该项目分叉并未及时更新，请发issue以通知Garden_eel该项目分叉已过时。If the ALIEN project has been updated but this branch has not been updated in time, please open an issue to notify Garden_eel that this branch is outdated.

<h1 align="center">
<a href="https://alien-project.org" target="_blank">ALIEN - Explore worlds of artificial life</a>
</h1>

![Preview](https://github.com/user-attachments/assets/ee578848-7dd7-458d-873f-89662a7c15f0)

<p>
<b><i>A</i></b>rtificial <b><i>LI</i></b>fe <b><i>EN</i></b>vironment <b>(ALIEN)</b> is an artificial life simulation tool based on a specialized 2D particle engine in CUDA for soft bodies and fluids. Each simulated body consists of a network of particles that can be upgraded with higher-level functions, ranging from pure information processing capabilities to physical equipment (such as sensors, muscles, weapons, constructors, etc.) whose executions are orchestrated by neural networks. The bodies can be thought of as agents or digital organisms operating in a common environment. Their blueprints can be stored in genomes and passed on to offspring.
</p>
<p>
The simulation code is written entirely in CUDA and optimized for large-scale real-time simulations with millions of particles.
The development is driven by the desire to better understand the conditions for (pre-)biotic evolution and the growing complexity of biological systems.
An important goal is to make the simulator user-friendly through a modern user interface, visually appealing rendering and a playful approach. 
</p>

<p>
  Please join our <a href="https://discord.gg/7bjyZdXXQ2" target="_blank">Discord server</a> as a place for discussions, new developments and feedback around ALIEN and artificial life in general.
</p>

<p>
  Demo video: <a href="https://youtu.be/qwbMGPkoJmg" target="_blank">Emerging Ecosystems | Winner of the ALIFE 2024 Virtual Creatures Competition</a>
</p>

# ⚡ Main features
### Physics and graphics engine
- Particles for simulating soft and rigid body mechanics, fluids, heat dissipation, damage, adhesion etc.
- Real-time user interactions with running simulations
- Simulation runs entirely on GPU via CUDA
- Rendering and post-processing via OpenGL using CUDA-OpenGL interoperability

https://user-images.githubusercontent.com/73127001/229868357-131fa71f-d03d-45db-ac76-9d192f5464af.mp4

### Artificial Life engine extensions
- Multi-cellular organisms are simulated as particle networks
- Genetic system and cell by cell construction of offspring
- Neural networks for controlling higher-level functions (e.g. sensors and muscles)
- Various colors may be used to customize cell types according to own specifications
- Support for spatially varying simulation parameters

https://user-images.githubusercontent.com/73127001/229569056-0db6562b-0147-43c8-a977-5f12c1b6277b.mp4

### Extensive editing tools
- Graph editor for manipulating every particle and connection
- Freehand and geometric drawing tools
- Genetic editor for designing customized organisms
- Mass-operations and (up/down) scaling functions

### Networking
- Built-in simulation browser
- Download and upload simulation files
- Upvote simulations by giving stars

# ❓ But for what is this useful
- A first attempt to answer: Feed your curiosity by watching evolution at work! As soon as self-replicating machines come into play and mutations are turned on, the simulation itself does everything.
- Perhaps the most honest answer: Fun! It is almost like a game with a pretty fast and realistic physics engine. You can make hundreds of thousands of machines accelerate and destroy with the mouse cursor. It feels like playing god in your own universe with your own rules. Different render styles and a visual editor offer fascinating insights into the events. There are a lot of videos on the [YouTube channel](https://youtube.com/channel/UCtotfE3yvG0wwAZ4bDfPGYw) for illustration.
- A more academic answer: A tool to tackle fundamental questions of how complexity or life-like structure may arise from simple components. How do entire ecosystems adapt to environmental changes and find a new equilibrium? How to find conditions that allow open-ended evolution?
- A tool for generative art: Evolution is a creative force that leads to ever new forms and behaviors.

# 📘 Documentation
A documentation for the previous major version, which introduces the reader to the simulator with tutorial-like articles, can be found at [alien-project.gitbook.io/docs](https://alien-project.gitbook.io/docs). Please notice that many of the information therein are no longer up to date.
The latest version includes a brief documentation and user guidance in the program itself via help windows and tooltips.

Further information and artwork:
* [Website](https://alien-project.org)
* [YouTube](https://youtube.com/channel/UCtotfE3yvG0wwAZ4bDfPGYw)
* [Twitter](https://twitter.com/chrx_h)
* [Reddit](https://www.reddit.com/r/AlienProject)
* [Discord](https://discord.gg/7bjyZdXXQ2)

# 🖥️ Minimal system requirements
An Nvidia graphics card with compute capability 6.0 or higher is needed. Please check [https://en.wikipedia.org/wiki/CUDA#GPUs_supported](https://en.wikipedia.org/wiki/CUDA#GPUs_supported).

# 💽 Installer
Installer for Windows: [alien-installer.msi](https://alien-project.org/media/files/alien-installer.msi) (updated: 2024-11-24)

In the case that the program crashes for an unknown reason, please refer to the troubleshooting section below.

# 🔨 How to build the sources
The build process is mostly automated using the cross-platform CMake build system and the vcpkg package manager, which is included as a Git submodule.

### Getting the sources
To obtain the sources, please open a command prompt in a suitable directory (which should not contain whitespace characters) and enter the following command:
```
git clone --recursive https://github.com/chrxh/alien.git
```
Note: The `--recursive` parameter is necessary to check out the vcpkg submodule as well. Besides that, submodules are not normally updated by the standard `git pull` command. Instead, you need to write `git pull --recurse-submodules`.

### Build instructions
Prerequisites: [CUDA Toolkit 11.2+](https://developer.nvidia.com/cuda-downloads) and a toolchain for CMake (e.g. GCC 9.x+ or [MSVC v142+](https://visualstudio.microsoft.com/vs/)).

Build steps:
```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j8
```
If everything goes well, the ALIEN executable can be found under the build directory in `./alien` or `.\Release\alien.exe` depending on the used toolchain and platform.
It is important to start ALIEN directly from the build folder, otherwise it will not find the resource folder.

There are reported build issues with (updated: 2024-06-22)
* GCC 12+ (version 11 should work)
* Visual Studio 17.10 (version 17.9 should work)
* CUDA 12.5 (version 12.4 should work)

# ⌨️ Command-line interface

This repository also contains a CLI for ALIEN. It can be used to run simulations without using a GUI. This is useful for performance measurements as well as for automatic execution and evaluation of simulations for different parameters.
The CLI takes the simulation file, along with its parameters and the number of time steps, as input. It then provides the resulting simulation file and the statistics (as a CSV file) as output.
For example,
```
.\cli.exe -i example.sim -o output.sim -t 1000
```
runs the simulation file `example.sim` for 1000 time steps.

# 🔎 Troubleshooting

Please make sure that:
1) You have an NVIDIA graphics card with compute capability 6.0 or higher (for example GeForce 10 series).
2) You have the latest NVIDIA graphics driver installed.
3) The name of the installation directory (including the parent directories) should not contain non-English characters. If this is not fulfilled, please re-install ALIEN to a suitable directory. Do not move the files manually. If you use Windows, make also sure that you install ALIEN with a Windows user that contains no non-English characters. If this is not the case, a new Windows user could be created to solve this problem.
4) ALIEN needs write access to its own directory. This should normally be the case.
5) If you have multiple graphics cards, please check that your primary monitor is connected to the CUDA-powered card. ALIEN uses the same graphics card for computation as well as rendering and chooses the one with the highest compute capability.
6) If you possess both integrated and dedicated graphics cards, please ensure that the alien-executable is configured to use your high-performance graphics card. On Windows you need to access the 'Graphics settings,' add 'alien.exe' to the list, click 'Options,' and choose 'High performance'.

If these conditions are not met, ALIEN may crash unexpectedly.
If the conditions are met and the error still occurs, please start ALIEN with the command line parameter `-d`, try to reproduce the error and then create a GitHub issue on https://github.com/chrxh/alien/issues where the log.txt is attached.

# 🌌 Screenshots
#### Different plant-like populations around a radiation source
![Screenshot1](https://user-images.githubusercontent.com/73127001/229311601-839649a6-c60c-4723-99b3-26086e3e4340.jpg)

<h1 align="center"></h1>

#### Close-up of different types of organisms so that their cell networks can be seen
![Screenshot2](https://user-images.githubusercontent.com/73127001/229311604-3ee433d4-7dd8-46e2-b3e6-489eaffbda7b.jpg)

<h1 align="center"></h1>

#### Different swarms attacking an ecosystem
![Screenshot3](https://user-images.githubusercontent.com/73127001/229311606-2f590bfb-71a8-4f71-8ff7-7013de9d7496.jpg)

<h1 align="center"></h1>

#### Genome editor
![Screenshot3b](https://user-images.githubusercontent.com/73127001/229313813-c9ce70e2-d61f-4745-b64f-ada0b6758901.jpg)

# 🧩 Contributing to the project
Contributions to the project are very welcome. The most convenient way is to communicate via [GitHub Issues](https://github.com/chrxh/alien/issues), [Pull requests](https://github.com/chrxh/alien/pulls) or the [Discussion forum](https://github.com/chrxh/alien/discussions) depending on the subject. For example, it could be
- Providing new content (simulation or genome files)
- Producing or sharing media files
- Reporting of bugs, wanted features, questions or feedback via GitHub Issues or in the Discussion forum.
- Pull requests for bug fixes, code cleanings, optimizations or minor tweaks. If you want to implement new features, refactorings or other major changes, please use the [Discussion forum](https://github.com/chrxh/alien/discussions) for consultation and coordination in advance.
- Extensions or corrections to the [alien-docs](https://alien-project.gitbook.io/docs). It has its [own repository](https://github.com/chrxh/alien-docs).

A short architectural overview of the source code can be found in the [documentation](https://alien-project.gitbook.io/docs/under-the-hood).

# 💎 Credits and dependencies

ALIEN has been initiated, mainly developed and maintained by [Christian Heinemann](mailto:heinemann.christian@gmail.com). Thanks to all the others who contributed to this repository:
- [tlemo](https://github.com/tlemo)
- [Gardene-el](https://github.com/Gardene-el)
- [mpersano](https://github.com/mpersano)
- [dguerizec](https://github.com/dguerizec)
- [Will Allen](https://github.com/willjallen)
- [TheBarret](https://github.com/TheBarret)

The following external libraries are used:
- [CUDA Toolkit](https://developer.nvidia.com/cuda-toolkit)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [ImPlot](https://github.com/epezent/implot)
- [ImFileDialog](https://github.com/dfranx/ImFileDialog)
- [boost](https://www.boost.org)
- [Glad](https://glad.dav1d.de)
- [GLFW](https://www.glfw.org)
- [glew](https://github.com/nigels-com/glew)
- [stb](https://github.com/nothings/stb)
- [cereal](https://github.com/USCiLab/cereal)
- [zlib](https://www.zlib.net)
- [zstr](https://github.com/mateidavid/zstr)
- [OpenSSL](https://github.com/openssl/openssl)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [googletest](https://github.com/google/googletest)
- [vcpkg](https://vcpkg.io/en/index.html)
- [WinReg](https://github.com/GiovanniDicanio/WinReg)
- [CLI11](https://github.com/CLIUtils/CLI11)

Free icons and icon font:
  - [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders)
  - [Iconduck](https://iconduck.com) (Noto Emoji by Google, [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt))
  - [Iconfinder](https://www.iconfinder.com) (Bogdan Rosu Creative, [CC BY 4.0](https://creativecommons.org/licenses/by/4.0))
  - [People icons created by Freepik - Flaticon](https://www.flaticon.com/free-icons/people) ([Flaticon license](https://media.flaticon.com/license/license.pdf))

# 🧾 License
ALIEN is licensed under the [BSD 3-Clause](LICENSE) license.

<a href="https://hellogithub.com/repository/d53e3c352f294f72a1bfd8f48ac0f866" target="_blank"><img src="https://abroad.hellogithub.com/v1/widgets/recommend.svg?rid=d53e3c352f294f72a1bfd8f48ac0f866&claim_uid=dKUYgLps8t45BW7&theme=small" alt="Featured｜HelloGitHub" /></a>
