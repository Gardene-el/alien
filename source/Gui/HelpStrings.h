#pragma once

#include <string>
#include <vector>

#include "Fonts/IconsFontAwesome5.h"

#include "EngineInterface/CellFunctionConstants.h"

namespace Const
{
    std::string const GeneralInformation =
        "Please make sure that:\n\n1) You have an NVIDIA graphics card with compute capability 6.0 or higher (for example "
        "GeForce 10 series).\n\n2) You have the latest NVIDIA graphics driver installed.\n\n3) The name of the "
        "installation directory (including the parent directories) should not contain non-English characters. If this is not fulfilled, "
        "please re-install ALIEN to a suitable directory. Do not move the files manually. If you use Windows, make also sure that you install ALIEN with a "
        "Windows user that contains no non-English characters. If this is not the case, a new Windows user could be created to solve this problem.\n\n4) ALIEN needs "
        "write access to its own "
        "directory. This should normally be the case.\n\n5) If you have multiple graphics cards, please check that your primary monitor is "
        "connected to the CUDA-powered card. ALIEN uses the same graphics card for computation as well as rendering and chooses the one "
        "with the highest compute capability.\n\n6) If you possess both integrated and dedicated graphics cards, please ensure that the alien-executable is "
        "configured to use your high-performance graphics card. On Windows you need to access the 'Graphics settings,' add 'alien.exe' to the list, click "
        "'Options,' and choose 'High performance'.\n\nIf these conditions are not met, ALIEN may crash unexpectedly.\n\n"
        "If the conditions are met and the error still occurs, please start ALIEN with the command line parameter '-d', try to reproduce the error and "
        "then create a GitHub issue on https://github.com/chrxh/alien/issues where the log.txt is attached.";

    std::string const NotAllowedCharacters = "Your input contains not allowed characters.";

    std::string const NeuronTooltip =
        "此功能为细胞配备了一个由8个神经元组成的小型网络，具有8x8可配置权重、8个偏置值和激活函数。它处理来自通道#0到#7的输入，并将输出提供给这些通道。更准确地说，每个神经元的输出计算为\noutput_j := sigma(sum_i (input_i * weight_ji) + bias_j)，\n其中sigma代表激活函数（有不同的选择）。";

    std::string const TransmitterTooltip =
        "传输细胞设计用于传输能量。这对于例如为构造器细胞提供能量或支持被攻击的细胞非常重要。能量传输的工作原理如下：收集自身细胞和直接连接的细胞的部分多余能量，并将其传输到附近的其他细胞。当细胞的能量超过定义的正常值时，它就有多余的能量（参见“细胞生命周期”中的模拟参数“正常能量”）。传输细胞不需要激活，但它们可以传输从输入接收到的信号。";

    std::string const ConstructorTooltip =
        "构造器细胞根据包含的基因组构建细胞网络。构造过程逐个细胞进行，每个新细胞都需要能量。一旦生成新细胞，它就会连接到已经构建的细胞网络。\n\n" ICON_FA_CHEVRON_RIGHT " 输入通道#0：abs(value) > 阈值激活构造器（仅在“手动”模式下需要）\n\n" ICON_FA_CHEVRON_RIGHT " 输出通道#0：0（无法构造下一个细胞，例如没有能量，所需连接检查失败，完整性检查失败），1（下一个细胞构造成功）";

    std::string const SensorTooltip =
        "传感器细胞扫描其环境中某种颜色的细胞浓度，并提供与最近匹配的距离和角度。\n\n" ICON_FA_CHEVRON_RIGHT " 输入通道#0：abs(value) > 阈值激活传感器\n\n" ICON_FA_CHEVRON_RIGHT " 输出通道#0：0（无匹配）或1（匹配）\n\n" ICON_FA_CHEVRON_RIGHT " 输出通道#1：最后一次匹配的密度\n\n" ICON_FA_CHEVRON_RIGHT " 输出通道#2：最后一次匹配的距离（0 = 远，1 = 近）\n\n" ICON_FA_CHEVRON_RIGHT " 输出通道#3：最后一次匹配的角度";

    std::string const NerveTooltip =
        "默认情况下，神经细胞转发来自连接细胞的信号（如果有多个这样的细胞，则将其汇总），并直接将其作为输入提供给其他细胞。除此之外，还可以指定它在常规间隔内在通道#0生成信号。这可以用于触发其他传感器细胞、攻击细胞等。";

    std::string const AttackerTooltip =
        "攻击细胞通过从其他细胞网络（具有不同生物体ID）的周围细胞中窃取能量来攻击它们。获得的能量随后在自己的细胞网络中分配。\n\n" ICON_FA_CHEVRON_RIGHT " 输入通道#0：abs(value) > 阈值激活攻击者\n\n" ICON_FA_CHEVRON_RIGHT " 输出通道#0：与获得的能量成比例的值";

