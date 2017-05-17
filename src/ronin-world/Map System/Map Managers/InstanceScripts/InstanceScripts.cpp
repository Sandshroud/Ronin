/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "033_shadowfang_keep.h"
#include "034_the_stockades.h"
#include "036_deadmines.h"
#include "043_wailing_caverns.h"
#include "044_monastery.h"
#include "047_razorfen_kraul.h"
#include "048_blackfathom_deeps.h"
#include "070_uldaman.h"
#include "090_gnomergan.h"
#include "109_sunken_temple.h"
#include "129_razorfen_downs.h"
#include "169_emerald_dream.h"
#include "189_scarlet_monastery.h"
#include "209_zulfarrak.h"
#include "229_blackrock_spire.h"
#include "230_blackrock_depths.h"
#include "249_onyxias_lair.h"
#include "269_the_black_morass.h"
#include "289_scholomance.h"
#include "309_zul_gurub.h"
#include "329_stratholme.h"
#include "349_maraudon.h"
#include "389_ragefire_chasm.h"
#include "409_molten_core.h"
#include "429_dire_maul.h"
#include "469_blackwing_lair.h"
#include "509_ruins_of_ahnqiraj.h"
#include "531_ahnqiraj_temple.h"
#include "532_karazhan.h"
#include "533_naxxramas.h"
#include "534_the_battle_for_mount_hyjal.h"
#include "540_hellfire_citadel_the_shattered_halls.h"
#include "542_hellfire_citadel_the_blood_furnace.h"
#include "543_hellfire_citadel_ramparts.h"
#include "544_magtheridons_lair.h"
#include "545_coilfang_the_steamvault.h"
#include "546_coilfang_the_underbog.h"
#include "547_coilfang_the_slave_pens.h"
#include "548_coilfang_serpentshrine_cavern.h"
#include "550_tempest_keep.h"
#include "552_tempest_keep_the_arcatraz.h"
#include "553_tempest_keep_the_botanica.h"
#include "554_tempest_keep_the_mechanar.h"
#include "555_auchindoun_shadow_labyrinth.h"
#include "556_auchindoun_sethekk_halls.h"
#include "557_auchindoun_mana_tombs.h"
#include "558_auchindoun_auchenai_crypts.h"
#include "560_old_hillsbrad_foothills.h"
#include "564_black_temple.h"
#include "565_gruuls_lair.h"
#include "568_zulaman.h"
#include "574_utgarde_keep.h"
#include "575_utgarde_pinnacle.h"
#include "576_the_nexus.h"
#include "578_the_oculus.h"
#include "580_the_sunwell.h"
#include "585_magisters_terrace.h"
#include "595_the_culling_of_stratholme.h"
#include "598_sunwell.h"
#include "599_halls_of_stone.h"
#include "600_draktharon_keep.h"
#include "601_azjol_nerub.h"
#include "602_halls_of_lightning.h"
#include "603_ulduar.h"
#include "604_gundrak.h"
#include "608_violet_hold.h"
#include "615_the_obsidian_sanctum.h"
#include "616_the_eye_of_eternity.h"
#include "619_ahnkahet_the_old_kingdom.h"
#include "624_wintergrasp_raid.h"
#include "631_icecrown_citadel.h"
#include "632_the_forge_of_souls.h"
#include "649_trial_of_the_crusader.h"
#include "650_trial_of_the_champion.h"
#include "658_pit_of_saron.h"
#include "668_halls_of_reflection.h"
#include "724_ruby_sanctum.h"

void InstanceManager::SetupInstanceScripts()
{
    SetupShadowfangKeep();
}
