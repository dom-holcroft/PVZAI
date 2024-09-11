#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <array>
#include <vector>
#include <tuple>
#include <iterator>
#include <algorithm>
#include <map>
#include <toml++/toml.h>

#include "pvz.h"
#include "memoryLocator.h"
#include "machinecode.h"
#include "memoryconfig.h"
#include "util.h"
#include "constants.h"

struct ConfigValues
{
    std::vector<int> seeds;
    std::unordered_map<std::string, int> zombieRewards;
    int speedUp;
    int actionInterval;
    int unsuccessfulPlantReward;
    int plantKilledReward;
    double survivalReward;
};

HANDLE getProcessByName(const char *processName)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE)
    {
        printf("Failed to create snapshot. Error: %lu\n", GetLastError());
        return NULL;
    }

    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (strcmp(processName, entry.szExeFile) == 0)
            {
                HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
                CloseHandle(snapshot);

                if (processHandle == NULL)
                {
                    printf("Failed to open process. Error: %lu\n", GetLastError());
                }

                return processHandle;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return NULL;
}

std::vector<int> parseSeeds(const toml::table &configTOML)
{
    std::vector<int> chosenSeeds;
    auto seedsTable = configTOML["seeds"].as_table();
    if (!seedsTable)
    {
        throw std::runtime_error("seeds is not a table");
    }

    for (const auto &[slot, seedName] : *seedsTable)
    {
        std::optional<std::string> seedString = seedName.value<std::string>();
        if (seedString.has_value())
        {
            auto it = PLANTS.find(seedString.value());
            if (it != PLANTS.end())
            {
                chosenSeeds.push_back(it->second);
            }
            else
            {
                std::cerr << "Seed not in list: " << seedString.value() << std::endl;
                throw std::runtime_error("Invalid seed");
            }
        }
    }
    return chosenSeeds;
}

std::unordered_map<std::string, int> parseZombieRewards(const toml::table &configTOML)
{
    std::unordered_map<std::string, int> chosenZombieRewards;
    auto zombieRewardsTable = configTOML["zombie_rewards"].as_table();
    if (!zombieRewardsTable)
    {
        throw std::runtime_error("zombie_rewards is not a table");
    }

    size_t index = 0;
    for (const auto &[zombieType, reward] : *zombieRewardsTable)
    {
        std::string possibleZombieName = POSSIBLE_ZOMBIES[index];
        std::optional<int> rewardInt = reward.value<int>();
        if (rewardInt)
        {
            chosenZombieRewards.emplace(std::make_pair(possibleZombieName, rewardInt.value()));
            ++index;
        }
        else
        {
            throw std::runtime_error("All zombie rewards must be ints");
        }
    }
    return chosenZombieRewards;
}

ConfigValues getConfigValues()
{
    toml::table configTOML;
    ConfigValues configValues;

    try
    {
        configTOML = toml::parse_file(CONFIG_FILE_NAME);
    }
    catch (const toml::parse_error &err)
    {
        std::cerr << "Parsing failed:\n"
                  << err << "\n";
        throw std::runtime_error("Failed to parse configuration file.");
    }

    configValues.speedUp = configTOML["timings"]["speed_up"].value_or(1);
    configValues.actionInterval = configTOML["timings"]["action_interval"].value_or(1);

    configValues.unsuccessfulPlantReward = configTOML["rewards"]["unsuccessful_plant_reward"].value_or(1);
    configValues.plantKilledReward = configTOML["rewards"]["plant_killed_reward"].value_or(1);

    configValues.survivalReward = configTOML["rewards"]["survival_reward"].value<double>().value_or(0.01);

    configValues.seeds = parseSeeds(configTOML);
    configValues.zombieRewards = parseZombieRewards(configTOML);

    return configValues;
}

int getSunValue(HANDLE hProcess)
{
    return MemoryConfig::memoryAddresses.sunValue.getInt(hProcess);
}