    std::string const InjectorTooltip =
        "注射器细胞可以通过自己的基因组覆盖其他构造器或注射器细胞的基因组。为此，它们需要被激活，保持在目标细胞附近一定的最短时间，并且在目标构造器细胞的情况下，其构造过程尚未开始。\n\n" ICON_FA_CHEVRON_RIGHT " 输入通道#0：abs(value) > 阈值激活注射器\n\n" ICON_FA_CHEVRON_RIGHT " 输出通道#0：0（未找到细胞）或1（注射进行中或已完成）";

    std::string const MuscleTooltip =
        "肌肉细胞可以根据输入和配置执行不同的运动和变形。\n\n" ICON_FA_CHEVRON_RIGHT " 输入通道#0：运动、弯曲或扩展/收缩的强度。负号对应相反的动作。\n\n" ICON_FA_CHEVRON_RIGHT " 输入通道#1：此通道仅用于弯曲加速。如果通道#1的符号与通道#0的符号不同，则在弯曲过程中不会获得加速。\n\n" ICON_FA_CHEVRON_RIGHT " 输入通道#3：此通道用于运动模式下的肌肉。它编码相对于检测到的物体的运动相对角度（如果激活了“朝向目标运动”参数）或相对于输入信号来自的相邻细胞的方向（如果未激活“朝向目标运动”参数）。在第一种情况下，物体必须由传感器细胞定位，输入信号来自该传感器细胞（不必是相邻细胞）。值-0.5对应-180度，+0.5对应+180度。";

    std::string const DefenderTooltip =
        "防御细胞不需要激活。它的存在减少了涉及攻击细胞的敌人攻击的强度，或延长了注射器细胞的注射时间。";

    std::string const ReconnectorTooltip =
        "重连器细胞可以与指定颜色的其他细胞（具有不同生物体ID）建立或断开细胞连接。\n\n" ICON_FA_CHEVRON_RIGHT " 输入通道#0：值 > 阈值触发与附近细胞的连接，值 < -阈值触发断开连接\n\n" ICON_FA_CHEVRON_RIGHT " 输出通道#0：0（未创建/移除连接）或1（创建/移除连接）";

    std::string const DetonatorTooltip =
        "如果引爆器细胞在通道#0接收到abs(value) > 阈值的输入，它将被激活。然后其计数器在每次执行后减少，直到达到0。之后，引爆器细胞将爆炸，周围的细胞将被高度加速。";

    std::string const CellFunctionTooltip =
        "细胞可以具有特定功能，使其能够感知环境、处理信息或采取行动。所有细胞功能的共同点是，它们从连接的细胞获取输入信号，这些细胞的执行编号与当前细胞的输入执行编号匹配。为此，从这些细胞的#0到#7通道的信号会被汇总，结果写入当前细胞的#0到#7通道。特别地，如果只有一个输入细胞，其信号将被简单地转发。执行细胞功能后，一些通道将被相应的细胞功能输出覆盖。\n\n重要提示：如果选择了细胞功能，此工具提示将更新以提供更具体的信息。";

    std::string const GenomeColorTooltip =
        "此属性定义细胞的颜色。这不仅仅是一个视觉标记。一方面，细胞颜色可以用来定义不同类型的细胞，这些细胞遵循不同的规则。为此，可以根据颜色指定模拟参数。例如，可以定义绿色细胞特别擅长吸收能量粒子，而其他颜色的细胞更擅长攻击外来细胞。\n另一方面，细胞颜色在感知中也起作用。传感器细胞专用于特定颜色，只能检测到相应颜色的细胞。";

    std::string const GenomeAngleTooltip =
        "可以在此处指定前驱细胞和后继细胞之间的角度。请注意，为方便起见，此处显示的角度已偏移180度。换句话说，值为0实际上对应于180度的角度，即一个直线段。";

    std::string const GenomeEnergyTooltip =
        "细胞在创建后应接收的能量。此值越大，构造细胞必须消耗的能量越多。";

    std::string const GenomeExecutionNumberTooltip =
        "细胞的功能可以按照此编号确定的特定顺序执行。值限制在0到5之间，并遵循模6逻辑。例如，执行编号为0的细胞将在时间点0、6、12、18等执行。执行编号为1的细胞将延迟一个时间步，即在1、7、13、19等执行。此时间偏移使细胞功能的协调成为可能。例如，需要从神经元细胞获取输入的肌肉细胞应在稍后的时间步执行。";

    std::string const GenomeInputExecutionNumberTooltip =
        "一个正常运作的生物体需要细胞协同工作。这可能涉及感知环境的传感器细胞、处理信息的神经元细胞、执行运动的肌肉细胞等。这些不同的细胞功能通常需要输入信号并产生输出信号。更新细胞信号的过程分为两个步骤：\n\n1) 当执行细胞功能时，首先计算输入信号。这涉及读取所有连接细胞的信号，这些细胞的“执行编号”与指定的“输入执行编号”匹配，并将其值汇总。\n\n2) 执行细胞功能并可以使用计算的信号作为输入。然后，细胞提供一个输出信号。\n\n设置“输入执行编号”是可选的。如果未设置，细胞将无法接收输入信号。";

