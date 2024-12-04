#include "GettingStartedWindow.h"

#include <imgui.h>
#include <Fonts/IconsFontAwesome5.h>

#include "StyleRepository.h"
#include "AlienImGui.h"

#ifdef _WIN32
#include <windows.h>
#endif

void GettingStartedWindow::initIntern()
{
    _showAfterStartup = _on;
}


GettingStartedWindow::GettingStartedWindow()
    : AlienWindow("入门手册", "windows.getting started", true)
{}

void GettingStartedWindow::shutdownIntern()
{
    _on = _showAfterStartup;
}

void GettingStartedWindow::processIntern()
{
    drawTitle();

    if (ImGui::BeginChild("##", ImVec2(0, ImGui::GetContentRegionAvail().y - scale(50.0f)), false)) {
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);

        /**
         * INTRO
         */
        drawHeading1("介绍");

        ImGui::Text("ALIEN 是一个基于 CUDA 驱动的 2D 粒子引擎的人工生命和物理模拟器，可以模拟软体和流体。");
        ImGui::Text(
            "每个粒子都可以配备更高级的功能，包括传感器、肌肉、神经元、构造器等，这些功能允许模拟生物细胞或机器人组件的某些功能。多细胞生物被模拟为通过其连接交换能量和信息的粒子网络。引擎包含一个遗传系统，能够在基因组中编码生物体的蓝图，这些基因组存储在单个细胞中。模拟器能够模拟由不同种群组成的整个生态系统，其中每个对象都是由具有特定功能的相互作用粒子组成的（无论它是植物、草食动物、肉食动物、病毒、环境结构等）。");

        /**
         * FIRST STEPS
         */
        drawHeading1("第一步");

        ImGui::Text("了解 ALIEN 模拟器的最简单方法是下载并运行现有的模拟文件。然后，您可以尝试不同的功能并根据自己的需求修改模拟。");
        ImGui::Text(
            "在游戏模拟浏览器中可以找到各种示例，展示引擎的功能，从纯物理示例、自部署结构、自复制器到进化生态系统。如果尚未打开，请在菜单栏中调用网络 " ICON_FA_LONG_ARROW_ALT_RIGHT " 浏览器。"
            "可以方便地从连接的服务器（默认情况下为 alien-project.org）下载和上传模拟文件。"
            "为了将自己的模拟上传到服务器或对其他模拟进行评分，您需要注册一个新用户，这可以在登录对话框中完成。");

        ImGui::Text("不过，开始时，您可以使用已经加载的示例。最初，建议熟悉时间和空间控制窗口。操作应该是直观的，不需要深入的知识。");
        drawItemText("在时间控制窗口中，可以启动或暂停模拟。如果需要，可以调节执行速度。此外，还可以计算和恢复单个时间步长，并拍摄模拟快照，随时可以返回到这些快照，而无需从文件重新加载模拟。");
        drawItemText("空间控制窗口结合了缩放信息和设置，以及缩放功能。对话框中的“缩放内容”选项是一个非常有用的功能。如果激活，可以制作原始世界的周期性空间副本。");
        ImGui::Text("用户在显示模拟的视图中基本上可以以两种模式操作：导航模式和编辑模式。您可以通过点击屏幕左下角的编辑按钮或在菜单中通过编辑器 " ICON_FA_LONG_ARROW_ALT_RIGHT " 激活来切换这两种模式。");
        drawItemText(
            "默认情况下启用导航模式，允许您连续放大（按住左键）和缩小（按住右键）。或者，您也可以使用鼠标滚轮。按住中键并移动鼠标，可以平移世界的可视化部分。");
        drawItemText(
            "在编辑模式下，可以通过按住并移动右键在运行的模拟中推动物体。使用左键可以拖放对象。请尝试一下，这会很有趣！编辑模式还允许您激活许多编辑窗口（模式编辑器、创建器、倍增器、基因组编辑器等），这些窗口的功能可以随着时间的推移进行探索。几乎所有单个粒子的属性都可以被操控。此外，还有大量的编辑功能可用。");

        ImGui::Text("为了能够实验现有的模拟，了解和更改模拟参数非常重要。这可以在“模拟参数”窗口中完成。例如，可以增加辐射强度或调整摩擦力。每个参数旁边的工具提示中可以找到对各个参数的解释。");

        ImGui::Text(
            "ALIEN 提供了通过 7 种不同颜色的颜色系统自定义基本实体的可能性。更具体地说，每个细胞都被分配了一个特定的颜色，允许根据细胞的颜色应用不同的模拟参数值。这使得在共享世界中为共存的种群创建特定条件成为可能。例如，类植物生物体可能具有更高的辐射粒子吸收率，因此它们可以从中获取能量。");

        drawHeading2("重要");
        ImGui::Text(
            "在较旧的显卡上或使用高分辨率（例如 4K）时，建议减少渲染的每秒帧数，因为这会显著提高模拟速度（每秒时间步长）。此调整可以在显示设置中进行。");

        /**
         * EXAMPLES
         */
        drawHeading1("示例");
        drawParagraph(
            "ALIEN 附带了许多可以在浏览器窗口中找到的模拟文件。它们适合用于实验程序的某些方面。我们挑选了一些示例以简要概述：");

        drawHeading2("流体、墙壁和软体");
        drawParagraph("有几个纯物理模拟展示了引擎的能力。它们适合测试模拟参数的影响，例如“平滑长度”、“压力”、“粘度”等。");
        drawItemText("流体/带软体的泵");
        drawItemText("演示/永动机");
        drawItemText("演示/暴风雨之夜");

        drawHeading2("进化模拟");
        drawParagraph("通过将更高级的功能附加到粒子网络，可以建模复杂的多细胞生物。它们可以随着时间的推移进化，因为它们受到突变的影响。以下示例包括由自我复制的代理填充的同质和变化的世界。不同的选择压力控制进化。");
        drawItemText("深处/精选结果");
        drawItemText("原始海洋/精选结果");
        drawItemText("v4.8-进化/梯度/精选结果");
        drawItemText("进化沙盒/示例");
        drawItemText("复杂进化测试床/示例");

        drawHeading2("植物-草食动物生态系统");
        drawParagraph("通过根据颜色自定义细胞，可以指定不同类型的生物体。有许多示例展示了两类生物：植物和草食动物。植物能够消耗辐射粒子，而草食动物可以消耗植物。这种简单的关系已经提供了有趣的动态，如以下示例所示。");
        drawItemText("双子世界/示例");
        drawItemText("虫子和花朵/示例");
        drawItemText("自我复制流体/初始设置");

        drawHeading2("蜂群");
        drawParagraph("作为细胞功能，有强大的传感器可用于检测周围特定颜色的浓度。配备这些传感器的生物体可以感知环境，滋养它们的神经网络，并做出相应的反应。");
        drawItemText("蜂群/太空入侵者");
        drawItemText("进化蜂群/示例");

        /**
         * BASIC NOTION
         */
        drawHeading1("基本概念");

        ImGui::Text("通常，在 ALIEN 模拟中，所有对象以及热辐射都是由不同类型的粒子在空旷空间中移动来建模的。以下术语经常使用：");

        ImGui::Spacing();
        drawHeading2("世界");
        ImGui::Text("ALIEN 世界是一个具有周期性边界条件的二维矩形域。空间被建模为一个连续体。");

        ImGui::Spacing();
        drawHeading2("细胞");
        ImGui::Text(
            "细胞是构成一切的基本构建块。它们可以相互连接，可能附着在背景上（用于建模障碍物），具有特殊功能并传输信号。此外，细胞具有各种物理属性，包括");
        drawItemText("空间中的位置");
        drawItemText("速度");
        drawItemText("内部能量（可以解释为其温度）");
        drawItemText("连接的上限");
        drawItemText("生存状态");

        ImGui::Spacing();
        drawHeading2("细胞连接");
        ImGui::Text(
            "细胞连接是两个细胞之间的键。它存储参考距离，并在每一侧存储到可能的进一步细胞连接的参考角度。参考距离和角度在建立连接时计算。一旦实际距离偏离参考距离，在两端施加拉/推力。此外，在角度不匹配的情况下，在两端施加切向力。");

        ImGui::Spacing();
        drawHeading2("细胞功能");
        ImGui::Text("可以为细胞分配一个特殊功能，该功能将在定期时间间隔内执行。以下功能已实现：");
        drawItemText("神经元：它为细胞配备了一个由 8 个神经元组成的小型网络。它处理从连接细胞的信号中获得的输入，并向其他连接细胞提供输出信号。");
        drawItemText("传输器：它将能量分配给其他构造器、传输器或周围的细胞。特别是，它可以用于为活动构造器供电。不需要信号触发。");
        drawItemText("构造器：构造器可以基于内置基因组构建细胞网络。构造是逐个细胞完成的，需要能量。构造器可以通过信号控制或自动激活（默认）。");
        drawItemText("注射器：它可以感染其他构造器细胞以注入其内置基因组。");
        drawItemText("反应神经：一方面，它传输来自连接输入细胞的信号，另一方面，它可以选择性地在特定时间间隔生成信号。");
        drawItemText("攻击者：它通过从其他细胞网络的周围细胞中窃取能量来攻击它们。");
        drawItemText("防御者：当附近的另一个细胞执行攻击时，它会减少攻击强度。");
        drawItemText("运动器（肌肉）：当肌肉细胞被激活时，它可以产生运动、弯曲或改变细胞连接的长度。");
        drawItemText("感知器：如果激活，它会对特定颜色的细胞浓度进行远程扫描。");
        drawItemText("重连器：能够动态创建或破坏与指定颜色的其他细胞的连接。");
        drawItemText("爆炸器：可以通过信号爆炸的细胞。它为周围的物体产生大量的动能。");

        ImGui::Spacing();
        drawHeading2("信号");
        drawParagraph(
            "细胞可以产生包含 8 个值的信号，主要用于控制细胞功能，有时称为通道 #0 到通道 #7。状态定期刷新，特别是在执行细胞功能时。更确切地说，每个细胞功能在定期时间间隔内执行（每 6 个时间步长）。“执行顺序号”指定了这些间隔内的确切时间偏移。");
        drawParagraph("更新信号值的过程如下：首先，所有输入信号（即来自连接细胞的信号，与输入执行号匹配）的值相加。结果的总和然后用作细胞功能的输入，可能会改变这些值。随后，结果用于生成输出信号。");

        ImGui::Spacing();
        drawHeading2("细胞颜色");
        drawParagraph("除了细胞功能外，还可以使用颜色来执行额外的用户定义的细胞自定义。为此，如果需要，大多数模拟参数可以分别为每种颜色调整。因此，不同颜色的细胞可能具有个体属性。");

        ImGui::Spacing();
        drawHeading2("细胞网络");
        drawParagraph("细胞网络是由细胞和细胞连接组成的连接图。因此，网络中的两个细胞通过其他细胞直接或间接连接在一起。细胞网络在物理上代表特定的身体。");

        ImGui::Spacing();
        drawHeading2("基因组");
        drawParagraph("整个细胞网络的蓝图可以存储在基因组中。这些基因组由构造器细胞翻译成真实的细胞网络，并在必要时复制到它们的后代中。此外，注射器细胞能够将其自身的基因组注入其他构造器细胞中，这允许建模病毒行为。");
        drawParagraph("ALIEN 中的基因组可以描述多个细胞网络，这些网络是分层结构的，并且在构建时可能连接在一起。更确切地说，这意味着有一个顶级蓝图描述了某个网络。如果这个网络中有进一步的构造器细胞，它们也可以包含进一步的基因组，这些基因组可以在单独的选项卡中编辑，依此类推。");

        ImGui::Spacing();
        drawHeading2("能量粒子");
        drawParagraph(
            "能量粒子是一种只有能量值、位置和速度的粒子。与细胞不同，它们不能形成网络或执行任何附加功能。能量粒子由细胞作为辐射或在衰变过程中产生，并且可以被吸收。");

        ImGui::Spacing();
        drawHeading2("模式");
        drawParagraph("模式是一组细胞网络和能量粒子。");

        /**
         * SIMULATION PARAMETERS
         */
        drawHeading1("模拟参数");
        drawParagraph(
            "所有与模拟相关的参数都可以在这里调整。默认情况下，参数在整个世界中统一设置。然而，也可以允许某些参数在特殊区域内局部变化。为此，您可以通过点击“+”按钮在模拟参数窗口中创建一个新选项卡。它添加了一个空间（模糊）限定区域，其中可以覆盖全局参数。该区域也通过不同的颜色可见。");
        drawParagraph("除此之外，许多参数也可以根据细胞颜色设置。为此，请点击参数旁边的“+”按钮。当您想定义不同类别的物种时，此自定义非常有用。");
        drawParagraph("有时在滑块中设置精确值很困难。在这种情况下，您可以在按住 CTRL 键的同时点击滑块。这允许您在输入字段中输入精确值并按 ENTER 确认。");
        drawParagraph("一般来说，可以设置以下类型的参数。");
        drawHeading2("渲染");
        drawParagraph(
            "除了背景颜色，您还可以在这里确定细胞的着色。每个细胞都被分配了一个特定的颜色，可以用于自定义，并且默认情况下也用于渲染。然而，在进化模拟中，将突变体着色为不同颜色非常有用。这允许更好地可视化评估多样性、突变率和成功的突变体等。为此，您可以将细胞着色切换为突变 ID。");
        drawHeading2("物理");
        drawParagraph("这些设置中可以修改基本的物理属性。这包括调整辐射强度、各种阈值和运动算法。更改可能对性能产生重大影响，在最坏的情况下，可能导致程序崩溃。");
        drawHeading2("辐射源");
        drawParagraph("可选地，您可以通过打开相应的编辑器来定义辐射源。通常，所有细胞通过发射粒子随着时间的推移失去能量。这些能量粒子在空间中移动，并在某些条件下被其他细胞吸收。当未定义辐射源时，能量粒子在细胞的位置产生，随着时间的推移，能量粒子在空间中更或少均匀分布。对于某些模拟，特别是在建模植物物种时，指定能量粒子应生成的显式源是有益的。这可以在“辐射源”窗口中实现。即使定义了源，细胞仍然会像以前一样失去相同数量的能量。不同之处在于粒子现在在指定的源处生成。能量守恒原理保持不变。");
        drawHeading2("细胞特定参数");
        drawParagraph("这些参数类型在模拟由细胞网络组成的（自我复制）代理时特别重要，超出了纯物理模拟的范围。许多不同的细胞功能依赖于特定参数，可以在这里调整。特别重要的是突变率和攻击功能的参数。后者可以配置不同颜色细胞之间的食物链。例如，在“食物链颜色矩阵”中，可以指定某种颜色的细胞只能消耗某种其他颜色的细胞，而不能消耗自身。");
        drawParagraph("突变率影响修改基础细胞基因组的概率。在调整这些率时，应注意不同类型的突变也有不同的影响。例如，“复制”突变比“神经网络”突变更具侵入性，后者仅调整权重和偏差。此外，应考虑到对于需要长时间自我复制的进化模拟，应避免高突变率。最佳值最好通过实验确定。");


        /**
         * EDITORS
         */
drawHeading1("编辑器");
        drawParagraph(
            "如果您想设计自己的世界、场景或生物体，有许多不同的编辑器可用，这些编辑器部分需要更深入的知识。要打开编辑器，您需要切换到编辑模式（例如，点击左下角的图标）。以下是可能性的简要概述。");
        drawHeading2("拖放");
        drawParagraph(
            "最简单的方法是使用鼠标选择和移动对象。您可以在模拟视图中简单地拖放细胞网络。这在运行模拟期间也有效。当模拟暂停时，您可以按住右键选择一个矩形区域。选择区域会被高亮显示，并且可以通过拖放移动。在鼠标操作期间按住 SHIFT 键，仅移动选定的细胞而不是关联的细胞网络。这可能导致未选择的细胞连接的破坏或形成。");
        drawHeading2("创建器");
        drawParagraph("如果您想创建单个细胞、细胞网络或能量粒子，可以打开“创建器”窗口。在此窗口中，您还可以通过在模拟区域上自由绘制来创建细胞结构。创建的细胞配备了默认值，如果需要，可以稍后修改。");
        drawHeading2("模式编辑器");
        drawParagraph("已经选择的细胞和能量粒子的物理属性，如中心速度、位置、颜色等，可以在“模式编辑器”中方便地更改。此外，选择可以保存、加载、复制和粘贴。");
        drawHeading2("基因组编辑器");
        drawParagraph("在“基因组编辑器”中，可以创建和修改描述细胞网络的基因组。每个选项卡显示基因组的主要部分。它是按顺序构建的细胞序列。如果其中一个细胞是构造器细胞，它包含一个可以在单独选项卡中编辑的附加基因组。");
        drawParagraph("将基因组转化为实际结构有两种选择：");
        drawItemText("您可以使用相应的工具栏按钮生成孢子。孢子是一个包含特定基因组并具有足够能量来创建主要结构的单个构造器细胞。");
        drawItemText(
            "另一种选择是将基因组注入现有生物体中。为此，您必须选择生物体并在编辑器菜单中点击“检查主要基因组”。会打开一个窗口，您可以在其中看到该生物体的现有基因组。然后，您可以通过调用“从编辑器注入”按钮注入自己的基因组。");
        drawHeading2("对象检查");
        drawParagraph("几乎每个粒子的每个属性都可以查看和编辑。为此，可以将特殊的编辑窗口附加到粒子上。为此，您需要选择一个或多个粒子（不要太多）并从编辑器菜单中调用“检查对象”。每个选定的粒子现在都连接到一个窗口。甚至可以在运行模拟期间查看这些编辑窗口。通过这种方式，您可以实时监控单个粒子属性随时间的变化。");
        drawHeading2("批量操作");
        drawParagraph("有各种批量编辑功能可用。一方面，模式编辑器允许更改整个选择的不同物理属性。另一方面，通过“工具”菜单可以打开“批量操作”对话框。在这里，可以全局修改细胞和基因组的颜色、能量值和其他属性。新值将从指定范围内随机选择。细胞网络内的细胞将分配相同的值。");
        drawParagraph("此外，可以通过增加世界的大小并用原始世界的副本填充生成的空白空间来扩展模拟。此功能可以通过在“空间控制”窗口中的调整对话框中设置新大小并激活“缩放内容”来实现。");
        /**
         * 常见问题
         */
        drawHeading1("FAQ 常见问题");

        drawHeading2("简单的自我复制生物体如何工作？");
        drawParagraph("一般来说，ALIEN 中的生物体由细胞网络组成，这些细胞通过信号相互通信协同工作。");
        drawParagraph(
            "一个简单的生物体首先需要一个包含其基因组并负责自我复制的构造器细胞。构造器细胞会自动触发，并在有足够能量时生成后代的细胞网络，按基因组描述逐个细胞构建。后代的基因组也会被复制并放置在后代的构造器细胞中。");
        drawParagraph(
            "自我复制需要能量，必须通过某种方式获取。一方面，能量可以通过吸收飞行的能量粒子获得。这可以是类植物物种的主要能量来源。另一方面，可以利用攻击细胞。它们可以通过从其他生物体的细胞中窃取能量来攻击它们。如果攻击细胞是生物体的一部分，它必须通过信号显式触发。例如，这个信号可能来自另一个配备神经网络的细胞。攻击细胞获得的能量会分配给附近的构造器或发射器细胞。");
        drawParagraph("为了进行运动，生物体需要肌肉细胞。这些细胞也由信号控制。肌肉细胞可以以各种模式工作：它们可以弯曲、收缩/扩展或产生冲动。");
        drawParagraph("为了感知环境，可以使用传感器细胞。当这种细胞被信号触发时，它会提供关于特定颜色细胞浓度相对位置的信息，这些信息可以进一步处理，例如由配备神经网络的细胞处理。");

        drawHeading2("为什么辐射源不产生能量粒子？");
        drawParagraph("能量守恒原理在模拟中成立，这意味着能量粒子不能凭空自发产生。这个原理在确保长期模拟的稳定性方面起着至关重要的作用。如果定义了辐射源，它只会在某个细胞失去能量时发射粒子。相反，在没有辐射源的情况下，任何发射的能量粒子都会直接在相应细胞的空间附近产生。");
        drawParagraph("细胞形式的物质存在是辐射源发射粒子的前提。此外，模拟参数应调整为确保细胞随时间逐渐失去能量。");

        drawHeading2("如何整合神经网络？");
        drawParagraph("神经网络作为细胞功能可用。如果为细胞分配了“神经元”功能，它将拥有一个由 8 个神经元组成的小型神经网络。然而，这些网络可以互连形成任意大的网络，因为每个细胞从某些连接细胞的输出接收输入。");
        drawParagraph("此外，这些网络可以由传感器提供输入，并反过来控制肌肉和攻击细胞。");

        drawHeading2("我应该运行模拟多长时间才能看到进化变化？");
        drawParagraph(
            "这取决于许多因素：模拟世界的大小、突变率、可以通过模拟参数影响的各种选择压力以及自我复制的持续时间。通常应该等待几十代，这可能对应于数十万或数百万时间步长。"
            "在具有较小生物体和高突变率的小世界中，根据硬件的不同，有时每分钟都可以观察到进化变化。对于更复杂的模拟，您应该预期需要几个小时。");

        drawHeading2("如何向模拟中添加能量？");
        drawParagraph("向模拟中添加能量可以增加可用资源，从而增加人口。有一种方便的方法可以直接为所有构造器细胞提供额外能量。这可以通过在模拟参数窗口中启用“外部能量控制”插件来实现。接下来，设置要添加的能量量（例如，如果每个细胞有 100 个能量单位，1M 可以维持 10K 个细胞）。外部能量不是立即添加的，而是以可以在“流入”下指定的速率添加。");

        drawHeading2("如何首先创建细胞信号？");
        drawParagraph("要激活大多数细胞功能，需要来自连接细胞的信号输入。生成信号的最简单方法如下：");
        drawItemText("最直接的方法是使用神经细胞，它会在定期时间间隔生成信号。这里的优点是您可以精确配置时间间隔的长度。");
        drawItemText("信号也可以在神经元细胞中使用偏置值生成。");
        drawParagraph("此外，其他细胞如构造器细胞在被触发（自动）时会提供输出信号。");

        //ImGui::Text("There is a lot to explore. ALIEN features an extensive graph and particle editor in order to build custom worlds with desired "
        //            "environmental structures and machines. A documentation with tutorial-like introductions to various topics can be found at");

        //ImGui::Dummy(ImVec2(0.0f, 20.0f));

        //ImGui::PushFont(StyleRepository::get().getMonospaceMediumFont());
        //auto windowWidth = ImGui::GetWindowSize().x;
        //auto weblink = "https://alien-project.gitbook.io/docs";
        //auto textWidth = ImGui::CalcTextSize(weblink).x;
        //ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        //if(AlienImGui::Button(weblink)) {
        //    openWeblink(weblink);
        //}
        //ImGui::PopFont();

        ImGui::Dummy(ImVec2(0.0f, scale(20.0f)));

        ImGui::PopTextWrapPos();
    }
    ImGui::EndChild();

    AlienImGui::Separator();
    AlienImGui::ToggleButton(AlienImGui::ToggleButtonParameters().name("启动后显示"), _showAfterStartup);
}

