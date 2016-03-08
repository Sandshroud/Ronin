/***
 * Demonstrike Core
 */

#pragma once

class CreatureData;
class CreatureInfoExtra;

class SERVER_DECL AIInterface
{
public:
    AIInterface(Unit *unit, Unit *owner = NULL);
    ~AIInterface();

	// Update
    void Update(uint32 p_time);

private:
    Unit* m_Unit;

public:
    bool m_is_in_instance;
	bool m_AllowedToEnterCombat;
	bool m_canCallForHelp;
	float m_CallForHelpHealth;
	bool m_canRangedAttack;
	bool m_canFlee;
	float m_FleeHealth;
	uint32 m_FleeDuration;
	int sendflee_message;
	std::string flee_message;
};