    std::string const GenomeBlockOutputTooltip =
        "激活此开关后，细胞的输出可以被锁定，防止任何其他细胞将其用作输入。";

    std::string const GenomeRequiredConnectionsTooltip =
        "默认情况下，基因组序列中的细胞在创建时会自动连接到属于同一基因组的所有邻近细胞。然而，这可能会带来挑战，因为构造的细胞需要时间折叠到其所需位置。如果构造细胞的当前空间位置不利，新形成的细胞可能无法连接到所需的细胞，例如，由于距离太远。更好的方法是延迟构造过程，直到所需数量的同一基因组的邻近细胞在直接附近。可以在此处设置所需的细胞数量。\n例如，“所需连接”=2表示只有当至少有2个已构造的细胞（不包括前驱细胞）可供潜在连接时，才会创建要构造的细胞。如果不满足条件，构造过程将被推迟。";

    std::string const GenomeNeuronActivationFunctionTooltip =
        "激活函数是一个映射，将应用于所有输入通道的累积值，考虑权重和偏差，以计算神经元的输出，即output_j = sigma(sum_i (input_i * weight_ji) + bias_j)，其中sigma表示激活函数。可用的sigma选择如下：\n\n" ICON_FA_CHEVRON_RIGHT " Sigmoid(x) := 2 / (1 + exp(x)) - 1\n\n" ICON_FA_CHEVRON_RIGHT " Binary step(x) := 1 if x >= 0 and 0 if x < 0\n\n" ICON_FA_CHEVRON_RIGHT " Identity(x) := x\n\n" ICON_FA_CHEVRON_RIGHT " Abs(x) := x if x >= 0 and -x if x < 0\n\n" ICON_FA_CHEVRON_RIGHT " Gaussian(x) := exp(-2 * x * x)";

    std::string const GenomeNeuronWeightAndBiasTooltip =
        "每个神经元有8个输入通道，并通过公式output_j = sigma((sum_i (input_i * weight_ji) + bias_j)计算输出，其中sigma表示激活函数。";

    std::string const GenomeTransmitterEnergyDistributionTooltip =
        "有两种方法可以控制能量分配，在此设置：\n\n" ICON_FA_CHEVRON_RIGHT " 连接的细胞：在这种情况下，能量将均匀分配给所有连接的和连接-连接的细胞。\n\n" ICON_FA_CHEVRON_RIGHT " 传输器和构造器：在这里，能量将传输到同一细胞网络内空间上附近的构造器或其他传输器细胞。如果在某些距离处存在多个这样的传输器细胞，能量可以传输到更远的距离，例如，从攻击细胞到构造细胞。";

    std::string const GenomeConstructorActivationModeTooltip =
        "有两种模式可用于控制构造细胞：\n\n" ICON_FA_CHEVRON_RIGHT " 手动：只有在通道#0有信号时才会触发构造过程。\n\n" ICON_FA_CHEVRON_RIGHT " 自动：构造过程会在定期间隔自动触发。通道#0的信号不是必需的。\n\n在这两种情况下，如果没有足够的能量可用于创建细胞，构造过程将暂停，直到下次触发。";

    std::string const GenomeConstructorIntervalTooltip =
        "此值指定构造细胞自动触发的时间间隔。它以6的倍数给出（这是一个完整的执行周期）。这意味着值为1表示构造细胞每6个时间步激活一次。";

    std::string const GenomeConstructorOffspringActivationTime =
        "当构造细胞完全构建了一个新的细胞网络时，可以定义激活前的时间步。在激活之前，细胞网络处于休眠状态。这在后代不应立即变得活跃时特别有用，例如，防止其攻击其创造者。";

    std::string const GenomeConstructorAngle1Tooltip =
        "默认情况下，当构造细胞启动新构造时，新细胞会在最有可用空间的区域创建。此角度指定偏离该规则的偏差。";

    std::string const GenomeConstructorAngle2Tooltip =
        "此值确定从最后构造的细胞到倒数第二个构造的细胞和构造细胞的角度。效果在基因组编辑器的预览中最为明显。";

    std::string const GenomeSensorModeTooltip =
        "传感器可以在两种模式下操作：\n\n" ICON_FA_CHEVRON_RIGHT " 扫描附近：在这种模式下，扫描整个附近区域（通常在几百个单位的半径内）。扫描半径可以通过模拟参数调整（参见传感器设置中的“范围”）。\n\n" ICON_FA_CHEVRON_RIGHT " 扫描特定方向：在这种模式下，扫描过程仅限于特定方向。方向以角度指定。";

