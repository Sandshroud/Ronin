/***
 * Demonstrike Core
 */

#pragma once

class HonorHandler
{
public:
    static int32 CalculateHonorPoints(uint32 playerLevel, uint32 victimLevel);
    static void RecalculateHonorFields(Player* pPlayer);
    static void AddHonorPointsToPlayer(Player* pPlayer, uint32 uAmount);
    static void OnPlayerKilled(Player* pPlayer, Player* pVictim);
};
