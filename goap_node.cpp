#include "goap_node.h"
#include "goap/goap.h"
#include "goap_action.h"
#include "core/error/error_macros.h"
#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"
#include <algorithm>

void GoapNode::_notification(int p_what) {
}

void GoapNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("pop_next_action"), &GoapNode::pop_next_action);
	ClassDB::bind_method(D_METHOD("skip_next_action"), &GoapNode::skip_next_action);
	ClassDB::bind_method(D_METHOD("create_plan", "update_cost"), &GoapNode::create_plan);

	ClassDB::bind_method(D_METHOD("clear_plan"), &GoapNode::clear_plan);
        ClassDB::bind_method(D_METHOD("clear_world_state"), &GoapNode::clear_world_state);
        ClassDB::bind_method(D_METHOD("clear_goal_state"), &GoapNode::clear_goal_state);
	ClassDB::bind_method(D_METHOD("clear_actions"), &GoapNode::clear_actions);

	ClassDB::bind_method(D_METHOD("set_user_object", "user_object"), &GoapNode::set_user_object);
	ClassDB::bind_method(D_METHOD("add_action_resource", "node"), &GoapNode::add_action_resource);
        ClassDB::bind_method(D_METHOD("add_world_state", "data"), &GoapNode::add_world_state);
        ClassDB::bind_method(D_METHOD("add_goal_state", "data"), &GoapNode::add_goal_state);

	ClassDB::bind_method(D_METHOD("set_action_precondition", "action_name", "precondition"), &GoapNode::set_action_precondition);
	ClassDB::bind_method(D_METHOD("set_action_postcondition", "action_name", "postcondition"), &GoapNode::set_action_postcondition);
	ClassDB::bind_method(D_METHOD("set_action_cost", "name", "cost"), &GoapNode::set_action_cost);

	ClassDB::bind_method(D_METHOD("get_plan_action_names"), &GoapNode::get_plan_action_names);

        BIND_ENUM_CONSTANT(FOUND);
        BIND_ENUM_CONSTANT(NOT_FOUND);
        BIND_ENUM_CONSTANT(UNCHANGED);
}

void GoapNode::set_user_object(Object* object) {
        user_object = object;
}

void GoapNode::add_action_resource(const Ref<Resource> &action) {
        auto res = Object::cast_to<GoapAction>(action.ptr());
        auto name = res->get_name();
        action_map[name] = action;
        set_action_precondition(name, res->get_precondition_data());
        set_action_postcondition(name, res->get_effect_data());
}

void GoapNode::add_world_state(const Dictionary& data) {
	int size = data.size();
	Array keys = data.keys();
	Array values = data.values();
	for (int i = 0; i < size; i++) {
		String state = String(keys[i]);
		bool val = values[i];

                ERR_FAIL_COND_MSG(
                    !goap_worldstate_set(&action_planner,
				&world_state,
				state.ascii().ptr(),
				val),
                    "No state named " + state);
	}
}

void GoapNode::add_goal_state(const Dictionary& data) {
	int size = data.size();
	Array keys = data.keys();
	Array values = data.values();
	for (int i = 0; i < size; i++) {
		String state = String(keys[i]);
		bool val = values[i];
		ERR_FAIL_COND_MSG(
                    !goap_worldstate_set(&action_planner,
				&goal_state,
				state.ascii().ptr(),
				val),
                    "No state named " + state);
	}
}

void GoapNode::set_action_precondition(const String& action_name, const Dictionary& data) {
	int size = data.size();
	Array keys = data.keys();
	Array values = data.values();
	for (int i = 0; i < size; i++) {
		String pre = String(keys[i]);
                bool val = values[i];
		ERR_FAIL_COND_MSG(
                    !goap_set_pre(&action_planner,
                                  action_name.ascii().ptr(),
                                  pre.ascii().ptr(),
                                  val),
                    "No action named " + action_name);
	}
}

void GoapNode::set_action_postcondition(const String& action_name, const Dictionary& data) {
	int size = data.size();
	Array keys = data.keys();
	Array values = data.values();
	for (int i = 0; i < size; i++) {
		String post = String(keys[i]);
		bool val = values[i];
		ERR_FAIL_COND_MSG(
                    !goap_set_pst(&action_planner,
                                  action_name.ascii().ptr(),
                                  post.ascii().ptr(),
                                  val),
                    "No action named " + action_name);
	}
}

void GoapNode::set_action_cost(const String& name, int cost) {
        const char* action_name = name.ascii().ptr();
        ERR_FAIL_COND_MSG(
            !goap_set_cost(&action_planner, action_name, cost),
            "No action named " + name);
}

Vector<String> GoapNode::get_plan_action_names() {
        return plan_action_names;
}

void GoapNode::clear_plan() {
        plan_action_names.resize(0);

        plan_cost = -1;
        plan_index = -1;
        plan_size = -1;
}

void GoapNode::clear_world_state() {
         goap_worldstate_clear(&world_state);
}

void GoapNode::clear_goal_state() {
        goap_worldstate_clear(&goal_state);
}

void GoapNode::clear_actions() {
	goap_actionplanner_clear(&action_planner);
}

GoapNode::PlanResult GoapNode::create_plan(bool update_cost) {
	PlanResult result = NOT_FOUND;

        if (action_map.is_empty()) {
                WARN_PRINT("No GoapAction nodes added.");
                return NOT_FOUND;
        }

	if (update_cost) {
		if (!user_object) {
			WARN_PRINT("No user object set.");
			return NOT_FOUND;
		}

		for (auto E = action_map.begin(); E; ++E) {
                        ERR_FAIL_COND_V(E->value.is_null(), NOT_FOUND);
			auto *g = Object::cast_to<GoapAction>(E->value.ptr());
                        int cost = g->_count_cost(user_object, g->cost);
                        set_action_cost(E->key, cost);
		}
	}

	Dictionary d;
	worldstate_t states[16];
	const char *plan[16]; // The planner will return the action plan in this array.
	int new_plan_size = 16; // Size of our return buffers.
	int new_plan_cost = astar_plan(
			&action_planner, world_state,
                        goal_state, plan, states, &new_plan_size);

	Vector<String> new_action_names;

	if (new_plan_cost != -1) {
                new_action_names.resize(new_plan_size);
		for (int i = 0; i < new_plan_size; i++) {
			const char *name = plan[i];
			new_action_names.set(i, String(name));
                        auto o = action_map.find(String(name));
                        if (o == action_map.end()) {
                                WARN_PRINT("GoapAction "+String(name)+" not found");
                                return NOT_FOUND;
                        }
		}

		result = new_action_names == plan_action_names
                         ? UNCHANGED : FOUND;
                if (result == FOUND) {
                        plan_action_names = new_action_names;
                        plan_cost = new_plan_cost;
                        plan_size = new_plan_size;
                        plan_index = 0;
                }
	} else {
		WARN_PRINT("Route to goal not found");
	}

	return result;
}

Ref<GoapAction> GoapNode::pop_next_action() {
	if (plan_size != -1 && plan_index < plan_size) {
		if (!user_object) {
			WARN_PRINT("No user object set.");
			return nullptr;
		}
                // auto action = action_map[plan_action_names[plan_index]];
                auto action = action_map[plan_action_names[plan_index]];

		plan_index++;
                return action;
	}
        else {
                return nullptr;
        }
}

void GoapNode::skip_next_action() {
        plan_index++;
}

GoapNode::GoapNode() {
        clear_actions();
        clear_plan();
        clear_world_state();

        user_object = nullptr;
}

GoapNode::~GoapNode() {
        clear_actions();
        clear_plan();
        clear_world_state();
}