std::vector<int> getPlantRecharges(HANDLE hProcess, std::vector<uint8_t> seedSlotData)
{
    std::vector<int> seedRechargeTimes;

    const ptrdiff_t seedTotalRechargeOffset = MemoryConfig::offsets.seedRechargeTime;
    const ptrdiff_t seedCurrentRechargeOffset = MemoryConfig::offsets.seedCurrentRecharge;
    size_t currentSeed;

    int totalRechargeTime;
    int currentRechargeTime;
    int remainingRecharge;

    for (size_t i = 0; i < NUMBEROFSEEDS; ++i)
    {
        currentSeed = i * MemoryConfig::offsets.seedNext;
        totalRechargeTime = *reinterpret_cast<DWORD *>(&seedSlotData[currentSeed + seedTotalRechargeOffset]);
        currentRechargeTime = *reinterpret_cast<DWORD *>(&seedSlotData[currentSeed + seedCurrentRechargeOffset]);

        remainingRecharge = (currentRechargeTime == 0) ? 0 : (totalRechargeTime - currentRechargeTime);
        seedRechargeTimes.push_back(remainingRecharge);
    }
    return seedRechargeTimes;
}

std::vector<int> getPlantPrices(HANDLE hProcess, std::vector<uint8_t> seedSlotData)
{
    std::vector<int> seedSunCost;
    std::vector<uint8_t> sunCostData;
    int currentSeedType;
    int currentSeedCost;
    size_t currentSeed;
    const size_t blockSize = MAXPLANTIDNUMBER * 36;
    const ptrdiff_t seedTypeOffset = MemoryConfig::offsets.seedType;

    sunCostData = MemoryConfig::memoryAddresses.sunCosts.getData(
        hProcess, blockSize);

    for (int i = 0; i < NUMBEROFSEEDS; ++i)
    {
        currentSeed = i * MemoryConfig::offsets.seedNext;
        currentSeedType = *reinterpret_cast<DWORD *>(&seedSlotData[currentSeed + seedTypeOffset]);
        currentSeedCost = *reinterpret_cast<DWORD *>(&sunCostData[36 * currentSeedType]);
        seedSunCost.push_back(currentSeedCost);
    }
    return seedSunCost;
}

std::array<std::array<std::array<int, 10>, 5>, 10> getPlantsOnGrid(HANDLE hProcess, std::vector<int> &seedIDs)
{
    const size_t blockSize = MemoryConfig::offsets.plantNext * 100;
    std::array<std::array<std::array<int, 10>, 5>, 10> plantGrid{{{}}};

    std::vector<uint8_t> plantData;
    plantData = MemoryConfig::memoryAddresses.plant.getData(
        hProcess, blockSize);
    const unsigned int numberOfPlants = MemoryConfig::memoryAddresses.plantNumber.getInt(hProcess);
    size_t plantCol, plantRow, plantType, seedPosition;
    size_t currentPlant = -MemoryConfig::offsets.plantNext;

    for (size_t i = 0; i < numberOfPlants; ++i)
    {

        seedPosition = 0;
        do
        {
            currentPlant += MemoryConfig::offsets.plantNext;
        } while (*reinterpret_cast<uint8_t *>(&plantData[currentPlant + MemoryConfig::offsets.plantDead]));

        plantCol = *reinterpret_cast<DWORD *>(&plantData[currentPlant + MemoryConfig::offsets.plantCol]);
        plantRow = *reinterpret_cast<DWORD *>(&plantData[currentPlant + MemoryConfig::offsets.plantRow]);
        plantType = *reinterpret_cast<DWORD *>(&plantData[currentPlant + MemoryConfig::offsets.plantType]);
        if (plantRow == 4294967295 || plantCol == 4294967295)
            continue;

        auto it = std::find(seedIDs.begin(), seedIDs.end(), plantType);
        if (it == seedIDs.end())
        {
            std::cerr << "Plant not in seed list, this should be impossible" << std::endl;
        }
        else
        {
            seedPosition = std::distance(seedIDs.begin(), it);
            plantGrid[seedPosition][plantRow][plantCol] = 1;
        }
    }
    return plantGrid;
}

