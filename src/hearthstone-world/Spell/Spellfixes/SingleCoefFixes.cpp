/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void ApplyCoeffSpellFixes(SpellEntry *sp)
{
    switch(sp->Id)
    {
    case 17: // Power Word: Shield - Rank 1
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 116: // Frostbolt - Rank 1
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 120: // Cone of Cold - Rank 1
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 122: // Frost Nova - Rank 1
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 133: // Fireball - Rank 1
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 324: // Lightning Shield - Rank 1
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 325: // Lightning Shield - Rank 2
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 331: // Healing Wave - Rank 1
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 332: // Healing Wave - Rank 2
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 339: // Entangling Roots - Rank 1
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 348: // Immolate - Rank 1
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 403: // Lightning Bolt - Rank 1
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 421: // Chain Lightning - Rank 1
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 529: // Lightning Bolt - Rank 2
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 547: // Healing Wave - Rank 3
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 548: // Lightning Bolt - Rank 3
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 585: // Smite - Rank 1
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 589: // Shadow Word: Pain - Rank 1
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 591: // Smite - Rank 2
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 592: // Power Word: Shield - Rank 2
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 594: // Shadow Word: Pain - Rank 2
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 596: // Prayer of Healing - Rank 1
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 598: // Smite - Rank 3
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 600: // Power Word: Shield - Rank 3
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 603: // Curse of Doom - Rank 1
        {
            sp->SP_coef_override = float(2.000000f);
        }break;
    case 635: // Holy Light - Rank 1
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 639: // Holy Light - Rank 2
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 647: // Holy Light - Rank 3
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 686: // Shadow Bolt - Rank 1
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 689: // Drain Life - Rank 1
        {
            sp->SP_coef_override = float(0.143000f);
        }break;
    case 695: // Shadow Bolt - Rank 2
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 699: // Drain Life - Rank 2
        {
            sp->SP_coef_override = float(0.143000f);
        }break;
    case 703: // Garrote - Rank 1
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 705: // Shadow Bolt - Rank 3
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 707: // Immolate - Rank 2
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 709: // Drain Life - Rank 3
        {
            sp->SP_coef_override = float(0.143000f);
        }break;
    case 740: // Tranquility - Rank 1
        {
            sp->SP_coef_override = float(0.538000f);
        }break;
    case 755: // Health Funnel - Rank 1
        {
            sp->SP_coef_override = float(0.448500f);
        }break;
    case 774: // Rejuvenation - Rank 1
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 837: // Frostbolt - Rank 3
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 879: // Exorcism - Rank 1
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 905: // Lightning Shield - Rank 3
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 913: // Healing Wave - Rank 4
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 915: // Lightning Bolt - Rank 4
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 930: // Chain Lightning - Rank 2
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 943: // Lightning Bolt - Rank 5
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 945: // Lightning Shield - Rank 4
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 970: // Shadow Word: Pain - Rank 3
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 974: // Earth Shield - Rank 1
        {
            sp->SP_coef_override = float(0.476100f);
        }break;
    case 980: // Curse of Agony - Rank 1
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 984: // Smite - Rank 4
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 992: // Shadow Word: Pain - Rank 4
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 996: // Prayer of Healing - Rank 2
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 1004: // Smite - Rank 5
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 1014: // Curse of Agony - Rank 2
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 1026: // Holy Light - Rank 4
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 1042: // Holy Light - Rank 5
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 1058: // Rejuvenation - Rank 2
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 1062: // Entangling Roots - Rank 2
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 1064: // Chain Heal - Rank 1
        {
            sp->SP_coef_override = float(0.800000f);
        }break;
    case 1079: // Rip - Rank 1
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 1088: // Shadow Bolt - Rank 4
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 1094: // Immolate - Rank 3
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 1106: // Shadow Bolt - Rank 5
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 1120: // Drain Soul - Rank 1
        {
            sp->SP_coef_override = float(0.429000f);
        }break;
    case 1430: // Rejuvenation - Rank 3
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 1449: // Arcane Explosion - Rank 1
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 1463: // Mana Shield - Rank 1
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 1495: // Mongoose Bite - Rank 1
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 1776: // Gouge
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 1777: // Gouge - Rank 2
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 1949: // Hellfire - Rank 1
        {
            sp->SP_coef_override = float(0.120000f);
        }break;
    case 1978: // Serpent Sting - Rank 1
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 2060: // Greater Heal - Rank 1
        {
            sp->SP_coef_override = float(1.613500f);
        }break;
    case 2061: // Flash Heal - Rank 1
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 2090: // Rejuvenation - Rank 4
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 2091: // Rejuvenation - Rank 5
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 2120: // Flamestrike - Rank 1
        {
            sp->SP_coef_override = float(0.122000f);
        }break;
    case 2121: // Flamestrike - Rank 2
        {
            sp->SP_coef_override = float(0.122000f);
        }break;
    case 2136: // Fire Blast - Rank 1
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 2137: // Fire Blast - Rank 2
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 2138: // Fire Blast - Rank 3
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 2643: // Multi-Shot - Rank 1
        {
            sp->RAP_coef_override = float(0.200000f);
        }break;
    case 2767: // Shadow Word: Pain - Rank 5
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 2812: // Holy Wrath - Rank 1
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 2818: // Deadly Poison - Rank 1
        {
            sp->AP_coef_override = float(0.024000f);
        }break;
    case 2819: // Deadly Poison II - Rank 2
        {
            sp->AP_coef_override = float(0.024000f);
        }break;
    case 2860: // Chain Lightning - Rank 3
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 2912: // Starfire - Rank 1
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 2941: // Immolate - Rank 4
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 2944: // Devouring Plague - Rank 1
        {
            sp->SP_coef_override = float(0.184900f);
        }break;
    case 2948: // Scorch - Rank 1
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 3009: // Claw - Rank 8
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 3010: // Claw - Rank 7
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 3044: // Arcane Shot - Rank 1
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 3140: // Fireball - Rank 4
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 3472: // Holy Light - Rank 6
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 3606: // Attack - Rank 1
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 3627: // Rejuvenation - Rank 6
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 3698: // Health Funnel - Rank 2
        {
            sp->SP_coef_override = float(0.448500f);
        }break;
    case 3699: // Health Funnel - Rank 3
        {
            sp->SP_coef_override = float(0.448500f);
        }break;
    case 3700: // Health Funnel - Rank 4
        {
            sp->SP_coef_override = float(0.448500f);
        }break;
    case 3747: // Power Word: Shield - Rank 4
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 5143: // Arcane Missiles - Rank 1
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 5144: // Arcane Missiles - Rank 2
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 5145: // Arcane Missiles - Rank 3
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 5176: // Wrath - Rank 1
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 5177: // Wrath - Rank 2
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 5178: // Wrath - Rank 3
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 5179: // Wrath - Rank 4
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 5180: // Wrath - Rank 5
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 5185: // Healing Touch - Rank 1
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 5186: // Healing Touch - Rank 2
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 5187: // Healing Touch - Rank 3
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 5188: // Healing Touch - Rank 4
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 5189: // Healing Touch - Rank 5
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 5195: // Entangling Roots - Rank 3
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 5196: // Entangling Roots - Rank 4
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 5308: // Execute - Rank 1
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 5570: // Insect Swarm - Rank 1
        {
            sp->SP_coef_override = float(0.127000f);
        }break;
    case 5614: // Exorcism - Rank 2
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 5615: // Exorcism - Rank 3
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 5676: // Searing Pain - Rank 1
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 5740: // Rain of Fire - Rank 1
        {
            sp->SP_coef_override = float(0.693200f);
        }break;
    case 6041: // Lightning Bolt - Rank 6
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 6060: // Smite - Rank 6
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 6065: // Power Word: Shield - Rank 5
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 6066: // Power Word: Shield - Rank 6
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 6074: // Renew - Rank 2
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 6075: // Renew - Rank 3
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 6076: // Renew - Rank 4
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 6077: // Renew - Rank 5
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 6078: // Renew - Rank 6
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 6131: // Frost Nova - Rank 3
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 6217: // Curse of Agony - Rank 3
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 6219: // Rain of Fire - Rank 2
        {
            sp->SP_coef_override = float(0.693200f);
        }break;
    case 6222: // Corruption - Rank 2
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 6223: // Corruption - Rank 3
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 6229: // Shadow Ward - Rank 1
        {
            sp->SP_coef_override = float(0.300000f);
        }break;
    case 6343: // Thunder Clap - Rank 1
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 6350: // Attack - Rank 2
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 6351: // Attack - Rank 3
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 6352: // Attack - Rank 4
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 6353: // Soul Fire - Rank 1
        {
            sp->SP_coef_override = float(1.150000f);
        }break;
    case 6572: // Revenge - Rank 1
        {
            sp->AP_coef_override = float(0.207000f);
        }break;
    case 6574: // Revenge - Rank 2
        {
            sp->AP_coef_override = float(0.207000f);
        }break;
    case 6778: // Healing Touch - Rank 6
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 6780: // Wrath - Rank 6
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 6789: // Death Coil - Rank 1
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 7322: // Frostbolt - Rank 4
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 7379: // Revenge - Rank 3
        {
            sp->AP_coef_override = float(0.207000f);
        }break;
    case 7641: // Shadow Bolt - Rank 6
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 7648: // Corruption - Rank 4
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 7651: // Drain Life - Rank 4
        {
            sp->SP_coef_override = float(0.143000f);
        }break;
    case 8004: // Lesser Healing Wave - Rank 1
        {
            sp->SP_coef_override = float(0.808200f);
        }break;
    case 8005: // Healing Wave - Rank 7
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 8008: // Lesser Healing Wave - Rank 2
        {
            sp->SP_coef_override = float(0.808200f);
        }break;
    case 8010: // Lesser Healing Wave - Rank 3
        {
            sp->SP_coef_override = float(0.808200f);
        }break;
    case 8026: // Flametongue Weapon Proc - Rank 1
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 8028: // Flametongue Weapon Proc - Rank 2
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 8029: // Flametongue Weapon Proc - Rank 3
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 8034: // Frostbrand Attack - Rank 1
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 8037: // Frostbrand Attack - Rank 2
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 8042: // Earth Shock - Rank 1
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 8044: // Earth Shock - Rank 2
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 8045: // Earth Shock - Rank 3
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 8046: // Earth Shock - Rank 4
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 8050: // Flame Shock - Rank 1
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 8052: // Flame Shock - Rank 2
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 8053: // Flame Shock - Rank 3
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 8056: // Frost Shock - Rank 1
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 8058: // Frost Shock - Rank 2
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 8092: // Mind Blast - Rank 1
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 8102: // Mind Blast - Rank 2
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 8103: // Mind Blast - Rank 3
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 8104: // Mind Blast - Rank 4
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 8105: // Mind Blast - Rank 5
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 8106: // Mind Blast - Rank 6
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 8134: // Lightning Shield - Rank 5
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 8187: // Magma Totem - Rank 1
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 8198: // Thunder Clap - Rank 2
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 8204: // Thunder Clap - Rank 3
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 8205: // Thunder Clap - Rank 4
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 8288: // Drain Soul - Rank 2
        {
            sp->SP_coef_override = float(0.429000f);
        }break;
    case 8289: // Drain Soul - Rank 3
        {
            sp->SP_coef_override = float(0.429000f);
        }break;
    case 8400: // Fireball - Rank 5
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 8401: // Fireball - Rank 6
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 8402: // Fireball - Rank 7
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 8406: // Frostbolt - Rank 5
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 8407: // Frostbolt - Rank 6
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 8408: // Frostbolt - Rank 7
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 8412: // Fire Blast - Rank 4
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 8413: // Fire Blast - Rank 5
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 8416: // Arcane Missiles - Rank 4
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 8417: // Arcane Missiles - Rank 5
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 8422: // Flamestrike - Rank 3
        {
            sp->SP_coef_override = float(0.122000f);
        }break;
    case 8423: // Flamestrike - Rank 4
        {
            sp->SP_coef_override = float(0.122000f);
        }break;
    case 8437: // Arcane Explosion - Rank 2
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 8438: // Arcane Explosion - Rank 3
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 8439: // Arcane Explosion - Rank 4
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 8444: // Scorch - Rank 2
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 8445: // Scorch - Rank 3
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 8446: // Scorch - Rank 4
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 8492: // Cone of Cold - Rank 2
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 8494: // Mana Shield - Rank 2
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 8495: // Mana Shield - Rank 3
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 8629: // Gouge - Rank 3
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 8631: // Garrote - Rank 2
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 8632: // Garrote - Rank 3
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 8633: // Garrote - Rank 4
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 8680: // Instant Poison - Rank 1
        {
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 8685: // Instant Poison II - Rank 2
        {
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 8689: // Instant Poison III - Rank 3
        {
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 8903: // Healing Touch - Rank 7
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 8905: // Wrath - Rank 7
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 8910: // Rejuvenation - Rank 7
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 8918: // Tranquility - Rank 2
        {
            sp->SP_coef_override = float(0.538000f);
        }break;
    case 8921: // Moonfire - Rank 1
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 8924: // Moonfire - Rank 2
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 8925: // Moonfire - Rank 3
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 8926: // Moonfire - Rank 4
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 8927: // Moonfire - Rank 5
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 8928: // Moonfire - Rank 6
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 8929: // Moonfire - Rank 7
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 8936: // Regrowth - Rank 1
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 8938: // Regrowth - Rank 2
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 8939: // Regrowth - Rank 3
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 8940: // Regrowth - Rank 4
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 8941: // Regrowth - Rank 5
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 8949: // Starfire - Rank 2
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 8950: // Starfire - Rank 3
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 8951: // Starfire - Rank 4
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 9472: // Flash Heal - Rank 2
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 9473: // Flash Heal - Rank 3
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 9474: // Flash Heal - Rank 4
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 9492: // Rip - Rank 2
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 9493: // Rip - Rank 3
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 9750: // Regrowth - Rank 6
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 9752: // Rip - Rank 4
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 9758: // Healing Touch - Rank 8
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 9839: // Rejuvenation - Rank 8
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 9840: // Rejuvenation - Rank 9
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 9841: // Rejuvenation - Rank 10
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 9852: // Entangling Roots - Rank 5
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 9853: // Entangling Roots - Rank 6
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 9856: // Regrowth - Rank 7
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 9857: // Regrowth - Rank 8
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 9858: // Regrowth - Rank 9
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 9862: // Tranquility - Rank 3
        {
            sp->SP_coef_override = float(0.538000f);
        }break;
    case 9863: // Tranquility - Rank 4
        {
            sp->SP_coef_override = float(0.538000f);
        }break;
    case 9875: // Starfire - Rank 5
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 9876: // Starfire - Rank 6
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 9888: // Healing Touch - Rank 9
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 9889: // Healing Touch - Rank 10
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 9894: // Rip - Rank 5
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 9896: // Rip - Rank 6
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 9912: // Wrath - Rank 8
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 10148: // Fireball - Rank 8
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 10149: // Fireball - Rank 9
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 10150: // Fireball - Rank 10
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 10151: // Fireball - Rank 11
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 10159: // Cone of Cold - Rank 3
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 10160: // Cone of Cold - Rank 4
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 10161: // Cone of Cold - Rank 5
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 10179: // Frostbolt - Rank 8
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 10180: // Frostbolt - Rank 9
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 10181: // Frostbolt - Rank 10
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 10191: // Mana Shield - Rank 4
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 10192: // Mana Shield - Rank 5
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 10193: // Mana Shield - Rank 6
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 10197: // Fire Blast - Rank 6
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 10199: // Fire Blast - Rank 7
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 10201: // Arcane Explosion - Rank 5
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 10202: // Arcane Explosion - Rank 6
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 10205: // Scorch - Rank 5
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 10206: // Scorch - Rank 6
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 10207: // Scorch - Rank 7
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 10211: // Arcane Missiles - Rank 6
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 10212: // Arcane Missiles - Rank 7
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 10215: // Flamestrike - Rank 5
        {
            sp->SP_coef_override = float(0.122000f);
        }break;
    case 10216: // Flamestrike - Rank 6
        {
            sp->SP_coef_override = float(0.122000f);
        }break;
    case 10230: // Frost Nova - Rank 4
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 10312: // Exorcism - Rank 4
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 10313: // Exorcism - Rank 5
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 10314: // Exorcism - Rank 6
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 10318: // Holy Wrath - Rank 2
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 10328: // Holy Light - Rank 7
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 10329: // Holy Light - Rank 8
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 10391: // Lightning Bolt - Rank 7
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 10392: // Lightning Bolt - Rank 8
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 10395: // Healing Wave - Rank 8
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 10396: // Healing Wave - Rank 9
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 10412: // Earth Shock - Rank 5
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 10413: // Earth Shock - Rank 6
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 10414: // Earth Shock - Rank 7
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 10431: // Lightning Shield - Rank 6
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 10432: // Lightning Shield - Rank 7
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 10435: // Attack - Rank 5
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 10436: // Attack - Rank 6
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 10445: // Flametongue Weapon Proc - Rank 4
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 10447: // Flame Shock - Rank 4
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 10448: // Flame Shock - Rank 5
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 10458: // Frostbrand Attack - Rank 3
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 10466: // Lesser Healing Wave - Rank 4
        {
            sp->SP_coef_override = float(0.808200f);
        }break;
    case 10467: // Lesser Healing Wave - Rank 5
        {
            sp->SP_coef_override = float(0.808200f);
        }break;
    case 10468: // Lesser Healing Wave - Rank 6
        {
            sp->SP_coef_override = float(0.808200f);
        }break;
    case 10472: // Frost Shock - Rank 3
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 10473: // Frost Shock - Rank 4
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 10579: // Magma Totem - Rank 2
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 10580: // Magma Totem - Rank 3
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 10581: // Magma Totem - Rank 4
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 10605: // Chain Lightning - Rank 4
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 10622: // Chain Heal - Rank 2
        {
            sp->SP_coef_override = float(0.800000f);
        }break;
    case 10623: // Chain Heal - Rank 3
        {
            sp->SP_coef_override = float(0.800000f);
        }break;
    case 10892: // Shadow Word: Pain - Rank 6
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 10893: // Shadow Word: Pain - Rank 7
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 10894: // Shadow Word: Pain - Rank 8
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 10898: // Power Word: Shield - Rank 7
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 10899: // Power Word: Shield - Rank 8
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 10900: // Power Word: Shield - Rank 9
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 10901: // Power Word: Shield - Rank 10
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 10915: // Flash Heal - Rank 5
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 10916: // Flash Heal - Rank 6
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 10917: // Flash Heal - Rank 7
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 10927: // Renew - Rank 7
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 10928: // Renew - Rank 8
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 10929: // Renew - Rank 9
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 10933: // Smite - Rank 7
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 10934: // Smite - Rank 8
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 10945: // Mind Blast - Rank 7
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 10946: // Mind Blast - Rank 8
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 10947: // Mind Blast - Rank 9
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 10960: // Prayer of Healing - Rank 3
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 10961: // Prayer of Healing - Rank 4
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 10963: // Greater Heal - Rank 2
        {
            sp->SP_coef_override = float(1.613500f);
        }break;
    case 10964: // Greater Heal - Rank 3
        {
            sp->SP_coef_override = float(1.613500f);
        }break;
    case 10965: // Greater Heal - Rank 4
        {
            sp->SP_coef_override = float(1.613500f);
        }break;
    case 11113: // Blast Wave - Rank 1
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 11285: // Gouge - Rank 4
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 11286: // Gouge - Rank 5
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 11289: // Garrote - Rank 5
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 11290: // Garrote - Rank 6
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 11335: // Instant Poison IV - Rank 4
        {
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 11336: // Instant Poison V - Rank 5
        {
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 11337: // Instant Poison VI - Rank 6
        {
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 11353: // Deadly Poison III - Rank 3
        {
            sp->AP_coef_override = float(0.024000f);
        }break;
    case 11354: // Deadly Poison IV - Rank 4
        {
            sp->AP_coef_override = float(0.024000f);
        }break;
    case 11366: // Pyroblast - Rank 1
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 11426: // Ice Barrier - Rank 1
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 11580: // Thunder Clap - Rank 5
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 11581: // Thunder Clap - Rank 6
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 11600: // Revenge - Rank 4
        {
            sp->AP_coef_override = float(0.207000f);
        }break;
    case 11601: // Revenge - Rank 5
        {
            sp->AP_coef_override = float(0.207000f);
        }break;
    case 11659: // Shadow Bolt - Rank 7
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 11660: // Shadow Bolt - Rank 8
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 11661: // Shadow Bolt - Rank 9
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 11665: // Immolate - Rank 5
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 11667: // Immolate - Rank 6
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 11668: // Immolate - Rank 7
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 11671: // Corruption - Rank 5
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 11672: // Corruption - Rank 6
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 11675: // Drain Soul - Rank 4
        {
            sp->SP_coef_override = float(0.429000f);
        }break;
    case 11677: // Rain of Fire - Rank 3
        {
            sp->SP_coef_override = float(0.693200f);
        }break;
    case 11678: // Rain of Fire - Rank 4
        {
            sp->SP_coef_override = float(0.693200f);
        }break;
    case 11683: // Hellfire - Rank 2
        {
            sp->SP_coef_override = float(0.120000f);
        }break;
    case 11684: // Hellfire - Rank 3
        {
            sp->SP_coef_override = float(0.120000f);
        }break;
    case 11693: // Health Funnel - Rank 5
        {
            sp->SP_coef_override = float(0.448500f);
        }break;
    case 11694: // Health Funnel - Rank 6
        {
            sp->SP_coef_override = float(0.448500f);
        }break;
    case 11695: // Health Funnel - Rank 7
        {
            sp->SP_coef_override = float(0.448500f);
        }break;
    case 11699: // Drain Life - Rank 5
        {
            sp->SP_coef_override = float(0.143000f);
        }break;
    case 11700: // Drain Life - Rank 6
        {
            sp->SP_coef_override = float(0.143000f);
        }break;
    case 11711: // Curse of Agony - Rank 4
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 11712: // Curse of Agony - Rank 5
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 11713: // Curse of Agony - Rank 6
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 11739: // Shadow Ward - Rank 2
        {
            sp->SP_coef_override = float(0.300000f);
        }break;
    case 11740: // Shadow Ward - Rank 3
        {
            sp->SP_coef_override = float(0.300000f);
        }break;
    case 12505: // Pyroblast - Rank 2
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 12522: // Pyroblast - Rank 3
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 12523: // Pyroblast - Rank 4
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 12524: // Pyroblast - Rank 5
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 12525: // Pyroblast - Rank 6
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 12526: // Pyroblast - Rank 7
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 12809: // Concussion Blow
        {
            sp->AP_coef_override = float(0.750000f);
        }break;
    case 13018: // Blast Wave - Rank 2
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 13019: // Blast Wave - Rank 3
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 13020: // Blast Wave - Rank 4
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 13021: // Blast Wave - Rank 5
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 13031: // Ice Barrier - Rank 2
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 13032: // Ice Barrier - Rank 3
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 13033: // Ice Barrier - Rank 4
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 13218: // Wound Poison - Rank 1
        {
            sp->AP_coef_override = float(0.008000f);
        }break;
    case 13222: // Wound Poison II - Rank 2
        {
            sp->AP_coef_override = float(0.008000f);
        }break;
    case 13223: // Wound Poison III - Rank 3
        {
            sp->AP_coef_override = float(0.008000f);
        }break;
    case 13224: // Wound Poison IV - Rank 4
        {
            sp->AP_coef_override = float(0.008000f);
        }break;
    case 13549: // Serpent Sting - Rank 2
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 13550: // Serpent Sting - Rank 3
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 13551: // Serpent Sting - Rank 4
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 13552: // Serpent Sting - Rank 5
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 13553: // Serpent Sting - Rank 6
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 13554: // Serpent Sting - Rank 7
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 13555: // Serpent Sting - Rank 8
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 13797: // Immolation Trap - Rank 1
        {
            sp->RAP_coef_override = float(0.020000f);
        }break;
    case 14269: // Mongoose Bite - Rank 2
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 14270: // Mongoose Bite - Rank 3
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 14271: // Mongoose Bite - Rank 4
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 14281: // Arcane Shot - Rank 2
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 14282: // Arcane Shot - Rank 3
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 14283: // Arcane Shot - Rank 4
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 14284: // Arcane Shot - Rank 5
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 14285: // Arcane Shot - Rank 6
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 14286: // Arcane Shot - Rank 7
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 14287: // Arcane Shot - Rank 8
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 14288: // Multi-Shot - Rank 2
        {
            sp->RAP_coef_override = float(0.200000f);
        }break;
    case 14289: // Multi-Shot - Rank 3
        {
            sp->RAP_coef_override = float(0.200000f);
        }break;
    case 14290: // Multi-Shot - Rank 4
        {
            sp->RAP_coef_override = float(0.200000f);
        }break;
    case 14298: // Immolation Trap - Rank 2
        {
            sp->RAP_coef_override = float(0.020000f);
        }break;
    case 14299: // Immolation Trap - Rank 3
        {
            sp->RAP_coef_override = float(0.020000f);
        }break;
    case 14300: // Immolation Trap - Rank 4
        {
            sp->RAP_coef_override = float(0.020000f);
        }break;
    case 14301: // Immolation Trap - Rank 5
        {
            sp->RAP_coef_override = float(0.020000f);
        }break;
    case 14914: // Holy Fire - Rank 1
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 15207: // Lightning Bolt - Rank 9
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 15208: // Lightning Bolt - Rank 10
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 15237: // Holy Nova - Rank 1
        {
            sp->SP_coef_override = float(0.160600f);
        }break;
    case 15261: // Holy Fire - Rank 8
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 15262: // Holy Fire - Rank 2
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 15263: // Holy Fire - Rank 3
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 15264: // Holy Fire - Rank 4
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 15265: // Holy Fire - Rank 5
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 15266: // Holy Fire - Rank 6
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 15267: // Holy Fire - Rank 7
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 15407: // Mind Flay - Rank 1
        {
            sp->SP_coef_override = float(0.257000f);
        }break;
    case 15430: // Holy Nova - Rank 2
        {
            sp->SP_coef_override = float(0.160600f);
        }break;
    case 15431: // Holy Nova - Rank 3
        {
            sp->SP_coef_override = float(0.160600f);
        }break;
    case 16343: // Flametongue Weapon Proc - Rank 5
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 16344: // Flametongue Weapon Proc - Rank 6
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 16352: // Frostbrand Attack - Rank 4
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 16353: // Frostbrand Attack - Rank 5
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 16827: // Claw - Rank 1
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 16828: // Claw - Rank 2
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 16829: // Claw - Rank 3
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 16830: // Claw - Rank 4
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 16831: // Claw - Rank 5
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 16832: // Claw - Rank 6
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 16857: // Faerie Fire (Feral)
        {
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 17253: // Bite - Rank 1
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 17255: // Bite - Rank 2
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 17256: // Bite - Rank 3
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 17257: // Bite - Rank 4
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 17258: // Bite - Rank 5
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 17259: // Bite - Rank 6
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 17260: // Bite - Rank 7
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 17261: // Bite - Rank 8
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 17311: // Mind Flay - Rank 2
        {
            sp->SP_coef_override = float(0.257000f);
        }break;
    case 17312: // Mind Flay - Rank 3
        {
            sp->SP_coef_override = float(0.257000f);
        }break;
    case 17313: // Mind Flay - Rank 4
        {
            sp->SP_coef_override = float(0.257000f);
        }break;
    case 17314: // Mind Flay - Rank 5
        {
            sp->SP_coef_override = float(0.257000f);
        }break;
    case 17877: // Shadowburn - Rank 1
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 17919: // Searing Pain - Rank 2
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 17920: // Searing Pain - Rank 3
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 17921: // Searing Pain - Rank 4
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 17922: // Searing Pain - Rank 5
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 17923: // Searing Pain - Rank 6
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 17924: // Soul Fire - Rank 2
        {
            sp->SP_coef_override = float(1.150000f);
        }break;
    case 17925: // Death Coil - Rank 2
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 17926: // Death Coil - Rank 3
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 17962: // Conflagrate
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 18220: // Dark Pact - Rank 1
        {
            sp->SP_coef_override = float(0.960000f);
        }break;
    case 18807: // Mind Flay - Rank 6
        {
            sp->SP_coef_override = float(0.257000f);
        }break;
    case 18809: // Pyroblast - Rank 8
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 18867: // Shadowburn - Rank 2
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 18868: // Shadowburn - Rank 3
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 18869: // Shadowburn - Rank 4
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 18870: // Shadowburn - Rank 5
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 18871: // Shadowburn - Rank 6
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 18937: // Dark Pact - Rank 2
        {
            sp->SP_coef_override = float(0.960000f);
        }break;
    case 18938: // Dark Pact - Rank 3
        {
            sp->SP_coef_override = float(0.960000f);
        }break;
    case 19236: // Desperate Prayer - Rank 1
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 19238: // Desperate Prayer - Rank 2
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 19240: // Desperate Prayer - Rank 3
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 19241: // Desperate Prayer - Rank 4
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 19242: // Desperate Prayer - Rank 5
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 19243: // Desperate Prayer - Rank 6
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 19276: // Devouring Plague - Rank 2
        {
            sp->SP_coef_override = float(0.184900f);
        }break;
    case 19277: // Devouring Plague - Rank 3
        {
            sp->SP_coef_override = float(0.184900f);
        }break;
    case 19278: // Devouring Plague - Rank 4
        {
            sp->SP_coef_override = float(0.184900f);
        }break;
    case 19279: // Devouring Plague - Rank 5
        {
            sp->SP_coef_override = float(0.184900f);
        }break;
    case 19280: // Devouring Plague - Rank 6
        {
            sp->SP_coef_override = float(0.184900f);
        }break;
    case 19750: // Flash of Light - Rank 1
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 19939: // Flash of Light - Rank 2
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 19940: // Flash of Light - Rank 3
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 19941: // Flash of Light - Rank 4
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 19942: // Flash of Light - Rank 5
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 19943: // Flash of Light - Rank 6
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 20116: // Consecration - Rank 2
        {
            sp->SP_coef_override = float(0.040000f);
        }break;
    case 20167: // Seal of Light
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 20168: // Seal of Wisdom
        {
            sp->SP_coef_override = float(0.250000f);
        }break;
    case 20252: // Intercept
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 20267: // Judgement of Light - Rank 1
        {
            sp->SP_coef_override = float(0.100000f);
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 20424: // Seal of Command
        {
            sp->SP_coef_override = float(0.230000f);
        }break;
    case 20467: // Judgement of Command - Rank 1
        {
            sp->SP_coef_override = float(0.250000f);
            sp->AP_coef_override = float(0.160000f);
        }break;
    case 20658: // Execute - Rank 2
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 20660: // Execute - Rank 3
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 20661: // Execute - Rank 4
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 20662: // Execute - Rank 5
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 20922: // Consecration - Rank 3
        {
            sp->SP_coef_override = float(0.040000f);
        }break;
    case 20923: // Consecration - Rank 4
        {
            sp->SP_coef_override = float(0.040000f);
        }break;
    case 20924: // Consecration - Rank 5
        {
            sp->SP_coef_override = float(0.040000f);
        }break;
    case 20925: // Holy Shield - Rank 1
        {
            sp->SP_coef_override = float(0.090000f);
            sp->AP_coef_override = float(0.056000f);
        }break;
    case 20927: // Holy Shield - Rank 2
        {
            sp->SP_coef_override = float(0.090000f);
            sp->AP_coef_override = float(0.056000f);
        }break;
    case 20928: // Holy Shield - Rank 3
        {
            sp->SP_coef_override = float(0.090000f);
            sp->AP_coef_override = float(0.056000f);
        }break;
    case 25742: // Seal of Righteousness
        {
            sp->SP_coef_override = float(0.044000f);
            sp->AP_coef_override = float(0.022000f);
        }break;
    case 23455: // Holy Nova - Rank 1
        {
            sp->SP_coef_override = float(0.303500f);
        }break;
    case 23458: // Holy Nova - Rank 2
        {
            sp->SP_coef_override = float(0.303500f);
        }break;
    case 23459: // Holy Nova - Rank 3
        {
            sp->SP_coef_override = float(0.303500f);
        }break;
    case 24239: // Hammer of Wrath - Rank 3
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 24274: // Hammer of Wrath - Rank 2
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 24275: // Hammer of Wrath - Rank 1
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 24583: // Scorpid Poison - Rank 2
        {
            sp->RAP_coef_override = float(0.015000f);
        }break;
    case 24586: // Scorpid Poison - Rank 3
        {
            sp->RAP_coef_override = float(0.015000f);
        }break;
    case 24587: // Scorpid Poison - Rank 4
        {
            sp->RAP_coef_override = float(0.015000f);
        }break;
    case 24640: // Scorpid Poison - Rank 1
        {
            sp->RAP_coef_override = float(0.015000f);
        }break;
    case 24844: // Lightning Breath - Rank 1
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 24974: // Insect Swarm - Rank 2
        {
            sp->SP_coef_override = float(0.127000f);
        }break;
    case 24975: // Insect Swarm - Rank 3
        {
            sp->SP_coef_override = float(0.127000f);
        }break;
    case 24976: // Insect Swarm - Rank 4
        {
            sp->SP_coef_override = float(0.127000f);
        }break;
    case 24977: // Insect Swarm - Rank 5
        {
            sp->SP_coef_override = float(0.127000f);
        }break;
    case 25008: // Lightning Breath - Rank 2
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 25009: // Lightning Breath - Rank 3
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 25010: // Lightning Breath - Rank 4
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 25011: // Lightning Breath - Rank 5
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 25012: // Lightning Breath - Rank 6
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 25210: // Greater Heal - Rank 6
        {
            sp->SP_coef_override = float(1.613500f);
        }break;
    case 25213: // Greater Heal - Rank 7
        {
            sp->SP_coef_override = float(1.613500f);
        }break;
    case 25217: // Power Word: Shield - Rank 11
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 25218: // Power Word: Shield - Rank 12
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 25221: // Renew - Rank 11
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 25222: // Renew - Rank 12
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 25233: // Flash Heal - Rank 8
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 25234: // Execute - Rank 6
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 25235: // Flash Heal - Rank 9
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 25236: // Execute - Rank 7
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 25264: // Thunder Clap - Rank 7
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 25269: // Revenge - Rank 7
        {
            sp->AP_coef_override = float(0.207000f);
        }break;
    case 25288: // Revenge - Rank 6
        {
            sp->AP_coef_override = float(0.207000f);
        }break;
    case 25292: // Holy Light - Rank 9
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 25294: // Multi-Shot - Rank 5
        {
            sp->RAP_coef_override = float(0.200000f);
        }break;
    case 25295: // Serpent Sting - Rank 9
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 25297: // Healing Touch - Rank 11
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 25298: // Starfire - Rank 7
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 25299: // Rejuvenation - Rank 11
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 25304: // Frostbolt - Rank 11
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 25306: // Fireball - Rank 12
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 25307: // Shadow Bolt - Rank 10
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 25308: // Prayer of Healing - Rank 6
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 25309: // Immolate - Rank 8
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 25311: // Corruption - Rank 7
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 25314: // Greater Heal - Rank 5
        {
            sp->SP_coef_override = float(1.613500f);
        }break;
    case 25315: // Renew - Rank 10
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 25316: // Prayer of Healing - Rank 5
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 25329: // Holy Nova - Rank 7
        {
            sp->SP_coef_override = float(0.303500f);
        }break;
    case 25331: // Holy Nova - Rank 7
        {
            sp->SP_coef_override = float(0.160600f);
        }break;
    case 25345: // Arcane Missiles - Rank 8
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 25349: // Deadly Poison V - Rank 5
        {
            sp->AP_coef_override = float(0.024000f);
        }break;
    case 25357: // Healing Wave - Rank 10
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 25363: // Smite - Rank 9
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 25364: // Smite - Rank 10
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 25367: // Shadow Word: Pain - Rank 9
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 25368: // Shadow Word: Pain - Rank 10
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 25372: // Mind Blast - Rank 10
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 25375: // Mind Blast - Rank 11
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 25384: // Holy Fire - Rank 9
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 25387: // Mind Flay - Rank 7
        {
            sp->SP_coef_override = float(0.257000f);
        }break;
    case 25391: // Healing Wave - Rank 11
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 25396: // Healing Wave - Rank 12
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 25420: // Lesser Healing Wave - Rank 7
        {
            sp->SP_coef_override = float(0.808200f);
        }break;
    case 25422: // Chain Heal - Rank 4
        {
            sp->SP_coef_override = float(0.800000f);
        }break;
    case 25423: // Chain Heal - Rank 5
        {
            sp->SP_coef_override = float(0.800000f);
        }break;
    case 25437: // Desperate Prayer - Rank 7
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 25439: // Chain Lightning - Rank 5
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 25442: // Chain Lightning - Rank 6
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 25448: // Lightning Bolt - Rank 11
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 25449: // Lightning Bolt - Rank 12
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 25454: // Earth Shock - Rank 8
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 25457: // Flame Shock - Rank 7
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 25464: // Frost Shock - Rank 5
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 25467: // Devouring Plague - Rank 7
        {
            sp->SP_coef_override = float(0.184900f);
        }break;
    case 25469: // Lightning Shield - Rank 8
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 25472: // Lightning Shield - Rank 9
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 25488: // Flametongue Weapon Proc - Rank 7
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 25501: // Frostbrand Attack - Rank 6
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 25530: // Attack - Rank 7
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 25550: // Magma Totem - Rank 5
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 25902: // Holy Shock - Rank 3
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 25903: // Holy Shock - Rank 3
        {
            sp->SP_coef_override = float(0.810000f);
        }break;
    case 25911: // Holy Shock - Rank 2
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 25912: // Holy Shock - Rank 1
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 25913: // Holy Shock - Rank 2
        {
            sp->SP_coef_override = float(0.810000f);
        }break;
    case 25914: // Holy Shock - Rank 1
        {
            sp->SP_coef_override = float(0.810000f);
        }break;
    case 26573: // Consecration - Rank 1
        {
            sp->SP_coef_override = float(0.040000f);
        }break;
    case 26839: // Garrote - Rank 7
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 26884: // Garrote - Rank 8
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 26890: // Instant Poison VII - Rank 7
        {
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 26968: // Deadly Poison VI - Rank 6
        {
            sp->AP_coef_override = float(0.024000f);
        }break;
    case 26978: // Healing Touch - Rank 12
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 26979: // Healing Touch - Rank 13
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 26980: // Regrowth - Rank 10
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 26981: // Rejuvenation - Rank 12
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 26982: // Rejuvenation - Rank 13
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 26983: // Tranquility - Rank 5
        {
            sp->SP_coef_override = float(0.538000f);
        }break;
    case 26984: // Wrath - Rank 9
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 26985: // Wrath - Rank 10
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 26986: // Starfire - Rank 8
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 26987: // Moonfire - Rank 11
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 26988: // Moonfire - Rank 12
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 26989: // Entangling Roots - Rank 7
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 27008: // Rip - Rank 7
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 27013: // Insect Swarm - Rank 6
        {
            sp->SP_coef_override = float(0.127000f);
        }break;
    case 27016: // Serpent Sting - Rank 10
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 27019: // Arcane Shot - Rank 9
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 27021: // Multi-Shot - Rank 6
        {
            sp->RAP_coef_override = float(0.200000f);
        }break;
    case 27024: // Immolation Trap - Rank 6
        {
            sp->RAP_coef_override = float(0.020000f);
        }break;
    case 27049: // Claw - Rank 9
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 27050: // Bite - Rank 9
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 27060: // Scorpid Poison - Rank 5
        {
            sp->RAP_coef_override = float(0.015000f);
        }break;
    case 27070: // Fireball - Rank 13
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 27071: // Frostbolt - Rank 12
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 27072: // Frostbolt - Rank 13
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 27073: // Scorch - Rank 8
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 27074: // Scorch - Rank 9
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 27075: // Arcane Missiles - Rank 9
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 27078: // Fire Blast - Rank 8
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 27079: // Fire Blast - Rank 9
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 27080: // Arcane Explosion - Rank 7
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 27082: // Arcane Explosion - Rank 8
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 27086: // Flamestrike - Rank 7
        {
            sp->SP_coef_override = float(0.122000f);
        }break;
    case 27087: // Cone of Cold - Rank 6
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 27088: // Frost Nova - Rank 5
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 27131: // Mana Shield - Rank 7
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 27132: // Pyroblast - Rank 9
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 27133: // Blast Wave - Rank 6
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 27134: // Ice Barrier - Rank 5
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 27135: // Holy Light - Rank 10
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 27136: // Holy Light - Rank 11
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 27137: // Flash of Light - Rank 7
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 27138: // Exorcism - Rank 7
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 27139: // Holy Wrath - Rank 3
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 27173: // Consecration - Rank 6
        {
            sp->SP_coef_override = float(0.040000f);
        }break;
    case 27175: // Holy Shock - Rank 4
        {
            sp->SP_coef_override = float(0.810000f);
        }break;
    case 27176: // Holy Shock - Rank 4
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 27179: // Holy Shield - Rank 4
        {
            sp->SP_coef_override = float(0.090000f);
            sp->AP_coef_override = float(0.056000f);
        }break;
    case 27180: // Hammer of Wrath - Rank 4
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 27187: // Deadly Poison VII - Rank 7
        {
            sp->AP_coef_override = float(0.024000f);
        }break;
    case 27189: // Wound Poison V - Rank 5
        {
            sp->AP_coef_override = float(0.008000f);
        }break;
    case 27209: // Shadow Bolt - Rank 11
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 27210: // Searing Pain - Rank 7
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 27211: // Soul Fire - Rank 3
        {
            sp->SP_coef_override = float(1.150000f);
        }break;
    case 27212: // Rain of Fire - Rank 5
        {
            sp->SP_coef_override = float(0.693200f);
        }break;
    case 27213: // Hellfire - Rank 4
        {
            sp->SP_coef_override = float(0.120000f);
        }break;
    case 27215: // Immolate - Rank 9
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 27216: // Corruption - Rank 8
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 27217: // Drain Soul - Rank 5
        {
            sp->SP_coef_override = float(0.429000f);
        }break;
    case 27218: // Curse of Agony - Rank 7
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 27219: // Drain Life - Rank 7
        {
            sp->SP_coef_override = float(0.143000f);
        }break;
    case 27220: // Drain Life - Rank 8
        {
            sp->SP_coef_override = float(0.143000f);
        }break;
    case 27223: // Death Coil - Rank 4
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 27243: // Seed of Corruption - Rank 1
        {
            sp->SP_coef_override = float(0.250000f);
        }break;
    case 27259: // Health Funnel - Rank 8
        {
            sp->SP_coef_override = float(0.448500f);
        }break;
    case 27263: // Shadowburn - Rank 7
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 27265: // Dark Pact - Rank 4
        {
            sp->SP_coef_override = float(0.960000f);
        }break;
    case 27799: // Holy Nova - Rank 4
        {
            sp->SP_coef_override = float(0.160600f);
        }break;
    case 27800: // Holy Nova - Rank 5
        {
            sp->SP_coef_override = float(0.160600f);
        }break;
    case 27801: // Holy Nova - Rank 6
        {
            sp->SP_coef_override = float(0.160600f);
        }break;
    case 27803: // Holy Nova - Rank 4
        {
            sp->SP_coef_override = float(0.303500f);
        }break;
    case 27804: // Holy Nova - Rank 5
        {
            sp->SP_coef_override = float(0.303500f);
        }break;
    case 27805: // Holy Nova - Rank 6
        {
            sp->SP_coef_override = float(0.303500f);
        }break;
    case 28610: // Shadow Ward - Rank 4
        {
            sp->SP_coef_override = float(0.300000f);
        }break;
    case 29228: // Flame Shock - Rank 6
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 29722: // Incinerate - Rank 1
        {
            sp->SP_coef_override = float(0.713900f);
        }break;
    case 30108: // Unstable Affliction - Rank 1
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 30283: // Shadowfury - Rank 1
        {
            sp->SP_coef_override = float(0.193200f);
        }break;
    case 30357: // Revenge - Rank 8
        {
            sp->AP_coef_override = float(0.207000f);
        }break;
    case 30404: // Unstable Affliction - Rank 2
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 30405: // Unstable Affliction - Rank 3
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 30413: // Shadowfury - Rank 2
        {
            sp->SP_coef_override = float(0.193200f);
        }break;
    case 30414: // Shadowfury - Rank 3
        {
            sp->SP_coef_override = float(0.193200f);
        }break;
    case 30451: // Arcane Blast - Rank 1
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 30455: // Ice Lance - Rank 1
        {
            sp->SP_coef_override = float(0.142900f);
        }break;
    case 30459: // Searing Pain - Rank 8
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 30546: // Shadowburn - Rank 8
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 30824: // Shamanistic Rage
        {
            sp->AP_coef_override = float(0.300000f);
        }break;
    case 30910: // Curse of Doom - Rank 2
        {
            sp->SP_coef_override = float(2.000000f);
        }break;
    case 31117: // Unstable Affliction
        {
            sp->SP_coef_override = float(1.800000f);
        }break;
    case 31661: // Dragon's Breath - Rank 1
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 53742: // Blood Corruption
    case 31803: // Holy Vengeance
        {
            sp->SP_coef_override = float(0.013000f);
            sp->AP_coef_override = float(0.025000f);
        }break;
    case 31893: // Seal of Blood
        {
            sp->SP_coef_override = float(0.020000f);
            sp->AP_coef_override = float(0.030000f);
        }break;
    case 31898: // Judgement of Blood - Rank 1
        {
            sp->SP_coef_override = float(0.250000f);
            sp->AP_coef_override = float(0.160000f);
        }break;
    case 31935: // Avenger's Shield - Rank 1
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 32231: // Incinerate - Rank 2
        {
            sp->SP_coef_override = float(0.713900f);
        }break;
    case 32379: // Shadow Word: Death - Rank 1
        {
            sp->SP_coef_override = float(0.429600f);
        }break;
    case 32546: // Binding Heal - Rank 1
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 32593: // Earth Shield - Rank 2
        {
            sp->SP_coef_override = float(0.476100f);
        }break;
    case 32594: // Earth Shield - Rank 3
        {
            sp->SP_coef_override = float(0.476100f);
        }break;
    case 32699: // Avenger's Shield - Rank 2
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 32700: // Avenger's Shield - Rank 3
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 32996: // Shadow Word: Death - Rank 2
        {
            sp->SP_coef_override = float(0.429600f);
        }break;
    case 33041: // Dragon's Breath - Rank 2
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 33042: // Dragon's Breath - Rank 3
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 33043: // Dragon's Breath - Rank 4
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 33073: // Holy Shock - Rank 5
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 33074: // Holy Shock - Rank 5
        {
            sp->SP_coef_override = float(0.810000f);
        }break;
    case 33405: // Ice Barrier - Rank 6
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 33745: // Lacerate - Rank 1
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 33763: // Lifebloom - Rank 1
        {
            sp->SP_coef_override = float(0.095180f);
        }break;
    case 33933: // Blast Wave - Rank 7
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 33938: // Pyroblast - Rank 10
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 34120: // Steady Shot - Rank 2
        {
            sp->RAP_coef_override = float(0.100000f);
        }break;
    case 34428: // Victory Rush
        {
            sp->AP_coef_override = float(0.450000f);
        }break;
    case 34861: // Circle of Healing - Rank 1
        {
            sp->SP_coef_override = float(0.402000f);
        }break;
    case 34863: // Circle of Healing - Rank 2
        {
            sp->SP_coef_override = float(0.402000f);
        }break;
    case 34864: // Circle of Healing - Rank 3
        {
            sp->SP_coef_override = float(0.402000f);
        }break;
    case 34865: // Circle of Healing - Rank 4
        {
            sp->SP_coef_override = float(0.402000f);
        }break;
    case 34866: // Circle of Healing - Rank 5
        {
            sp->SP_coef_override = float(0.402000f);
        }break;
    case 34889: // Fire Breath - Rank 1
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 34914: // Vampiric Touch - Rank 1
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 34916: // Vampiric Touch - Rank 2
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 34917: // Vampiric Touch - Rank 3
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 35290: // Gore - Rank 1
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 35291: // Gore - Rank 2
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 35292: // Gore - Rank 3
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 35293: // Gore - Rank 4
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 35294: // Gore - Rank 5
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 35295: // Gore - Rank 6
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 35323: // Fire Breath - Rank 2
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 35387: // Poison Spit - Rank 1
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 35389: // Poison Spit - Rank 2
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 35392: // Poison Spit - Rank 3
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 36916: // Mongoose Bite - Rank 5
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 38692: // Fireball - Rank 14
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 38697: // Frostbolt - Rank 14
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 38699: // Arcane Missiles - Rank 10
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 38704: // Arcane Missiles - Rank 11
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 38764: // Gouge - Rank 6
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 41637: // Prayer of Mending
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 42198: // Blizzard - Rank 7
        {
            sp->SP_coef_override = float(0.143700f);
        }break;
    case 42208: // Blizzard - Rank 1
        {
            sp->SP_coef_override = float(0.143700f);
        }break;
    case 42209: // Blizzard - Rank 2
        {
            sp->SP_coef_override = float(0.143700f);
        }break;
    case 42210: // Blizzard - Rank 3
        {
            sp->SP_coef_override = float(0.143700f);
        }break;
    case 42211: // Blizzard - Rank 4
        {
            sp->SP_coef_override = float(0.143700f);
        }break;
    case 42212: // Blizzard - Rank 5
        {
            sp->SP_coef_override = float(0.143700f);
        }break;
    case 42213: // Blizzard - Rank 6
        {
            sp->SP_coef_override = float(0.143700f);
        }break;
    case 42230: // Hurricane - Rank 4
        {
            sp->SP_coef_override = float(0.128980f);
        }break;
    case 42231: // Hurricane - Rank 1
        {
            sp->SP_coef_override = float(0.128980f);
        }break;
    case 42232: // Hurricane - Rank 2
        {
            sp->SP_coef_override = float(0.128980f);
        }break;
    case 42233: // Hurricane - Rank 3
        {
            sp->SP_coef_override = float(0.128980f);
        }break;
    case 42832: // Fireball - Rank 15
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 42833: // Fireball - Rank 16
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 42841: // Frostbolt - Rank 15
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 42842: // Frostbolt - Rank 16
        {
            sp->SP_coef_override = float(0.814300f);
        }break;
    case 42843: // Arcane Missiles - Rank 12
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 42846: // Arcane Missiles - Rank 13
        {
            sp->SP_coef_override = float(0.285700f);
        }break;
    case 42858: // Scorch - Rank 10
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 42859: // Scorch - Rank 11
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 42872: // Fire Blast - Rank 10
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 42873: // Fire Blast - Rank 11
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 42890: // Pyroblast - Rank 11
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 42891: // Pyroblast - Rank 12
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 42894: // Arcane Blast - Rank 2
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 42896: // Arcane Blast - Rank 3
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 42897: // Arcane Blast - Rank 4
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 42913: // Ice Lance - Rank 2
        {
            sp->SP_coef_override = float(0.142900f);
        }break;
    case 42914: // Ice Lance - Rank 3
        {
            sp->SP_coef_override = float(0.142900f);
        }break;
    case 42917: // Frost Nova - Rank 6
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 42920: // Arcane Explosion - Rank 9
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 42921: // Arcane Explosion - Rank 10
        {
            sp->SP_coef_override = float(0.212800f);
        }break;
    case 42925: // Flamestrike - Rank 8
        {
            sp->SP_coef_override = float(0.122000f);
        }break;
    case 42926: // Flamestrike - Rank 9
        {
            sp->SP_coef_override = float(0.122000f);
        }break;
    case 42930: // Cone of Cold - Rank 7
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 42931: // Cone of Cold - Rank 8
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 42937: // Blizzard - Rank 8
        {
            sp->SP_coef_override = float(0.143700f);
        }break;
    case 42938: // Blizzard - Rank 9
        {
            sp->SP_coef_override = float(0.143700f);
        }break;
    case 42944: // Blast Wave - Rank 8
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 42945: // Blast Wave - Rank 9
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 42949: // Dragon's Breath - Rank 5
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 42950: // Dragon's Breath - Rank 6
        {
            sp->SP_coef_override = float(0.193600f);
        }break;
    case 43019: // Mana Shield - Rank 8
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 43020: // Mana Shield - Rank 9
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 43038: // Ice Barrier - Rank 7
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 43039: // Ice Barrier - Rank 8
        {
            sp->SP_coef_override = float(0.805300f);
        }break;
    case 44425: // Arcane Barrage - Rank 1
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 44457: // Living Bomb - Rank 1
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 44614: // Frostfire Bolt - Rank 1
        {
            sp->SP_coef_override = float(0.857100f);
        }break;
    case 44780: // Arcane Barrage - Rank 2
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 44781: // Arcane Barrage - Rank 3
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 46968: // Shockwave
        {
            sp->AP_coef_override = float(0.750000f);
        }break;
    case 47470: // Execute - Rank 8
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 47471: // Execute - Rank 9
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 47501: // Thunder Clap - Rank 8
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 47502: // Thunder Clap - Rank 9
        {
            sp->AP_coef_override = float(0.120000f);
        }break;
    case 47610: // Frostfire Bolt - Rank 2
        {
            sp->SP_coef_override = float(0.857100f);
        }break;
    case 47666: // Penance - Rank 1
        {
            sp->SP_coef_override = float(0.229000f);
        }break;
    case 47750: // Penance - Rank 1
        {
            sp->SP_coef_override = float(0.535000f);
        }break;
    case 47808: // Shadow Bolt - Rank 12
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 47809: // Shadow Bolt - Rank 13
        {
            sp->SP_coef_override = float(0.856900f);
        }break;
    case 47810: // Immolate - Rank 10
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 47811: // Immolate - Rank 11
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 47812: // Corruption - Rank 9
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 47813: // Corruption - Rank 10
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 47814: // Searing Pain - Rank 9
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 47815: // Searing Pain - Rank 10
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 47819: // Rain of Fire - Rank 6
        {
            sp->SP_coef_override = float(0.693200f);
        }break;
    case 47820: // Rain of Fire - Rank 7
        {
            sp->SP_coef_override = float(0.693200f);
        }break;
    case 47823: // Hellfire - Rank 5
        {
            sp->SP_coef_override = float(0.120000f);
        }break;
    case 47824: // Soul Fire - Rank 5
        {
            sp->SP_coef_override = float(1.150000f);
        }break;
    case 47825: // Soul Fire - Rank 6
        {
            sp->SP_coef_override = float(1.150000f);
        }break;
    case 47826: // Shadowburn - Rank 9
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 47827: // Shadowburn - Rank 10
        {
            sp->SP_coef_override = float(0.429300f);
        }break;
    case 47835: // Seed of Corruption - Rank 2
        {
            sp->SP_coef_override = float(0.250000f);
        }break;
    case 47836: // Seed of Corruption - Rank 3
        {
            sp->SP_coef_override = float(0.250000f);
        }break;
    case 47837: // Incinerate - Rank 3
        {
            sp->SP_coef_override = float(0.713900f);
        }break;
    case 47838: // Incinerate - Rank 4
        {
            sp->SP_coef_override = float(0.713900f);
        }break;
    case 47841: // Unstable Affliction - Rank 4
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 47843: // Unstable Affliction - Rank 5
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 47846: // Shadowfury - Rank 4
        {
            sp->SP_coef_override = float(0.193200f);
        }break;
    case 47847: // Shadowfury - Rank 5
        {
            sp->SP_coef_override = float(0.193200f);
        }break;
    case 47855: // Drain Soul - Rank 6
        {
            sp->SP_coef_override = float(0.429000f);
        }break;
    case 47856: // Health Funnel - Rank 9
        {
            sp->SP_coef_override = float(0.448500f);
        }break;
    case 47857: // Drain Life - Rank 9
        {
            sp->SP_coef_override = float(0.143000f);
        }break;
    case 47859: // Death Coil - Rank 5
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 47860: // Death Coil - Rank 6
        {
            sp->SP_coef_override = float(0.214000f);
        }break;
    case 47863: // Curse of Agony - Rank 8
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 47864: // Curse of Agony - Rank 9
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 47867: // Curse of Doom - Rank 3
        {
            sp->SP_coef_override = float(2.000000f);
        }break;
    case 47890: // Shadow Ward - Rank 5
        {
            sp->SP_coef_override = float(0.300000f);
        }break;
    case 47891: // Shadow Ward - Rank 6
        {
            sp->SP_coef_override = float(0.300000f);
        }break;
    case 48062: // Greater Heal - Rank 8
        {
            sp->SP_coef_override = float(1.613500f);
        }break;
    case 48063: // Greater Heal - Rank 9
        {
            sp->SP_coef_override = float(1.613500f);
        }break;
    case 48065: // Power Word: Shield - Rank 13
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 48066: // Power Word: Shield - Rank 14
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 48067: // Renew - Rank 13
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 48068: // Renew - Rank 14
        {
            sp->SP_coef_override = float(0.360000f);
        }break;
    case 48070: // Flash Heal - Rank 10
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 48071: // Flash Heal - Rank 11
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 48072: // Prayer of Healing - Rank 7
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 48075: // Holy Nova - Rank 8
        {
            sp->SP_coef_override = float(0.303500f);
        }break;
    case 48076: // Holy Nova - Rank 9
        {
            sp->SP_coef_override = float(0.303500f);
        }break;
    case 48077: // Holy Nova - Rank 8
        {
            sp->SP_coef_override = float(0.160600f);
        }break;
    case 48078: // Holy Nova - Rank 9
        {
            sp->SP_coef_override = float(0.160600f);
        }break;
    case 48088: // Circle of Healing - Rank 6
        {
            sp->SP_coef_override = float(0.402000f);
        }break;
    case 48089: // Circle of Healing - Rank 7
        {
            sp->SP_coef_override = float(0.402000f);
        }break;
    case 48119: // Binding Heal - Rank 2
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 48120: // Binding Heal - Rank 3
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 48122: // Smite - Rank 11
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 48123: // Smite - Rank 12
        {
            sp->SP_coef_override = float(0.714000f);
        }break;
    case 48124: // Shadow Word: Pain - Rank 11
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 48125: // Shadow Word: Pain - Rank 12
        {
            sp->SP_coef_override = float(0.182900f);
        }break;
    case 48126: // Mind Blast - Rank 12
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 48127: // Mind Blast - Rank 13
        {
            sp->SP_coef_override = float(0.428000f);
        }break;
    case 48134: // Holy Fire - Rank 10
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 48135: // Holy Fire - Rank 11
        {
            sp->SP_coef_override = float(0.024000f);
        }break;
    case 48155: // Mind Flay - Rank 8
        {
            sp->SP_coef_override = float(0.257000f);
        }break;
    case 48156: // Mind Flay - Rank 9
        {
            sp->SP_coef_override = float(0.257000f);
        }break;
    case 48157: // Shadow Word: Death - Rank 3
        {
            sp->SP_coef_override = float(0.429600f);
        }break;
    case 48158: // Shadow Word: Death - Rank 4
        {
            sp->SP_coef_override = float(0.429600f);
        }break;
    case 48159: // Vampiric Touch - Rank 4
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 48160: // Vampiric Touch - Rank 5
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 48172: // Desperate Prayer - Rank 8
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 48173: // Desperate Prayer - Rank 9
        {
            sp->SP_coef_override = float(0.806800f);
        }break;
    case 48181: // Haunt - Rank 1
        {
            sp->SP_coef_override = float(0.479300f);
        }break;
    case 48299: // Devouring Plague - Rank 8
        {
            sp->SP_coef_override = float(0.184900f);
        }break;
    case 48300: // Devouring Plague - Rank 9
        {
            sp->SP_coef_override = float(0.184900f);
        }break;
    case 48377: // Healing Touch - Rank 14
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 48378: // Healing Touch - Rank 15
        {
            sp->SP_coef_override = float(1.610400f);
        }break;
    case 48438: // Wild Growth - Rank 1
        {
            sp->SP_coef_override = float(0.115050f);
        }break;
    case 48440: // Rejuvenation - Rank 14
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 48441: // Rejuvenation - Rank 15
        {
            sp->SP_coef_override = float(0.376040f);
        }break;
    case 48442: // Regrowth - Rank 11
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 48443: // Regrowth - Rank 12
        {
            sp->SP_coef_override = float(0.188000f);
        }break;
    case 48446: // Tranquility - Rank 6
        {
            sp->SP_coef_override = float(0.538000f);
        }break;
    case 48447: // Tranquility - Rank 7
        {
            sp->SP_coef_override = float(0.538000f);
        }break;
    case 48450: // Lifebloom - Rank 2
        {
            sp->SP_coef_override = float(0.095180f);
        }break;
    case 48451: // Lifebloom - Rank 3
        {
            sp->SP_coef_override = float(0.095180f);
        }break;
    case 48459: // Wrath - Rank 11
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 48461: // Wrath - Rank 12
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 48462: // Moonfire - Rank 13
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 48463: // Moonfire - Rank 14
        {
            sp->SP_coef_override = float(0.130000f);
        }break;
    case 48464: // Starfire - Rank 9
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 48465: // Starfire - Rank 10
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 48466: // Hurricane - Rank 5
        {
            sp->SP_coef_override = float(0.128980f);
        }break;
    case 48468: // Insect Swarm - Rank 7
        {
            sp->SP_coef_override = float(0.127000f);
        }break;
    case 48567: // Lacerate - Rank 2
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 48675: // Garrote - Rank 9
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 48676: // Garrote - Rank 10
        {
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 48781: // Holy Light - Rank 12
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 48782: // Holy Light - Rank 13
        {
            sp->SP_coef_override = float(1.660000f);
        }break;
    case 48784: // Flash of Light - Rank 8
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 48785: // Flash of Light - Rank 9
        {
            sp->SP_coef_override = float(1.000000f);
        }break;
    case 48800: // Exorcism - Rank 8
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 48801: // Exorcism - Rank 9
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 48805: // Hammer of Wrath - Rank 5
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 48806: // Hammer of Wrath - Rank 6
        {
            sp->SP_coef_override = float(0.150000f);
            sp->AP_coef_override = float(0.150000f);
        }break;
    case 48816: // Holy Wrath - Rank 4
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 48817: // Holy Wrath - Rank 5
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 48818: // Consecration - Rank 7
        {
            sp->SP_coef_override = float(0.040000f);
        }break;
    case 48819: // Consecration - Rank 8
        {
            sp->SP_coef_override = float(0.040000f);
        }break;
    case 48820: // Holy Shock - Rank 6
        {
            sp->SP_coef_override = float(0.810000f);
        }break;
    case 48821: // Holy Shock - Rank 7
        {
            sp->SP_coef_override = float(0.810000f);
        }break;
    case 48822: // Holy Shock - Rank 6
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 48823: // Holy Shock - Rank 7
        {
            sp->SP_coef_override = float(0.428600f);
        }break;
    case 48826: // Avenger's Shield - Rank 4
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 48827: // Avenger's Shield - Rank 5
        {
            sp->SP_coef_override = float(0.070000f);
            sp->AP_coef_override = float(0.070000f);
        }break;
    case 48951: // Holy Shield - Rank 5
        {
            sp->SP_coef_override = float(0.090000f);
            sp->AP_coef_override = float(0.056000f);
        }break;
    case 48952: // Holy Shield - Rank 6
        {
            sp->SP_coef_override = float(0.090000f);
            sp->AP_coef_override = float(0.056000f);
        }break;
    case 49000: // Serpent Sting - Rank 11
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 49001: // Serpent Sting - Rank 12
        {
            sp->RAP_coef_override = float(0.040000f);
        }break;
    case 49044: // Arcane Shot - Rank 10
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 49045: // Arcane Shot - Rank 11
        {
            sp->RAP_coef_override = float(0.150000f);
        }break;
    case 49047: // Multi-Shot - Rank 7
        {
            sp->RAP_coef_override = float(0.200000f);
        }break;
    case 49048: // Multi-Shot - Rank 8
        {
            sp->RAP_coef_override = float(0.200000f);
        }break;
    case 49051: // Steady Shot - Rank 3
        {
            sp->RAP_coef_override = float(0.100000f);
        }break;
    case 49052: // Steady Shot - Rank 4
        {
            sp->RAP_coef_override = float(0.100000f);
        }break;
    case 49053: // Immolation Trap - Rank 7
        {
            sp->RAP_coef_override = float(0.020000f);
        }break;
    case 49054: // Immolation Trap - Rank 8
        {
            sp->RAP_coef_override = float(0.020000f);
        }break;
    case 49230: // Earth Shock - Rank 9
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 49231: // Earth Shock - Rank 10
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 49232: // Flame Shock - Rank 8
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 49233: // Flame Shock - Rank 9
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 49235: // Frost Shock - Rank 6
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 49236: // Frost Shock - Rank 7
        {
            sp->SP_coef_override = float(0.385800f);
        }break;
    case 49237: // Lightning Bolt - Rank 13
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 49238: // Lightning Bolt - Rank 14
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 49270: // Chain Lightning - Rank 7
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 49271: // Chain Lightning - Rank 8
        {
            sp->SP_coef_override = float(0.400000f);
        }break;
    case 49272: // Healing Wave - Rank 13
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 49273: // Healing Wave - Rank 14
        {
            sp->SP_coef_override = float(1.610600f);
        }break;
    case 49275: // Lesser Healing Wave - Rank 8
        {
            sp->SP_coef_override = float(0.808200f);
        }break;
    case 49276: // Lesser Healing Wave - Rank 9
        {
            sp->SP_coef_override = float(0.808200f);
        }break;
    case 49280: // Lightning Shield - Rank 10
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 49281: // Lightning Shield - Rank 11
        {
            sp->SP_coef_override = float(0.330000f);
        }break;
    case 49283: // Earth Shield - Rank 4
        {
            sp->SP_coef_override = float(0.476100f);
        }break;
    case 49284: // Earth Shield - Rank 5
        {
            sp->SP_coef_override = float(0.476100f);
        }break;
    case 49799: // Rip - Rank 8
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 49800: // Rip - Rank 9
        {
            sp->AP_coef_override = float(0.010000f);
        }break;
    case 49821: // Mind Sear - Rank 1
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 50288: // Starfall - Rank 1
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 50294: // Starfall - Rank 1
        {
            sp->SP_coef_override = float(0.012000f);
        }break;
    case 50464: // Nourish - Rank 1
        {
            sp->SP_coef_override = float(0.661100f);
        }break;
    case 50516: // Typhoon - Rank 1
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 50590: // Immolation - Rank 1
        {
            sp->SP_coef_override = float(0.162200f);
        }break;
    case 50796: // Chaos Bolt - Rank 1
        {
            sp->SP_coef_override = float(0.713900f);
        }break;
    case 51505: // Lava Burst - Rank 1
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 52041: // Healing Stream Totem
        {
            sp->SP_coef_override = float(0.045000f);
        }break;
    case 52046: // Healing Stream Totem
        {
            sp->SP_coef_override = float(0.045000f);
        }break;
    case 52047: // Healing Stream Totem
        {
            sp->SP_coef_override = float(0.045000f);
        }break;
    case 52048: // Healing Stream Totem
        {
            sp->SP_coef_override = float(0.045000f);
        }break;
    case 52049: // Healing Stream Totem
        {
            sp->SP_coef_override = float(0.045000f);
        }break;
    case 52050: // Healing Stream Totem
        {
            sp->SP_coef_override = float(0.045000f);
        }break;
    case 52471: // Claw - Rank 10
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 52472: // Claw - Rank 11
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 52473: // Bite - Rank 10
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 52474: // Bite - Rank 11
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 52983: // Penance - Rank 2
        {
            sp->SP_coef_override = float(0.535000f);
        }break;
    case 52984: // Penance - Rank 3
        {
            sp->SP_coef_override = float(0.535000f);
        }break;
    case 52985: // Penance - Rank 4
        {
            sp->SP_coef_override = float(0.535000f);
        }break;
    case 52998: // Penance - Rank 2
        {
            sp->SP_coef_override = float(0.229000f);
        }break;
    case 52999: // Penance - Rank 3
        {
            sp->SP_coef_override = float(0.229000f);
        }break;
    case 53000: // Penance - Rank 4
        {
            sp->SP_coef_override = float(0.229000f);
        }break;
    case 53022: // Mind Sear - Rank 2
        {
            sp->SP_coef_override = float(0.714300f);
        }break;
    case 53188: // Starfall - Rank 2
        {
            sp->SP_coef_override = float(0.012000f);
        }break;
    case 53189: // Starfall - Rank 3
        {
            sp->SP_coef_override = float(0.012000f);
        }break;
    case 53190: // Starfall - Rank 4
        {
            sp->SP_coef_override = float(0.012000f);
        }break;
    case 53191: // Starfall - Rank 2
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 53194: // Starfall - Rank 3
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 53195: // Starfall - Rank 4
        {
            sp->SP_coef_override = float(0.050000f);
        }break;
    case 53223: // Typhoon - Rank 2
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 53225: // Typhoon - Rank 3
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 53226: // Typhoon - Rank 4
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 53248: // Wild Growth - Rank 2
        {
            sp->SP_coef_override = float(0.115050f);
        }break;
    case 53249: // Wild Growth - Rank 3
        {
            sp->SP_coef_override = float(0.115050f);
        }break;
    case 53251: // Wild Growth - Rank 4
        {
            sp->SP_coef_override = float(0.115050f);
        }break;
    case 53301: // Explosive Shot - Rank 1
        {
            sp->RAP_coef_override = float(0.140000f);
        }break;
    case 53308: // Entangling Roots - Rank 8
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 53339: // Mongoose Bite - Rank 6
        {
            sp->AP_coef_override = float(0.200000f);
        }break;
    case 53351: // Kill Shot - Rank 1
        {
            sp->RAP_coef_override = float(0.400000f);
        }break;
    case 53719: // Seal of the Martyr
        {
            sp->SP_coef_override = float(0.250000f);
        }break;
    case 53726: // Judgement of the Martyr - Rank 1
        {
            sp->SP_coef_override = float(0.250000f);
            sp->AP_coef_override = float(0.160000f);
        }break;
    case 55359: // Living Bomb - Rank 2
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 55360: // Living Bomb - Rank 3
        {
            sp->SP_coef_override = float(0.200000f);
        }break;
    case 55458: // Chain Heal - Rank 6
        {
            sp->SP_coef_override = float(0.800000f);
        }break;
    case 55459: // Chain Heal - Rank 7
        {
            sp->SP_coef_override = float(0.800000f);
        }break;
    case 55482: // Fire Breath - Rank 3
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 55483: // Fire Breath - Rank 4
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 55484: // Fire Breath - Rank 5
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 55485: // Fire Breath - Rank 6
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 55555: // Poison Spit - Rank 4
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 55556: // Poison Spit - Rank 5
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 55557: // Poison Spit - Rank 6
        {
            sp->RAP_coef_override = float(0.125000f);
        }break;
    case 55728: // Scorpid Poison - Rank 6
        {
            sp->RAP_coef_override = float(0.015000f);
        }break;
    case 56641: // Steady Shot - Rank 1
        {
            sp->RAP_coef_override = float(0.100000f);
        }break;
    case 57755: // Heroic Throw
        {
            sp->AP_coef_override = float(0.500000f);
        }break;
    case 57823: // Revenge - Rank 9
        {
            sp->AP_coef_override = float(0.207000f);
        }break;
    case 57964: // Instant Poison VIII - Rank 8
        {
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 57965: // Instant Poison IX - Rank 9
        {
            sp->AP_coef_override = float(0.100000f);
        }break;
    case 57969: // Deadly Poison VIII - Rank 8
        {
            sp->AP_coef_override = float(0.024000f);
        }break;
    case 57970: // Deadly Poison IX - Rank 9
        {
            sp->AP_coef_override = float(0.024000f);
        }break;
    case 57974: // Wound Poison VI - Rank 6
        {
            sp->AP_coef_override = float(0.008000f);
        }break;
    case 57975: // Wound Poison VII - Rank 7
        {
            sp->AP_coef_override = float(0.008000f);
        }break;
    case 58597: // Sacred Shield - Rank 1
        {
            sp->SP_coef_override = float(0.750000f);
        }break;
    case 58700: // Attack - Rank 8
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 58701: // Attack - Rank 9
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 58702: // Attack - Rank 10
        {
            sp->SP_coef_override = float(0.166700f);
        }break;
    case 58732: // Magma Totem - Rank 6
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 58735: // Magma Totem - Rank 7
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 58759: // Healing Stream Totem
        {
            sp->SP_coef_override = float(0.045000f);
        }break;
    case 58760: // Healing Stream Totem
        {
            sp->SP_coef_override = float(0.045000f);
        }break;
    case 58761: // Healing Stream Totem
        {
            sp->SP_coef_override = float(0.045000f);
        }break;
    case 58786: // Flametongue Weapon Proc - Rank 8
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 58787: // Flametongue Weapon Proc - Rank 9
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 58788: // Flametongue Weapon Proc - Rank 10
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 58797: // Frostbrand Attack - Rank 7
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 58798: // Frostbrand Attack - Rank 8
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 58799: // Frostbrand Attack - Rank 9
        {
            sp->SP_coef_override = float(0.100000f);
        }break;
    case 59092: // Dark Pact - Rank 5
        {
            sp->SP_coef_override = float(0.960000f);
        }break;
    case 59161: // Haunt - Rank 2
        {
            sp->SP_coef_override = float(0.479300f);
        }break;
    case 59163: // Haunt - Rank 3
        {
            sp->SP_coef_override = float(0.479300f);
        }break;
    case 59164: // Haunt - Rank 4
        {
            sp->SP_coef_override = float(0.479300f);
        }break;
    case 59170: // Chaos Bolt - Rank 2
        {
            sp->SP_coef_override = float(0.713900f);
        }break;
    case 59171: // Chaos Bolt - Rank 3
        {
            sp->SP_coef_override = float(0.713900f);
        }break;
    case 59172: // Chaos Bolt - Rank 4
        {
            sp->SP_coef_override = float(0.713900f);
        }break;
    case 60043: // Lava Burst - Rank 2
        {
            sp->SP_coef_override = float(0.571400f);
        }break;
    case 60051: // Explosive Shot - Rank 2
        {
            sp->RAP_coef_override = float(0.140000f);
        }break;
    case 60052: // Explosive Shot - Rank 3
        {
            sp->RAP_coef_override = float(0.140000f);
        }break;
    case 60053: // Explosive Shot - Rank 4
        {
            sp->RAP_coef_override = float(0.140000f);
        }break;
    case 60089: // Faerie Fire (Feral)
        {
            sp->AP_coef_override = float(0.050000f);
        }break;
    case 61005: // Kill Shot - Rank 2
        {
            sp->RAP_coef_override = float(0.400000f);
        }break;
    case 61006: // Kill Shot - Rank 3
        {
            sp->RAP_coef_override = float(0.400000f);
        }break;
    case 61295: // Riptide - Rank 1
        {
            sp->SP_coef_override = float(0.180000f);
        }break;
    case 61299: // Riptide - Rank 2
        {
            sp->SP_coef_override = float(0.180000f);
        }break;
    case 61300: // Riptide - Rank 3
        {
            sp->SP_coef_override = float(0.180000f);
        }break;
    case 61301: // Riptide - Rank 4
        {
            sp->SP_coef_override = float(0.180000f);
        }break;
    case 61384: // Typhoon - Rank 5
        {
            sp->SP_coef_override = float(0.193000f);
        }break;
    case 61650: // Fire Nova - Rank 8
        {
            sp->SP_coef_override = float(0.214200f);
        }break;
    case 61654: // Fire Nova - Rank 9
        {
            sp->SP_coef_override = float(0.214200f);
        }break;
    case 62124: // Hand of Reckoning
        {
            sp->SP_coef_override = float(0.085000f);
        }break;
    case 62606: // Savage Defense
        {
            sp->AP_coef_override = float(0.250000f);
        }break;
    default:
        {
            sp->SP_coef_override = 0.0f;
            sp->AP_coef_override = 0.0f;
            sp->RAP_coef_override = 0.0f;
        }break;
    }
}