    std::string const GenomeSensorScanAngleTooltip =
        "可以在此处确定扫描过程应进行的方向角度。角度为0表示扫描将沿输入细胞（信号来源细胞）到传感器细胞的方向进行。";

    std::string const GenomeSensorScanColorTooltip = "限制传感器仅扫描特定颜色的细胞。";

    std::string const SensorRestrictToMutantsTooltip =
        "以下选项可用于仅检测具有特定属性的细胞：\n\n" ICON_FA_CHEVRON_RIGHT " 无：没有进一步限制。\n\n" ICON_FA_CHEVRON_RIGHT " 相同突变体：具有相关基因组的细胞。\n\n" ICON_FA_CHEVRON_RIGHT " 其他突变体：具有显著不同基因组的细胞。\n\n" ICON_FA_CHEVRON_RIGHT " 自由细胞：不是通过复制过程创建的细胞，而是通过能量粒子转化创建的细胞（它们可以作为自由食物）。\n\n" ICON_FA_CHEVRON_RIGHT " 手工制作的构造物：在编辑器中创建的细胞（例如墙壁）。\n\n" ICON_FA_CHEVRON_RIGHT " 复杂性较低的突变体：具有较低复杂性基因组的细胞。复杂性计算可以在模拟参数中的“基因组复杂性测量”附加功能中自定义。默认情况下，它是基因组中编码的细胞数量。\n\n" ICON_FA_CHEVRON_RIGHT " 复杂性较高的突变体：具有较高复杂性基因组的细胞。\n\n";

    std::string const GenomeSensorMinDensityTooltip =
        "搜索特定颜色细胞浓度的最小密度。此值范围在0到1之间。它控制传感器的灵敏度。通常，已经检测到相应颜色的少量细胞时，值为0.1。";

    std::string const GenomeSensorMinRangeTooltip = "如果激活，传感器仅检测距离等于或大于指定值的物体。";
    std::string const GenomeSensorMaxRangeTooltip = "如果激活，传感器仅检测距离等于或小于指定值的物体。";

    std::string const GenomeNerveGeneratePulsesTooltip = "如果启用，将在定期时间间隔生成通道#0的信号。";

    std::string const GenomeNervePulseIntervalTooltip =
        "可以在此处设置两个脉冲之间的间隔。它以周期指定，每个周期对应6个时间步。";

    std::string const GenomeNerveAlternatingPulsesTooltip =
        "默认情况下，生成的脉冲在通道#0中包含一个正值。当启用“交替脉冲”时，此值的符号将在特定时间间隔交替。这可以用于例如轻松创建肌肉细胞的来回运动或弯曲信号。";

    std::string const GenomeNervePulsesPerPhaseTooltip = "此值表示在通道#0中符号更改之前的脉冲数量。";

    std::string const GenomeAttackerEnergyDistributionTooltip =
        "攻击细胞可以通过两种不同的方法分配获得的能量。能量分配类似于传输器细胞。\n\n" ICON_FA_CHEVRON_RIGHT " 连接的细胞：在这种情况下，能量将均匀分配给所有连接的和连接-连接的细胞。\n\n" ICON_FA_CHEVRON_RIGHT " 传输器和构造器：在这里，能量将传输到同一细胞网络内空间上附近的构造器或其他传输器细胞。如果在某些距离处存在多个这样的传输器细胞，能量可以传输到更远的距离，例如，从攻击细胞到构造细胞。";

    std::string const GenomeInjectorModeTooltip = ICON_FA_CHEVRON_RIGHT
        " 仅空细胞：只有具有空基因组的细胞可以被感染。当生物体想要将其基因组注入另一个自己的构造细胞时（例如构建孢子），此模式非常有用。在此模式下，注射过程不需要任何时间。\n\n" ICON_FA_CHEVRON_RIGHT " 所有细胞：在此模式下没有限制，例如，任何其他构造细胞或注射器细胞都可以被感染。注射过程的持续时间取决于模拟参数“注射时间”。";

    std::string const GenomeMuscleModeTooltip = ICON_FA_CHEVRON_RIGHT
        " 移动到传感器目标：如果输入信号源自先前检测到目标的传感器细胞，则可以执行移动。移动方向相对于目标指定。\n\n" ICON_FA_CHEVRON_RIGHT " 扩展和收缩：导致参考距离相对于输入细胞的延长（或收缩）。\n\n" ICON_FA_CHEVRON_RIGHT " 弯曲：增加（或减少）肌肉细胞、输入细胞和从肌肉细胞顺时针方向最近连接细胞之间的角度。";

    std::string const GenomeDefenderModeTooltip =
        ICON_FA_CHEVRON_RIGHT " 反攻击：减少敌方攻击细胞的攻击强度\n\n" ICON_FA_CHEVRON_RIGHT " 反注射：增加敌方注射器细胞的注射时间";

