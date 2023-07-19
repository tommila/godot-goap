#pragma once

#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/variant/dictionary.h"

#include "scene/main/node.h"

#include "map"
#include "vector"
#include "string"
#include <cstdint>

class GoapNode;

class GoapAction : public Resource {
	GDCLASS(GoapAction, Resource);

protected:
        int cost;
        float cost_random;

        HashMap<String, bool> precondition_data;
        HashMap<String, bool> effect_data;

	void _notification(int p_what);
	static void _bind_methods();

        friend class GoapNode;

public:
        int get_cost() const;
        void set_cost(const int cost);

        float get_cost_random() const;
        void set_cost_random(const float cost_random);

        void add_precondition_data(const Dictionary& data);
        Dictionary get_precondition_data() const;

        void add_effect_data(const Dictionary& data);
        Dictionary get_effect_data() const;

        virtual int _count_cost(const Object* object, int cost);

	GDVIRTUAL2RC(int, _count_cost, int, const Object*)

	GoapAction();
};
