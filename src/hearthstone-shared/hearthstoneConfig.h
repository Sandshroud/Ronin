/***
 * Demonstrike Core
 */

// Hearthstone Configuration Header File
// Feel free to customize to your needs.

#pragma once

/** Enable/disable Hearthstone world server cluster support.
 * Warning: Hearthstone will no longer function independantly when built in cluster mode.
 * It will require a realmserver to function.
 * As of last update, it is nowhere near complete, only enable for dev use.
 * Default: Disabled
 */
//#define CLUSTERING 1

// For Linux, disabled for compile reasons.
//#define LOAD_LACRIMI

/* This makes it so that object pushing for players is handled in a different
 * thread than the map manager, so if a player enters a heavily populated area,
 * he doesn't get a hang up, and information is sent after the player enters world.
 */
//#define MULTI_THREADED_OBJECT_PUSHING