    std::string const GenomeReconnectorRestrictToColorTooltip = "指定要建立或破坏连接的细胞颜色。";

    std::string const ReconnectorRestrictToMutantsTooltip =
        "以下选项可用于仅绑定具有某些属性的细胞：\n\n"
        ICON_FA_CHEVRON_RIGHT" 无：没有进一步的限制。\n\n"
        ICON_FA_CHEVRON_RIGHT" 相同突变体：具有相关基因组的细胞。\n\n"
        ICON_FA_CHEVRON_RIGHT" 其他突变体：具有显著不同基因组的细胞。\n\n"
        ICON_FA_CHEVRON_RIGHT" 自由细胞：不是通过繁殖而是通过能量粒子转换创建的细胞（它们可以作为自由食物）。\n\n"
        ICON_FA_CHEVRON_RIGHT" 手工构造：在编辑器中创建的细胞（例如墙）。\n\n"
        ICON_FA_CHEVRON_RIGHT" 复杂度较低的突变体：具有较低复杂度基因组的细胞。复杂度计算可以在模拟参数中的“基因组复杂度测量”插件中自定义。默认情况下，它是基因组中编码的细胞数量。\n\n"
        ICON_FA_CHEVRON_RIGHT" 复杂度较高的突变体：具有较高复杂度基因组的细胞。\n\n";

    std::string const DetonatorStateTooltip =
        ICON_FA_CHEVRON_RIGHT " 准备：引爆器细胞等待通道#0上的输入。如果abs(value) > 阈值，引爆器将被激活。\n\n"
        ICON_FA_CHEVRON_RIGHT " 激活：每次引爆器执行时，倒计时减少直到0。如果倒计时为0，引爆器将爆炸。\n\n"
        ICON_FA_CHEVRON_RIGHT " 爆炸：引爆器已经爆炸。";

    std::string const GenomeDetonatorCountdownTooltip = "倒计时指定了引爆器爆炸前的周期数（每周期包含6个时间步）。";

    std::string const SubGenomeTooltip =
        "如果在基因组中编码了构造器或注射器细胞，该细胞本身可以包含另一个基因组。这个子基因组可以描述额外的身体部位或生物的分支。例如，子基因组可以进一步包含子子基因组，等等。要在此处通过点击“粘贴”插入子基因组，必须先将其复制到剪贴板。这可以通过工具栏中的“复制基因组”按钮完成。此操作将当前选项卡中的整个基因组复制到剪贴板。如果要创建自我复制，不应插入子基因组；而是将其切换到“自我复制”模式。在这种情况下，构造器的子基因组指的是其上级基因组。";

    std::string const GenomeGeometryTooltip =
        "基因组描述了一个连接细胞的网络。一方面，可以选择预定义的几何形状（例如三角形或六边形）。然后，基因组中编码的细胞将沿着这种几何形状生成并适当地连接在一起。另一方面，也可以通过为每个细胞（序列中的第一个和最后一个除外）设置前驱细胞和后继细胞之间的角度来定义自定义几何形状。";

    std::string const GenomeConnectionDistanceTooltip =
        "这里确定基因组序列中每个细胞与其前驱细胞之间的空间距离。";

    std::string const GenomeStiffnessTooltip = "此值设置整个编码细胞网络的刚度。刚度决定了生成的力将细胞网络推向其参考配置的程度。";

    std::string const GenomeAngleAlignmentTooltip =
        "网络中的连接细胞三元组彼此之间具有特定的空间角度。这些角度由细胞中编码的参考角度引导。通过此设置，可以选择性地指定参考角度必须仅为某些值的倍数。这允许创建的网络具有更大的稳定性，因为否则角度会更容易受到外部影响。建议选择60度，因为它允许准确表示大多数几何形状。";

    std::string const GenomeNumBranchesTooltip = "指定构造器可以用来构建细胞网络的分支数量。每个分支连接到构造器细胞，并由编码的细胞网络的重复组成。";

    std::string const GenomeRepetitionsPerBranchTooltip =
        "此值指定基因组中描述的细胞网络应为每个构造连接多少次。对于大于1的值，细胞网络几何形状必须满足某些要求（例如，矩形、六边形、循环和棒棒糖几何形状不适合连接）。也可以使用无限值，但不应用于激活的完整性检查（参见模拟参数）。";

    std::string const GenomeConcatenationAngle1 =
        "此值描述从后续细胞网络的第一个细胞看两个连接的细胞网络之间的角度。";

    std::string const GenomeConcatenationAngle2 =
        "此值描述从前一个细胞网络的最后一个细胞看两个连接的细胞网络之间的角度。";

    std::string const GenomeSeparationConstructionTooltip =
        "在这里，可以配置基因组中编码的细胞网络在完全构建后是否应与构造器细胞分离。禁用此属性对于编码生长结构（如植物状物种）或生物体部位是有用的。";

