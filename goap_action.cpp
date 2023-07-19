#include "goap_action.h"
#include "core/io/resource.h"
#include "core/math/math_funcs.h"
#include "core/object/object.h"
#include <cstdint>
#include <cstdio>

void GoapAction::_notification(int p_what) {

}

void GoapAction::_bind_methods() {

	ClassDB::bind_method(D_METHOD("add_precondition_data", "data_dict"), &GoapAction::add_precondition_data);
	ClassDB::bind_method(D_METHOD("get_precondition_data"), &GoapAction::get_precondition_data);

	ClassDB::bind_method(D_METHOD("add_effect_data", "data_dict"), &GoapAction::add_effect_data);
	ClassDB::bind_method(D_METHOD("get_effect_data"), &GoapAction::get_effect_data);

	ClassDB::bind_method(D_METHOD("set_cost", "cost"), &GoapAction::set_cost);
	ClassDB::bind_method(D_METHOD("get_cost"), &GoapAction::get_cost);


	ClassDB::bind_method(D_METHOD("set_cost_random", "cost_random"), &GoapAction::set_cost_random);
	ClassDB::bind_method(D_METHOD("get_cost_random"), &GoapAction::get_cost_random);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "cost", PROPERTY_HINT_RANGE, "0,100,1,or_greater"), "set_cost", "get_cost");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cost_random", PROPERTY_HINT_RANGE, "0,1,0.05,or_greater"), "set_cost_random", "get_cost_random");

        GDVIRTUAL_BIND(_count_cost, "user_object" ,"cost");

        ADD_SIGNAL(MethodInfo("goap_action_done"));
}

int GoapAction::get_cost() const {
        return cost;
}

void GoapAction::set_cost(const int cost) {
        this->cost = cost;
}

float GoapAction::get_cost_random() const {
        return cost_random;
}

void GoapAction::set_cost_random(const float cost_random) {
        this->cost_random = cost_random;
}

void GoapAction::add_precondition_data(const Dictionary& data) {
        int size = data.size();
	Array keys = data.keys();
	Array values = data.values();
	for (int i = 0; i < size; i++) {
		String pre = String(keys[i]);
		bool val = values[i];
                precondition_data.insert(pre, val);
	}
}

Dictionary GoapAction::get_precondition_data() const {
        Dictionary b;
	for (const auto &E : precondition_data) {
                b[E.key] = E.value;
        }
        return b;
}

void GoapAction::add_effect_data(const Dictionary& data) {
        int size = data.size();
	Array keys = data.keys();
	Array values = data.values();
	for (int i = 0; i < size; i++) {
		String pre = String(keys[i]);
		bool val = values[i];
                effect_data.insert(pre, val);
	}
}

Dictionary GoapAction::get_effect_data() const {
        Dictionary b;
	for (const auto &E : effect_data) {
                b[E.key] = E.value;
        }
        return b;
}

int GoapAction::_count_cost(const Object *object, int cost) {
        cost *= (1.0f - this->cost_random * Math::randf());
        int vcost;
        if (GDVIRTUAL_CALL(_count_cost, cost, object, vcost)) {
                return vcost;
        }
        return cost;
}

GoapAction::GoapAction() :
		cost(1),
                cost_random(0)
{
        set_name("GoapAction");
}
