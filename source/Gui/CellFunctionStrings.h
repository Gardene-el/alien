#pragma once

#include <vector>
#include <string>
#include <map>

#include "EngineInterface/CellFunctionConstants.h"


using namespace std::string_literals;

namespace Const
{
    std::vector const CellFunctionStrings = {
        "神经元"s,
        "传输器"s,
        "构筑器"s,
        "感知器"s,
        "反应神经"s,
        "攻击器"s,
        "注射器"s,
        "运动器（肌肉）"s,
        "防御器"s,
        "重连器"s,
        "爆炸器"s,
        "无"s};

    std::map<CellFunction, std::string> const CellFunctionToStringMap = {
        {CellFunction_Constructor, "构筑器"},
        {CellFunction_Attacker, "攻击器"},
        {CellFunction_Injector, "注射器"},
        {CellFunction_Muscle, "运动器（肌肉）"},
        {CellFunction_Nerve, "反应神经"},
        {CellFunction_Neuron, "神经元"},
        {CellFunction_Sensor, "感知器"},
        {CellFunction_Transmitter, "传输器"},
        {CellFunction_Defender, "防御器"},
        {CellFunction_Reconnector, "重连器"},
        {CellFunction_Detonator, "爆炸器"},
        {CellFunction_None, "无"},
    };
}