    std::string const CellEnergyTooltip = "细胞的内部能量量。当其能量低于临界阈值时，细胞会衰减（参见“最低能量”模拟参数）。";

    std::string const CellStiffnessTooltip = "刚度决定了位移后产生的力的大小，以将细胞（网络）推回到其参考配置。";

    std::string const CellMaxConnectionTooltip = "细胞可以与其他细胞形成的最大连接数。";

    std::string const CellIndestructibleTooltip = "当细胞被设置为不可破坏的墙时，它变得不朽，抗外力，但仍然能够线性移动。此外，未连接的普通细胞和能量粒子会从不可破坏的细胞上弹开。";

    std::string const CellReferenceDistanceTooltip = "参考距离定义了两个连接细胞之间没有力作用的距离。如果实际距离大于参考距离，细胞会相互吸引。如果小于参考距离，细胞会相互排斥。";

    std::string const CellReferenceAngleTooltip = "参考角度定义了两个细胞连接之间的角度。如果实际角度较大，切向力会作用于连接的细胞，旨在减小角度。相反，如果实际角度较小，切向力会倾向于增大此角度。通过这种类型的力，细胞网络可以在变形后折回到所需的形状。";

    std::string const CellAgeTooltip = "细胞的年龄，以时间步为单位。";

    std::string const CellIdTooltip = "细胞的ID是一个唯一的64位数字，用于标识整个世界中的细胞，且无法更改。细胞ID以十六进制表示。";

    std::string const CellCreatureIdTooltip = "此值大致标识特定生物。虽然不能保证，但很可能两个生物会有不同的生物ID。";

    std::string const CellMutationIdTooltip = "突变ID是用于区分突变体的值。在大多数突变（神经网络和细胞属性除外）之后，突变ID会发生变化。一些值具有特殊意义：\n\n" ICON_FA_CHEVRON_RIGHT " 0：此值用于手工制作的细胞。这指的是用户人工创建的细胞。\n\n" ICON_FA_CHEVRON_RIGHT " 1：此值用于自由细胞。自由细胞是指不是通过自我复制过程创建的细胞，而是通过能量粒子的转化创建的细胞。";
    std::string const GenomeComplexityTooltip =
        "此值表示生物体基因组的复杂性。计算方法可以在模拟参数中的“基因组复杂度测量”插件中自定义。默认情况下，它是基因组中编码的细胞数量。";

    std::string const CellLivingStateTooltip =
        "细胞可以存在于各种状态。当生物体的细胞网络正在构建时，其细胞处于“建设中”状态。一旦细胞网络完成，细胞会短暂进入“激活”状态，然后很快转变为“准备就绪”状态。如果细胞网络正在死亡过程中，其细胞处于“死亡中”状态。\n\n"
        "如果参数“细胞死亡后果”设置为“分离的生物体部分死亡”：当细胞与其自我复制的构造器细胞所在的生物体分离时，细胞处于“分离”状态。然而，如果仍然存在一个非死亡的自我复制细胞，分离的细胞将转变为“复活”状态，然后很快转变为“准备就绪”状态。";

    std::string const ColoringParameterTooltip =
            "在这里，可以设置细胞在渲染期间的着色方式。\n\n" ICON_FA_CHEVRON_RIGHT
            " 能量：细胞的能量越多，显示越亮。使用灰度。\n\n" ICON_FA_CHEVRON_RIGHT
            " 标准细胞颜色：每个细胞被分配一种默认的7种颜色之一，并以此选项显示。\n\n" ICON_FA_CHEVRON_RIGHT
            " 基因代际：不同的突变体用不同的颜色表示（仅考虑较大的结构突变，如平移或复制）。\n\n" ICON_FA_CHEVRON_RIGHT
            " 基因代际和细胞功能：突变体和细胞功能着色的组合。\n\n" ICON_FA_CHEVRON_RIGHT
            " 细胞状态：绿色 = 建设中，蓝色 = 准备就绪，红色 = 死亡中\n\n" ICON_FA_CHEVRON_RIGHT
            " 基因组复杂度：当参数“复杂生物保护”激活时，攻击细胞可以利用此属性（参见那里的工具提示）。着色如下：蓝色 = 低奖励的生物（通常是小型或简单的基因组结构），红色 = 大奖励\n\n" ICON_FA_CHEVRON_RIGHT
            " 单一细胞功能：可以突出显示特定类型的细胞功能，在下一个参数中选择。\n\n" ICON_FA_CHEVRON_RIGHT
            " 所有细胞功能：细胞根据其细胞功能着色。";

