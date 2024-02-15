﻿#include "RenderingKernels.cuh"

#include <boost/mpl/min_max.hpp>

#include "CellFunctionProcessor.cuh"
#include "SpotCalculator.cuh"

namespace
{
    auto constexpr ZoomLevelForConnections = 1.0f;
    auto constexpr ZoomLevelForShadedCells = 10.0f;
    auto constexpr ZoomLevelForArrows = 15.0f;

    __device__ __inline__ void drawPixel(uint64_t* imageData, unsigned int index, float3 const& color)
    {
        imageData[index] = toUInt64(color.y * 225.0f) << 16 | toUInt64(color.x * 225.0f) << 0 | toUInt64(color.z * 225.0f) << 32;
    }

    __device__ __inline__ void drawAddingPixel(uint64_t* imageData, unsigned int const& numPixels, unsigned int index, float3 const& colorToAdd)
    {
        if (index < numPixels) {
            uint64_t rawColorToAdd = toUInt64(colorToAdd.y * 255.0f) << 16 | toUInt64(colorToAdd.x * 255.0f) << 0 | toUInt64(colorToAdd.z * 255.0f) << 32;
            alienAtomicAdd64(&imageData[index], rawColorToAdd);
        }
    }

    __device__ __inline__ float3 colorToFloat3(unsigned int value)
    {
        return float3{toFloat(value & 0xff) / 255, toFloat((value >> 8) & 0xff) / 255, toFloat((value >> 16) & 0xff) / 255};
    }

    __device__ __inline__ float2 mapWorldPosToVectorImagePos(
        float2 const& rectUpperLeft,
        float2 const& pos,
        float2 const& universeImageSize,
        int2 const& imageSize,
        float zoom)
    {
        auto result = float2{(pos.x - rectUpperLeft.x) * zoom, (pos.y - rectUpperLeft.y) * zoom};
        if (cudaSimulationParameters.borderlessRendering) {
            result.x = Math::modulo(result.x, universeImageSize.x);
            result.y = Math::modulo(result.y, universeImageSize.y);
        }
        return result;
    }

    __device__ __inline__ int3 convertHSVtoRGB(float h, float s, float v)
    {
        auto c = v * s;
        auto x = c * (1 - abs(fmodf((h / 60), 2) - 1));
        auto m = v - c;

        float r_, g_, b_;
        if (0 <= h && h < 60.0f) {
            r_ = c;
            g_ = x;
            b_ = 0;
        }
        if (60.0f <= h && h < 120.0f) {
            r_ = x;
            g_ = c;
            b_ = 0;
        }
        if (120.0f <= h && h < 180.0f) {
            r_ = 0;
            g_ = c;
            b_ = x;
        }
        if (180.0f <= h && h < 240.0f) {
            r_ = 0;
            g_ = x;
            b_ = c;
        }
        if (240.0f <= h && h < 300.0f) {
            r_ = x;
            g_ = 0;
            b_ = c;
        }
        if (300.0f <= h && h <= 360.0f) {
            r_ = c;
            g_ = 0;
            b_ = x;
        }
        return {toInt((r_ + m) * 255), toInt((g_ + m) * 255), toInt((b_ + m) * 255)};
    }