std::array<std::array<std::array<int, 10>, 5>, POSSIBLE_ZOMBIES.size()> getZombiesOnGrid(HANDLE hProcess)
{

    std::array<std::array<std::array<int, 10>, 5>, POSSIBLE_ZOMBIES.size()> zombieGrid{{{}}};
    const size_t zombieMaxNumber = MemoryConfig::memoryAddresses.zombieMaxNumber.getInt(hProcess);
    size_t blockSize = MemoryConfig::offsets.zombieNext * zombieMaxNumber;
    std::vector<uint8_t> zombieData;
    zombieData = MemoryConfig::memoryAddresses.zombie.getData(
        hProcess, blockSize);

    float zombieXPos, zombieYPos;
    size_t currentZombie, zombieType, encodedZombieType, zombieRow, zombieCol;

    const unsigned int numberOfZombies = MemoryConfig::memoryAddresses.zombieNumber.getInt(hProcess);

    for (size_t i = 0; i < zombieMaxNumber; ++i)
    {
        currentZombie = i * MemoryConfig::offsets.zombieNext;
        if (*reinterpret_cast<uint8_t *>(&zombieData[currentZombie + MemoryConfig::offsets.zombieDead]))
        {
            continue;
        }

        zombieXPos = *reinterpret_cast<float *>(&zombieData[currentZombie + MemoryConfig::offsets.zombieXPos]);
        zombieYPos = *reinterpret_cast<float *>(&zombieData[currentZombie + MemoryConfig::offsets.zombieYPos]);
        zombieType = *reinterpret_cast<DWORD *>(&zombieData[currentZombie + MemoryConfig::offsets.zombieType]);

        zombieCol = static_cast<size_t>(std::floor((zombieXPos + 30) / 80));
        if (zombieCol <= 9)
        {
            zombieRow = static_cast<size_t>(std::floor(zombieYPos / 100));
            auto it = std::find(POSSIBLE_ZOMBIES.begin(), POSSIBLE_ZOMBIES.end(), ZOMBIES[zombieType]);
            if (it == POSSIBLE_ZOMBIES.end())
            {
                std::cerr << "Zombie not in possible zombies list, this should be impossible" << std::endl;
            }
            else
            {
                encodedZombieType = std::distance(POSSIBLE_ZOMBIES.begin(), it);
                zombieGrid[encodedZombieType][zombieRow][zombieCol] = 1;
            }
        }
    }
    return zombieGrid;
}

double getReward(HANDLE hProcess, DWORD rewardAddress, double survivalReward)
{
    double reward = readInt(hProcess, rewardAddress);
    reward += survivalReward;
    writeInt(hProcess, 0, rewardAddress);
    return reward;
}

void changePauseState(HANDLE hProcess, bool state)
{
    DWORD gamePauesAddress = MemoryConfig::memoryAddresses.gamePaused.getAddressFromOffsets(hProcess);
    if (state)
    {
        writeByte(hProcess, 1, gamePauesAddress);
    }
    else
    {
        writeByte(hProcess, 0, gamePauesAddress);
    }
}

GameValues getGameValues(HANDLE hProcess, DWORD restartFlagAddress, std::vector<int> seedIDs)
{
    if (isGameOver(hProcess, restartFlagAddress) || !isGameRunning(hProcess))
    {
        GameValues gameValues;
        gameValues.gameOver = true;
        changePauseState(hProcess, false);
        return gameValues;
    }
    std::array<std::array<std::array<int, 10>, 5>, 10> plantsOnGrid;
    std::array<std::array<std::array<int, 10>, 5>, POSSIBLE_ZOMBIES.size()> zombiesOnGrid;

    const size_t blockSize = MemoryConfig::offsets.seedNext * (NUMBEROFSEEDS + 1);
    std::vector<uint8_t> seedSlotData;
    seedSlotData = MemoryConfig::memoryAddresses.seedSlot.getData(
        hProcess, blockSize);

    std::vector<int> inputInformation;
    int sunValue = getSunValue(hProcess);

    std::vector<int> plantRecharges = getPlantRecharges(hProcess, seedSlotData);
    std::vector<int> plantPrices = getPlantPrices(hProcess, seedSlotData);
    plantsOnGrid = getPlantsOnGrid(hProcess, seedIDs);
    zombiesOnGrid = getZombiesOnGrid(hProcess);

    auto combinedGrid = merge3DArray(plantsOnGrid, zombiesOnGrid);

    inputInformation.reserve(1 + plantRecharges.size() + plantPrices.size());
    inputInformation.push_back(sunValue);
    inputInformation.insert(inputInformation.end(), plantRecharges.begin(), plantRecharges.end());
    inputInformation.insert(inputInformation.end(), plantPrices.begin(), plantPrices.end());

    return GameValues{inputInformation, combinedGrid};
}

bool isGameRunning(HANDLE hProcess)
{
    const int scene = MemoryConfig::memoryAddresses.sceneType.getInt(hProcess);
    return (scene == 3);
}

