#pragma once

#include "AnimationBase.h"

namespace animation
{
	class AnimationCurve;
	class AnimationDelegate {
	public:
		// TODO(yigu): The Notify* methods will be called multiple times per
		// animation (once for effect/property pairing).
		// Ideally, we would only notify start once (e.g., wait on all effects to
		// start before notifying delegate) this way effect becomes an internal
		// details of the animation. Perhaps we can do that at some point maybe as
		// part of https://bugs.chromium.org/p/chromium/issues/detail?id=810003
		virtual void NotifyAnimationStarted(base::TimeTicks monotonic_time,
			int target_property,
			int group) = 0;
		virtual void NotifyAnimationFinished(base::TimeTicks monotonic_time,
			int target_property,
			int group) = 0;

		virtual void NotifyAnimationAborted(base::TimeTicks monotonic_time,
			int target_property,
			int group) = 0;

		virtual void NotifyAnimationTakeover(
			base::TimeTicks monotonic_time,
			int target_property,
			base::TimeTicks animation_start_time,
			std::unique_ptr<AnimationCurve> curve) = 0;

	protected:
		virtual ~AnimationDelegate() {}
	};
}