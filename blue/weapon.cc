#include "stdafx.hh"

#include "netvar.hh"
#include "weapon.hh"

using namespace TF;

auto next_primary_attack = Netvar("DT_BaseCombatWeapon", "LocalActiveWeaponData", "m_flNextPrimaryAttack");
auto Weapon::next_primary_attack() -> float {
    return ::next_primary_attack.get<float>(this);
}

auto next_secondary_attack = Netvar("DT_BaseCombatWeapon", "LocalActiveWeaponData", "m_flNextSecondaryAttack");
auto Weapon::next_secondary_attack() -> float {
    return ::next_secondary_attack.get<float>(this);
}

auto Weapon::owner() -> Entity * {
    // TODO:
    return nullptr;
}