int playStep(HANDLE hProcess, int actionInterval, int previousTime)
{
    int delay = actionInterval * 100;
    changePauseState(hProcess, false);
    while (isGameRunning(hProcess))
    {

        int currentGameSpeed = MemoryConfig::memoryAddresses.gameClock.getInt(hProcess);

        if (currentGameSpeed - previousTime >= delay)
        {
            changePauseState(hProcess, false);
            return currentGameSpeed;
        }
    }

    changePauseState(hProcess, false);
    return 0;
}

void restartGame(HANDLE hProcess, DWORD restartAddress)
{
    writeInt(hProcess, 0, restartAddress);
    while (isGameRunning(hProcess))
    {
        ;
    }
    return;
}

int isGameOver(HANDLE hProcess, DWORD restartAddress)
{
    return readInt(hProcess, restartAddress);
}

void placePlant(HANDLE process,
                MemoryConfig::PlacePlantAddresses *placePlantAddresses,
                int col, int row, int seedSlot)
{
    if (isGameRunning(process))
    {
        //LPTHREAD_START_ROUTINE plantFuncAddress = reinterpret_cast<LPTHREAD_START_ROUTINE>(placePlantAddresses->placePlantFunctionAddress);
        writeInt(process, col * 80 + 60, placePlantAddresses->plantColAddress);
        writeInt(process, row * 130 + 80, placePlantAddresses->plantRowAddress);
        writeInt(process, seedSlot, placePlantAddresses->seedSlotAddress);
        //HANDLE hThread = CreateRemoteThread(process, NULL, 0, plantFuncAddress, NULL, 0, NULL);
        //WaitForSingleObject(hThread, INFINITE);
        //CloseHandle(hThread);
    }
}