    __device__ __inline__ float3 calcColor(Cell* cell, int selected)
    {
        float factor = min(300.0f, cell->energy) / 320.0f;
        if (1 == selected) {
            factor *= 2.5f;
        }
        if (2 == selected) {
            factor *= 1.75f;
        }

        uint32_t cellColor;
        if (cudaSimulationParameters.cellColoring == CellColoring_None) {
            cellColor = 0xbfbfbf;
        }
        if (cudaSimulationParameters.cellColoring == CellColoring_CellColor) {
            switch (calcMod(cell->color, 7)) {
            case 0: {
                cellColor = Const::IndividualCellColor1;
                break;
            }
            case 1: {
                cellColor = Const::IndividualCellColor2;
                break;
            }
            case 2: {
                cellColor = Const::IndividualCellColor3;
                break;
            }
            case 3: {
                cellColor = Const::IndividualCellColor4;
                break;
            }
            case 4: {
                cellColor = Const::IndividualCellColor5;
                break;
            }
            case 5: {
                cellColor = Const::IndividualCellColor6;
                break;
            }
            case 6: {
                cellColor = Const::IndividualCellColor7;
                break;
            }
            }
        }
        if (cudaSimulationParameters.cellColoring == CellColoring_MutationId) {
            auto h = abs(toInt((cell->mutationId * 12107) % 360));
            auto s = 0.6f + toFloat(abs(toInt(cell->mutationId * 12107)) % 400) / 1000;
            auto rgb = convertHSVtoRGB(toFloat(h), s, 1.0f);
            cellColor = (rgb.x << 16) | (rgb.y << 8) | rgb.z;
        }
        if (cudaSimulationParameters.cellColoring == CellColoring_LivingState) {
            switch (cell->livingState) {
            case LivingState_Ready:
                cellColor = 0x0000ff;
                break;
            case LivingState_UnderConstruction:
                cellColor = 0x00ff00;
                break;
            case LivingState_Activating:
                cellColor = 0xffffff;
                break;
            case LivingState_Dying:
                cellColor = 0xff0000;
                break;
            default:
                cellColor = 0x000000;
                break;
            }
        }

        if (cudaSimulationParameters.cellColoring == CellColoring_GenomeSize) {
            auto rgb = convertHSVtoRGB(toFloat(min(360.0f, 240.0f + powf(toFloat(cell->genomeComplexity), 0.3f) * 5.0f)),  1.0f, 1.0f);
            cellColor = (rgb.x << 16) | (rgb.y << 8) | rgb.z;
        }

        if (cudaSimulationParameters.cellColoring == CellColoring_CellFunction) {
            if (cell->cellFunction == cudaSimulationParameters.highlightedCellFunction) {
                auto h = (toFloat(cell->cellFunction) / toFloat(CellFunction_Count - 1)) * 360.0f;
                auto rgb = convertHSVtoRGB(toFloat(h), 0.7f, 1.0f);
                cellColor = (rgb.x << 16) | (rgb.y << 8) | rgb.z;
                factor = 5.0f;
            } else {
                cellColor = 0x303030;
            }
        }

        return {
            toFloat((cellColor >> 16) & 0xff) / 256.0f * factor,
            toFloat((cellColor >> 8) & 0xff) / 256.0f * factor,
            toFloat(cellColor & 0xff) / 256.0f * factor};
    }

    __device__ __inline__ float3 calcColor(Particle* particle, bool selected)
    {
        auto intensity = max(min((toInt(particle->energy) + 10.0f) * 5, 450.0f), 20.0f) / 1000.0f;
        if (selected) {
            intensity *= 2.5f;
        }

        return {intensity, intensity, 0.08f};
    }

    __device__ __inline__ void drawDot(uint64_t* imageData, int2 const& imageSize, float2 const& pos, float3 const& colorToAdd)
    {
        int2 intPos{toInt(pos.x), toInt(pos.y)};
        if (intPos.x >= 0 && intPos.y >= 0 && intPos.y < imageSize.y) {

            float2 posFrac{pos.x - intPos.x, pos.y - intPos.y};
            unsigned int index = intPos.x + intPos.y * imageSize.x;
            auto numPixels = imageSize.x * imageSize.y;

            if (intPos.x < imageSize.x) {
                float3 colorToAdd1 = colorToAdd * (1.0f - posFrac.x) * (1.0f - posFrac.y);
                drawAddingPixel(imageData, numPixels, index, colorToAdd1);

                float3 colorToAdd3 = colorToAdd * (1.0f - posFrac.x) * posFrac.y;
                drawAddingPixel(imageData, numPixels, index + imageSize.x, colorToAdd3);
            }
            if (intPos.x + 1 < imageSize.x) {
                float3 colorToAdd2 = colorToAdd * posFrac.x * (1.0f - posFrac.y);
                drawAddingPixel(imageData, numPixels, index + 1, colorToAdd2);

                float3 colorToAdd4 = colorToAdd * posFrac.x * posFrac.y;
                drawAddingPixel(imageData, numPixels, index + imageSize.x + 1, colorToAdd4);
            }
        }
    }