    inline std::string getCellFunctionTooltip(CellFunction cellFunction)
    {
        switch (cellFunction) {
        case CellFunction_Neuron:
            return Const::NeuronTooltip;
        case CellFunction_Transmitter:
            return Const::TransmitterTooltip;
        case CellFunction_Constructor:
            return Const::ConstructorTooltip;
        case CellFunction_Sensor:
            return Const::SensorTooltip;
        case CellFunction_Nerve:
            return Const::NerveTooltip;
        case CellFunction_Attacker:
            return Const::AttackerTooltip;
        case CellFunction_Injector:
            return Const::InjectorTooltip;
        case CellFunction_Muscle:
            return Const::MuscleTooltip;
        case CellFunction_Defender:
            return Const::DefenderTooltip;
        case CellFunction_Reconnector:
            return Const::ReconnectorTooltip;
        case CellFunction_Detonator:
            return Const::DetonatorTooltip;
        default:
            return Const::CellFunctionTooltip;
        }
    };

    std::string const GenomeNumCellsRecursivelyTooltip =
        "基因组中所有编码细胞的数量，包括其子基因组、重复次数和分支数量。";

    std::string const GenomeBytesTooltip = "基因组的长度，以字节为单位。";

    std::string const GenomeGenerationTooltip = "此值表示该基因组被后代继承的次数。";

    std::string const GenomeNumCellsTooltip = "基因组中每次重复编码的细胞数量。不包括子基因组的细胞。";

    std::string const GenomeCurrentBranchTooltip = "此数字指定当前构造过程所在的分支。每个分支连接到构造器细胞，并由编码的细胞网络的重复组成。";

    std::string const GenomeCurrentRepetitionTooltip =
        "基因组中编码的细胞网络可以通过指定重复次数来重复构建。此值表示当前重复的索引。";

    std::string const GenomeCurrentCellTooltip = "基因组中下一个要构建的细胞的序列号。";

    std::string const GenomePreviewTooltip = "此处显示基因组中编码细胞的空间结构。这只是一个粗略的预测，不使用物理引擎。";

    std::string const CellInjectorCounterTooltip =
        "当基因组注射开始时，每次连续成功激活注射器后，计数器递增。一旦计数器达到特定阈值（参见“注射时间”模拟参数），注射过程完成。";

    std::string const CellSensorTargetCreatureIdTooltip = "最后一次扫描的生物体的ID。";

    std::string const NeuronInputTooltipByChannel[8] = {
        "以下细胞功能将其输出写入通道#0：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元\n\n" ICON_FA_CHEVRON_RIGHT " 构造器：0（无法构造下一个细胞，例如没有能量，所需连接检查失败，完整性检查失败），1（下一个细胞构造成功）\n\n" ICON_FA_CHEVRON_RIGHT " 传感器：0（无匹配）或1（匹配）\n\n" ICON_FA_CHEVRON_RIGHT " 攻击者：与获得的能量成比例的值\n\n" ICON_FA_CHEVRON_RIGHT " 注射器：0（未找到细胞）或1（注射进行中或已完成）\n\n" ICON_FA_CHEVRON_RIGHT " 重连器：0（未创建/移除连接）或1（创建/移除连接）",
        "以下细胞功能将其输出写入通道#1：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元\n\n" ICON_FA_CHEVRON_RIGHT " 传感器：最后一次匹配的密度",
        "以下细胞功能将其输出写入通道#2：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元\n\n" ICON_FA_CHEVRON_RIGHT " 传感器：最后一次匹配的距离（0 = 远，1 = 近）",
        "以下细胞功能将其输出写入通道#3：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元\n\n" ICON_FA_CHEVRON_RIGHT " 传感器：最后一次匹配的角度",
        "以下细胞功能将其输出写入通道#4：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元",
        "以下细胞功能将其输出写入通道#5：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元",
        "以下细胞功能将其输出写入通道#6：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元",
        "以下细胞功能将其输出写入通道#7：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元\n\n" ICON_FA_CHEVRON_RIGHT " 攻击者：如果细胞被其他攻击细胞攻击，则为1"
    };

