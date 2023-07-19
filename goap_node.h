#pragma once

#include "goap/goap.h"
#include "goap/astar.h"

#include "core/io/resource.h"
#include "core/variant/dictionary.h"
#include "core/object/object.h"
#include "core/templates/vector.h"

#include "scene/main/node.h"

#include "map"
#include "vector"
#include "string"
#include <map>

class GoapAction;

class GoapNode : public Node {
	GDCLASS(GoapNode, Node);
public:
	enum PlanResult {
		FOUND     = 0,
                NOT_FOUND = 1,
                UNCHANGED = 2
	};

protected:
	worldstate_t world_state;
	worldstate_t goal_state;

	actionplanner_t action_planner;

        HashMap<String, Ref<Resource>> action_map;

        Vector<String> plan_action_names;

        Object* user_object;

        int plan_size;
        int plan_cost;
        int plan_index;

	void _notification(int p_what);
	static void _bind_methods();

public:
        void set_user_object(Object *data);

	void add_action_resource(const Ref<Resource> &action);

        void add_world_state(const Dictionary &state_bundle);
        void add_goal_state(const Dictionary &state_bundle);

	void set_action_precondition(const String& action_name,
                                     const Dictionary &data);

	void set_action_postcondition(const String& action_name,
                                      const Dictionary &data);

        void set_action_cost(const String& name, int cost);

        Vector<String> get_plan_action_names();

	void clear_plan();
	void clear_world_state();
	void clear_goal_state();
	void clear_actions();
	PlanResult create_plan(bool update_cost);

        Ref<GoapAction> pop_next_action();
        void skip_next_action();

	GoapNode();
	~GoapNode();
};

VARIANT_ENUM_CAST(GoapNode::PlanResult);
