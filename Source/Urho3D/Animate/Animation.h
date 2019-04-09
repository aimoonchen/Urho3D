#pragma once
#include "AnimationBase.h"
#include <vector>

namespace animation
{
	class AnimationDelegate;
	class AnimationEvents;
	class AnimationHost;
	class AnimationTimeline;
	class KeyframeEffect;
	struct AnimationEvent;

	class Animation
	{
	public:
		static std::unique_ptr<Animation> Create(int id);
		virtual std::unique_ptr<Animation> CreateImplInstance() const;

		int id() const { return id_; }
		typedef size_t KeyframeEffectId;

		virtual void PushPropertiesTo(Animation* animation_impl);
		
		void UpdateState(bool start_ready_keyframe_models, AnimationEvents* events);
		virtual void Tick(base::TimeTicks monotonic_time);

		void AddToTicking();
		void KeyframeModelRemovedFromTicking();

		void NotifyKeyframeModelStarted(const AnimationEvent& event);
		void NotifyKeyframeModelFinished(const AnimationEvent& event);
		void NotifyKeyframeModelAborted(const AnimationEvent& event);
		void NotifyKeyframeModelTakeover(const AnimationEvent& event);

		void SetNeedsPushProperties();
		void ActivateKeyframeEffects();

		void SetNeedsCommit();

		virtual bool IsWorkletAnimation() const;
		void AddKeyframeEffect(std::unique_ptr<KeyframeEffect>);

		KeyframeEffect* GetKeyframeEffectById(KeyframeEffectId keyframe_effect_id) const;
		KeyframeEffectId NextKeyframeEffectId() { return keyframe_effects_.size(); }
		explicit Animation(int id);
		virtual ~Animation();
	private:
		//void RegisterKeyframeEffect(ElementId element_id, KeyframeEffectId keyframe_effect_id);
		//void UnregisterKeyframeEffect(ElementId element_id,	KeyframeEffectId keyframe_effect_id);
		void RegisterKeyframeEffects();
		void UnregisterKeyframeEffects();

		void PushAttachedKeyframeEffectsToImplThread(Animation* animation_impl) const;
		void PushPropertiesToImplThread(Animation* animation_impl);

	protected:
// 		explicit Animation(int id);
// 		virtual ~Animation();

		AnimationHost* animation_host_;
		AnimationTimeline* animation_timeline_;
		AnimationDelegate* animation_delegate_;

		int id_;

// 		using ElementToKeyframeEffectIdMap =
// 			std::unordered_map<ElementId,
// 			std::unordered_set<KeyframeEffectId>,
// 			ElementIdHash>;
		using KeyframeEffects = std::vector<std::unique_ptr<KeyframeEffect>>;

		// It is possible for a keyframe_effect to be in keyframe_effects_ but not in
		// element_to_keyframe_effect_id_map_ but the reverse is not possible.
//		ElementToKeyframeEffectIdMap element_to_keyframe_effect_id_map_;
		KeyframeEffects keyframe_effects_;

		int ticking_keyframe_effects_count;

		DISALLOW_COPY_AND_ASSIGN(Animation);
	};
}