    std::string const NeuronOutputTooltipByChannel[8] = {
        "以下细胞功能从通道#0获取其输入：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元\n\n" ICON_FA_CHEVRON_RIGHT " 构造器：abs(value) > 阈值激活构造器（仅在“手动”模式下需要）\n\n" ICON_FA_CHEVRON_RIGHT " 传感器：abs(value) > 阈值激活传感器\n\n" ICON_FA_CHEVRON_RIGHT " 攻击者：abs(value) > 阈值激活攻击者\n\n" ICON_FA_CHEVRON_RIGHT " 注射器：abs(value) > 阈值激活注射器\n\n" ICON_FA_CHEVRON_RIGHT " 肌肉：运动、弯曲或扩展/收缩的强度。负号对应相反的动作。\n\n" ICON_FA_CHEVRON_RIGHT " 重连器：值 > 阈值触发与附近细胞的连接，值 < -阈值触发断开连接\n\n" ICON_FA_CHEVRON_RIGHT " 引爆器：abs(value) > 阈值激活引爆器",
        "以下细胞功能从通道#1获取其输入：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元\n\n" ICON_FA_CHEVRON_RIGHT " 肌肉：此通道仅用于弯曲加速。如果通道#1的符号与通道#0的符号不同，则在弯曲过程中不会获得加速。",
        "以下细胞功能从通道#2获取其输入：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元",
        "以下细胞功能从通道#3获取其输入：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元\n\n" ICON_FA_CHEVRON_RIGHT " 肌肉：此通道用于运动模式下的肌肉。它编码相对于检测到的物体的运动相对角度（如果激活了“朝向目标运动”参数）或相对于输入信号来自的相邻细胞的方向（如果未激活“朝向目标运动”参数）。在第一种情况下，物体必须由传感器细胞定位，输入信号来自该传感器细胞（不必是相邻细胞）。值-0.5对应-180度，+0.5对应+180度。",
        "以下细胞功能从通道#4获取其输入：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元",
        "以下细胞功能从通道#5获取其输入：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元",
        "以下细胞功能从通道#6获取其输入：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元",
        "以下细胞功能从通道#7获取其输入：\n\n" ICON_FA_CHEVRON_RIGHT " 神经元"
    };

    std::string const CreatorPencilRadiusTooltip = "铅笔的半径，以单元格数量表示。";

    std::string const CreatorAscendingExecutionOrderNumberTooltip =
        "每个生成的单元格的“执行顺序号”比前一个生成的单元格大一。";

    std::string const CreatorRectangleWidthTooltip = "矩形的宽度，以单元格数量表示。";

    std::string const CreatorRectangleHeightTooltip = "矩形的高度，以单元格数量表示。";

    std::string const CreatorHexagonLayersTooltip = "从中心开始的六边形层数，以单元格数量表示。";

    std::string const CreatorDiscOuterRadiusTooltip = "圆盘的外半径，以单元格数量表示。";

    std::string const CreatorDiscInnerRadiusTooltip = "圆盘的内半径，以单元格数量表示。";

    std::string const CreatorDistanceTooltip = "两个连接单元格之间的距离。";

    std::string const CreatorStickyTooltip = "如果选择了粘性属性，创建的单元格通常可以形成进一步的连接。也就是说，它们在碰撞后可以与其他单元格网络“粘在一起”。";

    std::string const LoginHowToCreateNewUseTooltip = "请输入您期望的用户名与密码。用户创建将通过点击“创建用户”按钮执行。";

    std::string const LoginForgotYourPasswordTooltip = "请输入用户名，密码重设将通过点击“重设密码”按钮执行。";

    std::string const LoginSecurityInformationTooltip =
       "传输至服务器的数据已通过https加密。在服务端，邮箱地址并不储存在明文中，是以加密为SHA-256哈希值的形式储存在数据库中。"
       "如果'记住'按钮已启用，密码将被储存在 Windows 注册表中的路径 'HKEY_CURRENT_USER\\SOFTWARE\\alien' 下。在其他操作系统中，密码将被储存在本地电脑上的 'settings.json' 文件中。";

    std::string const LoginRememberTooltip = "如果'记住'按钮已启用，密码将被储存在 Windows 注册表中的路径 'HKEY_CURRENT_USER\\SOFTWARE\\alien' 下。在其他操作系统中，密码将被储存在本地电脑上的 'settings.json' 文件中。推荐您选用一个不在其他地方使用的密码。";

    std::string const LoginShareGpuInfoTooltip1 =
        "如果该选项已启用，其他用户将可以在浏览器窗口中看到下列你有的显卡：";
    std::string const LoginShareGpuInfoTooltip2 =
        "同样的，你也能够看到其他已注册用户分享的显卡。";

    std::vector<std::string> const ActivationFunctions = {"Sigmoid", "Binary step", "Identity", "Absolute value", "Gaussian"};

    std::string const BrowserWorkspaceTooltip =
        "这里有三类空间你可以找到，在一些空间中你能上传模拟器和基因组:\n\n" ICON_FA_CHEVRON_RIGHT
        " ALIEN项目: 这个空间包含了随发布版本一起发布的模拟器。它们涵盖了广泛的范围并利用了不同的功能。\n\n" ICON_FA_CHEVRON_RIGHT
        " 公开: 所有已登录的用户可以在这里分享他们的模拟器和基因组。保存在此的文件对所有的用户都是可浏览的。\n\n" ICON_FA_CHEVRON_RIGHT
        " 个人: 每一个用户账号都有一个自己的个人空间。上传在此的模拟器和基因组仅对该已登录的用户可浏览。";

    std::string const ParameterRadiationAbsorptionLowGenomeComplexityPenaltyTooltip =
        "当该参数减少时,拥有更少基因组复杂度的细胞将会从吸收的粒子中得到更少能量。";
}
