#pragma once
#include <memory>
#include <vector>
#include "AnimationBase.h"

namespace animation
{
	class AnimationCurve;

	class MutatorEvents {
	public:
		virtual ~MutatorEvents() {}
		virtual bool IsEmpty() const = 0;
	};

	struct AnimationEvent {
		enum Type { STARTED, FINISHED, ABORTED, TAKEOVER };

		AnimationEvent(Type type,
			//ElementId element_id,
			int group_id,
			int target_property,
			base::TimeTicks monotonic_time);

		AnimationEvent(const AnimationEvent& other);
		AnimationEvent& operator=(const AnimationEvent& other);

		~AnimationEvent();

		Type type;
		//ElementId element_id;
		int group_id;
		int target_property;
		base::TimeTicks monotonic_time;
		bool is_impl_only;
		float opacity;
// 		gfx::Transform transform;
// 		FilterOperations filters;

		// For continuing a scroll offset animation on the main thread.
		base::TimeTicks animation_start_time;
		std::unique_ptr<AnimationCurve> curve;
	};

	class AnimationEvents : public MutatorEvents {
	public:
		AnimationEvents();

		// MutatorEvents implementation.
		~AnimationEvents() override;
		bool IsEmpty() const override;

		std::vector<AnimationEvent> events_;
	};
}