    __device__ __inline__ void drawCircle(uint64_t* imageData, int2 const& imageSize, float2 pos, float3 color, float radius, bool shaded = true, bool inverted = false)
    {
        if (radius > 2.0 - NEAR_ZERO) {
            auto radiusSquared = radius * radius;
            for (float x = -radius; x <= radius; x += 1.0f) {
                for (float y = -radius; y <= radius; y += 1.0f) {
                    auto rSquared = x * x + y * y;
                    if (rSquared <= radiusSquared) {
                        auto factor = inverted ? (rSquared / radiusSquared) * 2 : (1.0f - rSquared / radiusSquared) * 2;
                        auto angle = Math::angleOfVector({x, y});
                        if (shaded) {
                            angle -= 45.0f;
                            if (angle > 180.0f) {
                                angle -= 360.0f;
                            }
                            if (angle < -180.0f) {
                                angle += 360.0f;
                            }
                            factor *= 65.0f / (abs(angle) + 1.0f); 
                        }
                        drawDot(imageData, imageSize, pos + float2{x, y}, color * min(factor, 1.0f));
                    }
                }
            }
        } else {
            color = color * radius * 2;
            drawDot(imageData, imageSize, pos, color);
            color = color * 0.3f;
            drawDot(imageData, imageSize, pos + float2{1, 0}, color);
            drawDot(imageData, imageSize, pos + float2{-1, 0}, color);
            drawDot(imageData, imageSize, pos + float2{0, 1}, color);
            drawDot(imageData, imageSize, pos + float2{0, -1}, color);
        }
    }

    __device__ __inline__ void
    drawLine(float2 const& start, float2 const& end, float3 const& color, uint64_t* imageData, int2 imageSize, float pixelDistance = 1.5f)
    {
        float dist = Math::length(end - start);
        float2 const v = {static_cast<float>(end.x - start.x) / dist * pixelDistance, static_cast<float>(end.y - start.y) / dist * pixelDistance};
        float2 pos = start;

        for (float d = 0; d <= dist; d += pixelDistance) {
            drawDot(imageData, imageSize, pos, color);
            pos = pos + v;
        }
    }

    __device__ __inline bool isLineVisible(float2 const& startImagePos, float2 const& endImagePos, float2 const& universeImageSize)
    {
        return abs(startImagePos.x - endImagePos.x) < universeImageSize.x / 2 && abs(startImagePos.y - endImagePos.y) < universeImageSize.y / 2;
    }
}

/************************************************************************/
/* Main      															*/
/************************************************************************/
__global__ void cudaDrawBackground(uint64_t* imageData, int2 imageSize, int2 worldSize, float zoom, float2 rectUpperLeft, float2 rectLowerRight)
{
    BaseMap map;
    map.init(worldSize);

    int2 outsideRectUpperLeft{-min(toInt(rectUpperLeft.x * zoom), 0), -min(toInt(rectUpperLeft.y * zoom), 0)};
    int2 outsideRectLowerRight{
        imageSize.x - max(toInt((rectLowerRight.x - worldSize.x) * zoom), 0), imageSize.y - max(toInt((rectLowerRight.y - worldSize.y) * zoom), 0)};

    auto baseColor = colorToFloat3(cudaSimulationParameters.backgroundColor);
    float3 spotColors[MAX_SPOTS];
    for (int i = 0; i < cudaSimulationParameters.numSpots; ++i) {
        spotColors[i] = colorToFloat3(cudaSimulationParameters.spots[i].color);
    }

    auto const partition = calcAllThreadsPartition(imageSize.x * imageSize.y);
    for (int index = partition.startIndex; index <= partition.endIndex; ++index) {
        auto x = index % imageSize.x;
        auto y = index / imageSize.x;
        float2 worldPos = {toFloat(x) / zoom + rectUpperLeft.x, toFloat(y) / zoom + rectUpperLeft.y};

        if (!cudaSimulationParameters.borderlessRendering && (x < outsideRectUpperLeft.x || y < outsideRectUpperLeft.y || x >= outsideRectLowerRight.x || y >= outsideRectLowerRight.y)) {
            imageData[index] = 0;
        } else {
            auto color = SpotCalculator::calcResultingValue(map, worldPos, baseColor, spotColors);
            drawPixel(imageData, index, color);
        }
    }
}