void GettingStartedWindow::drawTitle()
{
    ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)Const::HeadlineColor);

    ImGui::PushFont(StyleRepository::get().getMediumFont());
    ImGui::Text("什么是");
    ImGui::PopFont();

    ImGui::SameLine();
    AlienImGui::NegativeSpacing();
    ImGui::PushFont(StyleRepository::get().getMediumBoldFont());
    ImGui::Text("人");
    ImGui::PopFont();

    ImGui::SameLine();
    AlienImGui::NegativeSpacing();
    AlienImGui::NegativeSpacing();
    ImGui::PushFont(StyleRepository::get().getMediumFont());
    ImGui::Text("工");
    ImGui::PopFont();

    ImGui::SameLine();
    AlienImGui::NegativeSpacing();
    ImGui::PushFont(StyleRepository::get().getMediumBoldFont());
    ImGui::Text("生");
    ImGui::PopFont();

    ImGui::SameLine();
    AlienImGui::NegativeSpacing();
    AlienImGui::NegativeSpacing();
    ImGui::PushFont(StyleRepository::get().getMediumFont());
    ImGui::Text("命");
    ImGui::PopFont();

    ImGui::SameLine();
    AlienImGui::NegativeSpacing();
    ImGui::PushFont(StyleRepository::get().getMediumBoldFont());
    ImGui::Text("模拟器");
    ImGui::PopFont();

    ImGui::SameLine();
    AlienImGui::NegativeSpacing();
    AlienImGui::NegativeSpacing();
    ImGui::PushFont(StyleRepository::get().getMediumFont());
    ImGui::Text("ALIEN？");
    ImGui::PopFont();

    ImGui::PopStyleColor();
    AlienImGui::Separator();
}

void GettingStartedWindow::drawHeading1(std::string const& text)
{
    AlienImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)Const::HeadlineColor);
    AlienImGui::BoldText(text);
    ImGui::PopStyleColor();
    AlienImGui::Separator();
}

void GettingStartedWindow::drawHeading2(std::string const& text)
{
    ImGui::Spacing();
    AlienImGui::BoldText(text);
}

void GettingStartedWindow::drawItemText(std::string const& text)
{
    ImGui::Text(ICON_FA_CHEVRON_RIGHT);
    ImGui::SameLine();
    AlienImGui::Text(text);
}

void GettingStartedWindow::drawParagraph(std::string const& text)
{
    AlienImGui::Text(text);
}
 