SetupCodeInjectionReturn setupCodeInjection(HANDLE process, MemoryConfig::PlacePlantAddresses *placePlantAddresses)
{
    ConfigValues configValues{getConfigValues()};

    DWORD rewardAddress = createVariable(process);
    DWORD restartFlagAddress = createVariable(process);
    DWORD xCoordAddress = createVariable(process);
    DWORD yCoordAddress = createVariable(process);
    DWORD seedSlotAddress = createVariable(process);
    DWORD timerAddress = createVariable(process);
    DWORD limitAddress = createVariable(process);

    placePlantAddresses->plantColAddress = xCoordAddress;
    placePlantAddresses->plantRowAddress = yCoordAddress;
    placePlantAddresses->seedSlotAddress = seedSlotAddress;

    DWORD gameSpeedAddress = MemoryConfig::memoryAddresses.gameSpeed.getAddressFromOffsets(process);
    writeDouble(process, configValues.speedUp, gameSpeedAddress);
    writeInt(process, 5, limitAddress);
    writeInt(process, 0, timerAddress);

    writeInt(process, 10, 0x7320BC);
    writeInt(process, 10, 0x7320C0);
    writeInt(process, 10, 0x7320A8);
    writeInt(process, 0, 0x7320A4);
    writeInt(process, 0, 0x7320B4);
    writeInt(process, 10, 0x7320B8);
    writeInt(process, 20, 0x7320AC);
    writeInt(process, 30, 0x7320B0);
    writeInt(process, 20, 0x7220C4);
    writeInt(process, 30, 0x7320C8);

    writeInt(process, 0, restartFlagAddress);

    std::vector<uint8_t> moveCameraToLawnBytes = getMoveCameraToLawnBytes();
    overwriteBytes(process, moveCameraToLawnBytes, 0x43FFD2);

    std::vector<uint8_t> changeCameraStartPositionBytes = getChangeCameraStartPositionBytes();
    overwriteBytes(process, changeCameraStartPositionBytes, 0x43FE56);

    std::vector<uint8_t> removeMoveCameraAtFlagEnd = getRemoveMoveCameraAtFlagEnd();
    overwriteBytes(process, removeMoveCameraAtFlagEnd, 0x43FE63);

    std::vector<uint8_t> lawnmowerStartTimeBytes = getLawnmowerStartSameTimeBytes();
    overwriteBytes(process, lawnmowerStartTimeBytes, 0x440446);

    std::vector<uint8_t> instantLawnmowerBytes = getInstantLawnmowerBytes();
    overwriteBytes(process, instantLawnmowerBytes, 0x4403F0);

    std::vector<uint8_t> startGameEarlierBytes = getStartGameEarlierBytes();
    overwriteBytes(process, startGameEarlierBytes, 0x4408E0);

    std::vector<uint8_t> restartLevelFunctionBytes = getRestartLevelFunction(restartFlagAddress);
    DWORD restartLevelFunctionAddress = createNewFunction(process, restartLevelFunctionBytes);
    std::vector<uint8_t> lawnmowerHookBytes = getLawnmowerHookBytes(restartLevelFunctionAddress);
    overwriteBytes(process, lawnmowerHookBytes, 0x45e765);

    std::vector<uint8_t> placePlantFunctionBytes = getPlacePlantFunctionBytes(restartFlagAddress, seedSlotAddress, xCoordAddress, yCoordAddress);
    DWORD placePlantFunctionAddress = createNewFunction(process, placePlantFunctionBytes);
    std::vector<uint8_t> placePlantHookBytes = getPlacePlantHookBytes(placePlantFunctionAddress);
    overwriteBytes(process, placePlantHookBytes, 0x4618E0);

    std::vector<uint8_t> choosePlantFunctionBytes = getChoosePlantFunctionBytes();
    DWORD choosePlantFunctionAddress = createNewFunction(process, choosePlantFunctionBytes);

    std::vector<uint8_t> addPlantFunctionBytes = getAddPlantFunctionBytes();
    DWORD addPlantFunctionAddress = createNewFunction(process, addPlantFunctionBytes);

    std::vector<uint8_t> letsRockFunctionBytes = getLetsRockFunctionBytes();
    DWORD letsRockFunctionAddress = createNewFunction(process, letsRockFunctionBytes);

    std::vector<uint8_t> startRoundFunctionBytes = getStartRoundFunction(configValues.seeds, choosePlantFunctionAddress, letsRockFunctionAddress, addPlantFunctionAddress);
    DWORD startRoundFunctionAddress = createNewFunction(process, startRoundFunctionBytes);

    std::vector<uint8_t> timerFunctionBytes = getTimerFunctionBytes(timerAddress, limitAddress, startRoundFunctionAddress);
    DWORD timerFunctionAddress = createNewFunction(process, timerFunctionBytes);
    std::vector<uint8_t> timerHookBytes = getTimerHookBytes(timerFunctionAddress);
    overwriteBytes(process, timerHookBytes, 0x4408E5);

    DWORD zombieRewardFunctionAddress = allocateMemory(process, 1024);
    std::vector<uint8_t> zombieRewardFunctionBytes = getZombieRewardFunctionBytes(zombieRewardFunctionAddress, rewardAddress, configValues.zombieRewards);
    overwriteBytes(process, zombieRewardFunctionBytes, zombieRewardFunctionAddress);
    std::vector<uint8_t> zombieRewardHookBytes = getZombieRewardHookBytes(zombieRewardFunctionAddress);
    overwriteBytes(process, zombieRewardHookBytes, 0x545113);

    std::vector<uint8_t> plantKilledFunctionBytes = getPlantKilledFunctionBytes(rewardAddress, configValues.plantKilledReward);
    DWORD plantKilledFunctionAddress = createNewFunction(process, plantKilledFunctionBytes);
    std::vector<uint8_t> plantKilledHookBytes = getPlantKilledHookBytes(plantKilledFunctionAddress);
    overwriteBytes(process, plantKilledHookBytes, 0x468F5A);

    std::vector<uint8_t> spaceOccupiedHookBytes = getSpaceOccupiedHookBytes(rewardAddress, configValues.unsuccessfulPlantReward);
    overwriteBytes(process, spaceOccupiedHookBytes, 0x413C4C);

    std::vector<uint8_t> notEnoughSunHookBytes = getNotEnoughSunHookBytes(rewardAddress, configValues.unsuccessfulPlantReward);
    overwriteBytes(process, notEnoughSunHookBytes, 0x41F664);

    placePlantAddresses->placePlantFunctionAddress = placePlantFunctionAddress;
    SetupCodeInjectionReturn setupCodeInjectionReturn = {
        rewardAddress,
        configValues.actionInterval,
        configValues.survivalReward,
        restartFlagAddress,
        configValues.seeds};
    return setupCodeInjectionReturn;
}
