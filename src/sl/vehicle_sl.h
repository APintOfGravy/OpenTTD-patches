/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file vehicle_sl.h Code handling saving and loading of vehicles. */

#ifndef SL_VEHICLE_SL_H
#define SL_VEHICLE_SL_H

#include "../transport_type.h"
#include "../group_type.h"
#include "../order_base.h"
#include "../base_consist.h"
#include "saveload_types.h"

#include <map>

struct DispatchRecordsStructHandlerBase : public SaveLoadStructHandler {
	using RecordPair = std::pair<const uint16_t, LastDispatchRecord>;

	NamedSaveLoadTable GetDescription() const override;
	void SaveDispatchRecords(btree::btree_map<uint16_t, LastDispatchRecord> &records) const;
	void LoadDispatchRecords(btree::btree_map<uint16_t, LastDispatchRecord> &records) const;
};

struct LegacyVSLProps
{
	TinyString name = TinyString();                            ///< Name of vehicle

	btree::btree_map<uint16_t, LastDispatchRecord> dispatch_records{}; ///< Records of last scheduled dispatches

	/* Used for timetabling. */
	uint32_t current_order_time = 0;              ///< How many ticks have passed since this order started.
	int32_t lateness_counter = 0;                 ///< How many ticks late (or early if negative) this vehicle is.
	StateTicks timetable_start{};                 ///< When the vehicle is supposed to start the timetable.

	uint16_t service_interval = 0;                ///< The interval for (automatic) servicing; either in days or %.

	VehicleOrderID cur_real_order_index = 0;      ///< The index to the current real (non-implicit) order
	VehicleOrderID cur_implicit_order_index = 0;  ///< The index to the current implicit order
	VehicleOrderID cur_timetable_order_index = 0; ///< The index to the current real (non-implicit) order used for timetable updates

	VehicleFlags consist_flags{};                 ///< Used for gradual loading and other miscellaneous things (@see VehicleFlags enum)

	Vehicle* first = nullptr;

	Vehicle *next_shared = nullptr;              ///< pointer to the next vehicle that shares the order

	UnitID unitnumber = 0;                       ///< unit number, for display purposes only
	uint8_t day_counter = 0;                     ///< Increased by one for each day
	uint8_t tick_counter = 0;                    ///< Increased by one for each tick

	GroupID group_id = GroupID::Invalid();       ///< Index of group Pool array

	Order current_order{};                       ///< The current order (+ status, like: loading)
	OrderList *orders = nullptr;                 ///< Pointer to the order list for this vehicle

	CargoPayment* cargo_payment = nullptr;       ///< The cargo payment we're currently in

	StationID last_station_visited = StationID::Invalid(); ///< The last station we stopped at.
	StationID last_loading_station = StationID::Invalid(); ///< Last station the vehicle has stopped at and could possibly leave from with any cargo loaded. (See ConsistFlag::LastLoadStationSeparate).
	StateTicks last_loading_tick{};              ///< Last tick (_state_ticks) the vehicle has stopped at a station and could possibly leave with any cargo loaded. (See ConsistFlag::LastLoadStationSeparate).

	int8_t trip_occupancy = 0;                   ///< NOSAVE: Occupancy of vehicle of the current trip (updated after leaving a station).
	uint32_t current_loading_time = 0;           ///< How long loading took. Less than current_order_time if vehicle is early.
};

static std::map<VehicleID, LegacyVSLProps> _legacy_vsl_props = {};

#endif /* SL_NEWGRF_SL_H */
