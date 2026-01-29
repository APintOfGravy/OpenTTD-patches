/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/pool_func.hpp"

#include "depot_base.h"
#include "depot_map.h"
#include "direction_func.h"

#include "consist.h"
#include "vehicle_base.h"
#include "vehicle_type.h"
#include "roadveh.h"
#include "train.h"
#include "ship.h"
#include "aircraft.h"

#include <cassert>
#include <utility>
#include <vector>

ConsistPool _consist_pool("Consist");
INSTANTIATE_POOL_METHODS(Consist)

bool IsUnitPowered();

bool IsUnitPowered(Train* t) { return t->IsEngine(); };
bool IsUnitPowered(RoadVehicle* rv) { return rv->IsEngine(); };
bool IsUnitPowered(Ship* s) { return !s->IsArticulatedPart(); };
bool IsUnitPowered(Aircraft* a) { return a->IsNormalAircraft(); };



Consist::Consist()
{
}

Consist::Consist(Vehicle* v) : Consist::Consist()
{
	Debug(misc, 0, "Consist(): Creating new Consist {}", this->index);

	this->SetFirst(v);

	#if OTTD_UPPER_TAGGED_PTR
	VehiclePoolOps::SetIsNonFrontVehiclePtr(_vehicle_pool.GetRawRef(v->index.base()), false);
	#endif

	this->type = v->type;
	this->owner = v->owner;
	this->group_id = DEFAULT_GROUP;

	// this->consist_flags.Set(ConsistFlag::AutomateTimetable, Company::Get(_current_company)->settings.vehicle.auto_timetable_by_default);
	// this->consist_flags.Set(ConsistFlag::TimetableSeparation, Company::Get(_current_company)->settings.vehicle.auto_separation_by_default);

	for (Vehicle* u = v; u != nullptr; u = u->Next())
	{
		Debug(misc, 0, "Consist(): including unit {}", u->index);
		u->SetConsist(this);
		if (v->IsUnitPowered())
		{
			this->powered_units.emplace(v);
		}
	}

	if (IsDepotTile(v->tile))
	{
		Debug(misc, 0, "Consist(): Assigning depot to Consist.");
		Depot* depot = Depot::GetByTile(this->FirstVehicle()->tile);
		depot->vehicles.emplace(this);
	}
}

Consist::~Consist()
{
	if (CleaningPool()) return;

	if (this->FirstVehicle() != nullptr && IsDepotTile(this->FirstVehicle()->tile))
	{
		Depot::GetByTile(this->FirstVehicle()->tile)->vehicles.erase(this);
	}
}

Vehicle* Consist::FirstVehicle() { return this->first; }
const Vehicle* Consist::FirstVehicle() const { return this->first; }

template<typename V>
V* Consist::FirstVehicle() { return (V*)this->FirstVehicle(); }

Vehicle* Consist::LastVehicle()
{
	Vehicle* v = this->FirstVehicle();

	while (v->Next() != nullptr) v = v->Next();

	return v;
}

void Consist::SetFirst(Vehicle* v)
{
	assert(v != nullptr);

	this->first = v;
}

void Consist::ConnectAhead(Vehicle* target, Vehicle* new_v)
{

}

void Consist::ConnectBehind(Vehicle* target, Vehicle* new_v, bool chain)
{
	if (!Consist::CanAllocateItem()) return;

	assert(target->consist == this);

	if (new_v != nullptr)
	{
		int num = (new_v->consist == nullptr) ? -1 : new_v->consist->index.base();
		Debug(misc, 0, "Consist::ConnectBehind() 1: Vehicle U-{} in Consist C-{} is having new Vehicle U-{} connected behind it, originating from Consist C-{}.", target->index, this->index, new_v->index, num);

		if (chain)
		{
			if (new_v->Previous() != nullptr)
			{
				new_v->Previous()->next = nullptr;
			}
			for (Vehicle* v = new_v; v != nullptr; v = v->Next())
			{
				v->SetConsist(this);
			}
		}
		else
		{
			if (new_v->Previous() != nullptr)
			{
				new_v->Previous()->next = new_v->Next();
			}
			if (new_v->Next() != nullptr)
			{
				if (new_v->Previous() != nullptr)
				{
					Debug(misc, 0, "Consist::ConnectBehind() 1.1: Decoupling U-{} between U-{} and U-{}.", new_v->index, new_v->Previous()->index, new_v->Next()->index);
					new_v->Next()->previous = new_v->Previous();
				}
				else
				{
					Debug(misc, 0, "Consist::ConnectBehind() 1.1: U-{} is the leader. U-{} is being made the leader. Lucky U-{}.", new_v->index, new_v->Next()->index, new_v->Next()->index);
					Vehicle* next = new_v->Next();
					next->previous = nullptr;
					next->consist->SetFirst(next);
				}
			}
			new_v->SetConsist(this);
			new_v->next = target->Next();
			if (target->Next() != nullptr)
			{
				target->Next()->previous = new_v;
			}
		}

		new_v->previous = target;
		target->next = new_v;
	}
	else
	{
		Debug(misc, 0, "Consist::ConnectBehind() 2: Vehicle U-{} in Consist C-{} is having it's following contents decoupled.", target->index, this->index);

		Vehicle* next = target->Next();

		target->next = nullptr;
		next->previous = nullptr;

		if (!chain && next->Next() != nullptr)
		{
			Debug(misc, 0, "Consist::ConnectBehind() 2.1: Not doing a chain.");
			next->Next()->previous = target;
			target->next = next->Next();
			next->next = nullptr;
		}

		new Consist(next);

		next->MarkDirty();
	}
	this->FirstVehicle()->MarkDirty();

	Debug(misc, 0, "Consist::ConnectBehind() END.");
}

void Consist::Reverse(bool first)
{
	for (Consist* c : this->children_ahead) c->Reverse(false);
	this->ReverseChain();
	for (Consist* c : this->children_behind) c->Reverse(false);
}

void Consist::ReverseChain()
{
	Vehicle* v = this->FirstVehicle();

	while (v != nullptr)
	{
		v->direction = ReverseDir(v->direction);
		std::swap(v->previous, v->next);
		v = v->Previous();
	}
}

void Consist::EraseFromChain(Vehicle* v)
{
	Debug(misc, 0, "Consist::RemoveFromChain(): Removing vehicle U-{} from Consist C-{}", v->index, this->index);
	assert(v != nullptr);

	if (v->IsUnitPowered())
	{
		this->powered_units.erase(v);
	}

	if (this->FirstVehicle() == v)
	{
		if (this->FirstVehicle()->Next() == nullptr)
		{
			Debug(misc, 0, "Consist::RemoveFromChain(): Removing Consist {}", this->index);
			delete this;
		}
		else
		{
			this->SetFirst(v->Next());
		}
	}
	v->consist = nullptr;
}