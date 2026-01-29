/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file consist_type.h Declares the Consist class, and other important data types, before the Consist class is defined. */

#include "core/enum_type.hpp"
#include "core/pool_id_type.hpp"

#ifndef CONSIST_TYPE_H
#define CONSIST_TYPE_H

struct ConsistIDTag : public PoolIDTraits<uint32_t, 0xFF000, 0xFFFFF> {};
using ConsistID = PoolID<ConsistIDTag>;

struct ConsistOrders;
struct Consist;

/** Vehicle state bits in #Vehicle::vehstatus. */
enum class ConsistState : uint8_t {
	Hidden         = 0, ///< Vehicle is not visible.
	Stopped        = 1, ///< Vehicle is stopped by the player.
	Unclickable    = 2, ///< Vehicle is not clickable by the user (shadow vehicles).
	DefaultPalette = 3, ///< Use default vehicle palette. @see DoDrawVehicle
	TrainSlowing   = 4, ///< Train is slowing down.
	Shadow         = 5, ///< Vehicle is a shadow vehicle.
	AircraftBroken = 6, ///< Aircraft is broken down.
	Crashed        = 7, ///< Vehicle is crashed.
};
using ConsistStates = EnumBitSet<ConsistState, uint8_t>;

#endif /* CONSIST_TYPE_H */