/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file consist.h Defines the Consist class - a 'manager' responsible for handling a Consist chain's orders. */

#include "core/alignment.hpp"

#include "company_type.h"
#include "group_type.h"
#include "order_base.h"
#include "base_consist.h"
#include "consist_type.h"
#include "transport_type.h"
#include "vehicle_type.h"

#include <vector>
#include <set>

#ifndef CONSIST_H
#define CONSIST_H

typedef Pool<Consist, ConsistID, 512> ConsistPool;

extern ConsistPool _consist_pool;

struct GroundVehicleCache;

/**
 *
 */
struct Consist : ConsistPool::PoolItem<&_consist_pool>, BaseConsist
{
	friend NamedSaveLoadTable GetConsistDescription();

	/* Basic Info */
	VehicleType type = VEH_INVALID;
	Owner owner = INVALID_OWNER;
	GroupID group_id = GroupID::Invalid();       ///< Index of group Pool array
	UnitID unitnumber = 0;                       ///< unit number, for display purposes only

	Money profit_this_year = 0;                  ///< Profit this year << 8, low 8 bits are fract
	Money profit_last_year = 0;                  ///< Profit last year << 8, low 8 bits are fract
	Money profit_lifetime = 0;                   ///< Profit lifetime << 8, low 8 bits are fract

	bool is_virtual = false;

	OrderList* orders = nullptr;
	Order current_order = {};

	Vehicle* first;
	std::set<Vehicle*> powered_units = {};

	private:
	// ConsistStates status{};                       ///< Status

	public:



	/* Misc */
	uint8_t day_counter = 0;                     ///< Increased by one for each day
	uint8_t tick_counter = 0;                    ///< Increased by one for each tick

	Depot* current_depot = nullptr;



	/* Connected Consists */
	std::vector<Consist*> children_ahead = {};         ///< A container of this Consist, and any Consists that this Consist controls.
	std::vector<Consist*> children_behind = {};         ///< A container of this Consist, and any Consists that this Consist controls.
	Consist* parent = nullptr;              ///< nullptr, or a Consist that controls this Consist.



	/* Group */
	Consist* next_shared = nullptr;              ///< pointer to the next vehicle that shares the order
	Consist* previous_shared = nullptr;          ///< NOSAVE: pointer to the previous vehicle in the shared order chain



	/* Order Data Details*/
	CargoPayment* cargo_payment = nullptr;       ///< The cargo payment we're currently in

	StationID last_station_visited = StationID::Invalid(); ///< The last station we stopped at.
	StationID last_loading_station = StationID::Invalid(); ///< Last station the vehicle has stopped at and could possibly leave from with any cargo loaded. (See ConsistFlag::LastLoadStationSeparate).
	StateTicks last_loading_tick{};              ///< Last tick (_state_ticks) the vehicle has stopped at a station and could possibly leave with any cargo loaded. (See ConsistFlag::LastLoadStationSeparate).

	uint8_t order_occupancy_average = 0;         ///< NOSAVE: order occupancy average. 0 = invalid, 1 = n/a, 16-116 = 0-100%
	int8_t trip_occupancy = 0;                   ///< NOSAVE: Occupancy of vehicle of the current trip (updated after leaving a station).
	uint32_t current_order_time = 0;              ///< How many ticks have passed since this order started.
	uint32_t current_loading_time = 0;           ///< How long loading took. Less than current_order_time if vehicle is early.
	// std::unique_ptr<VehicleUnbunchState> unbunch_state{};



	/* Creation/Deletion */
	Consist();
	Consist(Vehicle* v);

	~Consist();



	/* Position */
	void Reverse(bool first = true);

	Vehicle* FirstVehicle();
	const Vehicle* FirstVehicle() const;

	template<typename V>
	V* FirstVehicle();

	void SetFirst(Vehicle* v);

	void ConnectAhead(Vehicle* target, Vehicle* new_v);
	void ConnectBehind(Vehicle* target, Vehicle* new_v, bool chain);

	Vehicle* LastVehicle();

	void EraseFromChain(Vehicle*);

protected:
	void ReverseChain();

	void AddToChain(Vehicle*, auto);
public:
};

#endif /* CONSIST_H */