__global__ void cudaDrawCells(uint64_t timestep, int2 worldSize, float2 rectUpperLeft, float2 rectLowerRight, Array<Cell*> cells, uint64_t* imageData, int2 imageSize, float zoom)
{
    auto const partition = calcAllThreadsPartition(cells.getNumEntries());

    BaseMap map;
    map.init(worldSize);

    auto shadedCells = zoom >= ZoomLevelForShadedCells;
    auto universeImageSize = toFloat2(worldSize) * zoom;
    for (int index = partition.startIndex; index <= partition.endIndex; ++index) {
        auto const& cell = cells.at(index);

        auto cellPos = cell->pos;
        auto cellImagePos = mapWorldPosToVectorImagePos(rectUpperLeft, cellPos, universeImageSize, imageSize, zoom);
        if (isContainedInRect({0, 0}, toFloat2(imageSize), cellImagePos)) {

            //draw cell
            auto color = calcColor(cell, cell->selected);
            auto radius = zoom / 3;
            drawCircle(imageData, imageSize, cellImagePos, color, radius, shadedCells, true);

            color = color * min((zoom - 1.0f) / 3, 1.0f);
            if (cell->isActive() && zoom >= cudaSimulationParameters.zoomLevelNeuronalActivity) {
                drawCircle(imageData, imageSize, cellImagePos, float3{0.3f, 0.3f, 0.3f}, radius, shadedCells);
            }

            //draw detonation
            if (cell->cellFunction == CellFunction_Detonator) {
                auto const& detonator = cell->cellFunctionData.detonator;
                if (detonator.state == DetonatorState_Activated && detonator.countdown < 2) {
                    auto radius = toFloat((timestep - cell->executionOrderNumber + 5) % 6 + (6 - detonator.countdown * 6));
                    radius *= radius;
                    radius *=  cudaSimulationParameters.cellFunctionDetonatorRadius[cell->color] * zoom / 36;
                    drawCircle(
                        imageData,
                        imageSize,
                        cellImagePos,
                        float3{0.3f, 0.3f, 0.0f},
                        radius,
                        shadedCells);
                }
            }

            //draw connections
            if (zoom >= ZoomLevelForConnections) {
                for (int i = 0; i < cell->numConnections; ++i) {
                    auto const otherCell = cell->connections[i].cell;
                    auto otherCellPos = otherCell->pos;
                    auto topologyCorrection = map.getCorrectionIncrement(cellPos, otherCellPos);
                    otherCellPos += topologyCorrection;

                    //if (Math::lengthSquared(topologyCorrection) < NEAR_ZERO) {
                    auto distFromCellCenter = Math::normalized(otherCellPos - cellPos) / 3;
                    auto const startImagePos = mapWorldPosToVectorImagePos(rectUpperLeft, cellPos + distFromCellCenter, universeImageSize, imageSize, zoom);
                    auto const endImagePos =
                        mapWorldPosToVectorImagePos(rectUpperLeft, otherCellPos - distFromCellCenter, universeImageSize, imageSize, zoom);
                    if (isLineVisible(startImagePos, endImagePos, universeImageSize)) {
                        drawLine(startImagePos, endImagePos, color, imageData, imageSize);
                    }
                    //}
                }
            }

            //draw arrows
            if (zoom >= ZoomLevelForArrows) {
                auto inputExecutionOrderNumber = cell->inputExecutionOrderNumber;
                if (inputExecutionOrderNumber != -1 && inputExecutionOrderNumber != cell->executionOrderNumber) {
                    for (int i = 0; i < cell->numConnections; ++i) {
                        auto const& otherCell = cell->connections[i].cell;
                        if (otherCell->executionOrderNumber == inputExecutionOrderNumber && !otherCell->outputBlocked) {
                            auto otherCellPos = otherCell->pos;
                            auto topologyCorrection = map.getCorrectionIncrement(cellPos, otherCellPos);
                            otherCellPos += topologyCorrection;

                            //if (Math::lengthSquared(topologyCorrection) > NEAR_ZERO) {
                            //    continue;
                            //}

                            auto const otherCellImagePos = mapWorldPosToVectorImagePos(rectUpperLeft, otherCellPos, universeImageSize, imageSize, zoom);
                            if (!isContainedInRect({0, 0}, toFloat2(imageSize), otherCellImagePos)) {
                                continue;
                            }
                            auto const arrowEnd = mapWorldPosToVectorImagePos(
                                rectUpperLeft, cellPos + Math::normalized(otherCellPos - cellPos) / 3, universeImageSize, imageSize, zoom);
                            if (!isContainedInRect({0, 0}, toFloat2(imageSize), arrowEnd)) {
                                continue;
                            }
                            auto direction = Math::normalized(arrowEnd - otherCellImagePos);
                            {
                                float2 arrowPartStart = {-direction.x + direction.y, -direction.x - direction.y};
                                arrowPartStart = arrowPartStart * zoom / 8 + arrowEnd;
                                if (isLineVisible(arrowPartStart, arrowEnd, universeImageSize)) {
                                    drawLine(arrowPartStart, arrowEnd, color, imageData, imageSize, 0.5f);
                                }
                            }
                            {
                                float2 arrowPartStart = {-direction.x - direction.y, direction.x - direction.y};
                                arrowPartStart = arrowPartStart * zoom / 8 + arrowEnd;
                                if (isLineVisible(arrowPartStart, arrowEnd, universeImageSize)) {
                                    drawLine(arrowPartStart, arrowEnd, color, imageData, imageSize, 0.5f);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

__global__ void
cudaDrawParticles(int2 worldSize, float2 rectUpperLeft, float2 rectLowerRight, Array<Particle*> particles, uint64_t* imageData, int2 imageSize, float zoom)
{
    BaseMap map;
    map.init(worldSize);

    auto const partition = calcPartition(particles.getNumEntries(), threadIdx.x + blockIdx.x * blockDim.x, blockDim.x * gridDim.x);

    auto universeImageSize = toFloat2(worldSize) * zoom;
    for (int index = partition.startIndex; index <= partition.endIndex; ++index) {
        auto const& particle = particles.at(index);
        auto particlePos = particle->absPos;
        map.correctPosition(particlePos);

        auto const particleImagePos = mapWorldPosToVectorImagePos(rectUpperLeft, particlePos, universeImageSize, imageSize, zoom);
        if (isContainedInRect({0, 0}, imageSize, particleImagePos)) {
            auto const color = calcColor(particle, 0 != particle->selected);
            auto radius = zoom / 3;
            drawCircle(imageData, imageSize, particleImagePos, color, radius);
        }
    }
}

__global__ void cudaDrawRadiationSources(uint64_t* targetImage, float2 rectUpperLeft, int2 worldSize, int2 imageSize, float zoom)
{
    auto universeImageSize = toFloat2(worldSize) * zoom;
    for (int i = 0; i < cudaSimulationParameters.numParticleSources; ++i) {
        float2 sourcePos{cudaSimulationParameters.particleSources[i].posX, cudaSimulationParameters.particleSources[i].posY};
        auto imagePos = mapWorldPosToVectorImagePos(rectUpperLeft, sourcePos, universeImageSize, imageSize, zoom);
        if (isContainedInRect({0, 0}, toFloat2(imageSize), imagePos)) {
            for (int dx = -5; dx <= 5; ++dx) {
                auto drawX = toInt(imagePos.x) + dx;
                auto drawY = toInt(imagePos.y);
                if (0 <= drawX && drawX < imageSize.x && 0 <= drawY && drawY < imageSize.y) {
                    int index = drawX + drawY * imageSize.x;
                    targetImage[index] = 0x0000000001ff;
                }
            }
            for (int dy = -5; dy <= 5; ++dy) {
                auto drawX = toInt(imagePos.x);
                auto drawY = toInt(imagePos.y) + dy;
                if (0 <= drawX && drawX < imageSize.x && 0 <= drawY && drawY < imageSize.y) {
                    int index = drawX + drawY * imageSize.x;
                    targetImage[index] = 0x0000000001ff;
                }
            }
        }
    }
}

__global__ void cudaDrawRepetition(int2 worldSize, int2 imageSize, float2 rectUpperLeft, float2 rectLowerRight, uint64_t* imageData, float zoom)
{
    BaseMap map;
    map.init(worldSize);

    auto universeImageSize = toFloat2(worldSize) * zoom;
    auto const partition = calcAllThreadsPartition(imageSize.x * imageSize.y);
    for (int index = partition.startIndex; index <= partition.endIndex; ++index) {
        auto x = index % imageSize.x;
        auto y = index / imageSize.x;
        if (x >= toInt(universeImageSize.x) || y >= toInt(universeImageSize.y)) {
            float2 worldPos = {toFloat(x) / zoom + rectUpperLeft.x, toFloat(y) / zoom + rectUpperLeft.y};

            auto refPos = mapWorldPosToVectorImagePos(rectUpperLeft, worldPos, universeImageSize, imageSize, zoom);
            int2 refIntPos{toInt(refPos.x), toInt(refPos.y)};
            if (refIntPos.x >= 0 && refIntPos.x < imageSize.x && refIntPos.y >= 0 && refIntPos.y < imageSize.y) {
                alienAtomicExch64(&imageData[index], imageData[refIntPos.x + refIntPos.y * imageSize.x]);
            }
        }
    }
}

