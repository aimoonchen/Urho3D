#pragma once
#include "AnimationBase.h"
#include "AnimationEvent.h"

namespace animation
{
	typedef size_t KeyframeEffectId;
	enum class UpdateTickingType { NORMAL, FORCE };
	struct AnimationEvent;
	class AnimationEvents;
	class KeyframeModel;
	class Animation;

	class KeyframeEffect
	{
	public:
		explicit KeyframeEffect(KeyframeEffectId id);
		virtual ~KeyframeEffect();

		static std::unique_ptr<KeyframeEffect> Create(KeyframeEffectId id);
		std::unique_ptr<KeyframeEffect> CreateImplInstance() const;

		bool has_bound_element_animations() const { return true; /*return !!element_animations_;*/ }

// 		bool has_attached_element() const { return !!element_id_; }
// 
// 		ElementId element_id() const { return element_id_; }

		// Returns true if there are any KeyframeModels at all to process.
		bool has_any_keyframe_model() const { return !keyframe_models_.empty(); }

		bool needs_push_properties() const { return needs_push_properties_; }
		void SetNeedsPushProperties();

		virtual void Tick(base::TimeTicks monotonic_time);
		static void TickKeyframeModel(base::TimeTicks monotonic_time,
			KeyframeModel* keyframe_model,
			AnimationTarget* target);
		void RemoveFromTicking();
		bool is_ticking() const { return is_ticking_; }

		void UpdateState(bool start_ready_keyframe_models, AnimationEvents* events);
		void UpdateTickingState(UpdateTickingType type);

		void Pause(base::TimeDelta pause_offset);

		void AddKeyframeModel(std::unique_ptr<KeyframeModel> keyframe_model);
		void PauseKeyframeModel(int keyframe_model_id, double time_offset);
		void RemoveKeyframeModel(int keyframe_model_id);
		void AbortKeyframeModel(int keyframe_model_id);
// 		void AbortKeyframeModelsWithProperty(TargetProperty::Type target_property,
// 			bool needs_completion);

		void ActivateKeyframeEffects();

		void KeyframeModelAdded();

		// The following methods should be called to notify the KeyframeEffect that
		// an animation event has been received for the same target (ElementId) as
		// this keyframe_effect. If the event matches a KeyframeModel owned by this
		// KeyframeEffect the call will return true, else it will return false.
		bool NotifyKeyframeModelStarted(const AnimationEvent& event);
		bool NotifyKeyframeModelFinished(const AnimationEvent& event);
		void NotifyKeyframeModelTakeover(const AnimationEvent& event);
		bool NotifyKeyframeModelAborted(const AnimationEvent& event);

		KeyframeModel* GetKeyframeModel(TargetProperty::Type target_property) const;
		KeyframeModel* GetKeyframeModelById(int keyframe_model_id) const;

		void MarkAbortedKeyframeModelsForDeletion(
			KeyframeEffect* element_keyframe_effect_impl);
		void PurgeKeyframeModelsMarkedForDeletion(bool impl_only);
		void PushNewKeyframeModelsToImplThread(
			KeyframeEffect* element_keyframe_effect_impl) const;
		void RemoveKeyframeModelsCompletedOnMainThread(
			KeyframeEffect* element_keyframe_effect_impl) const;

		void PushPropertiesTo(KeyframeEffect* keyframe_effect_impl);

		void SetAnimation(Animation* animation);

		std::string KeyframeModelsToString() const;
		KeyframeEffectId id() const { return id_; }
	private:
	private:
		void StartKeyframeModels(base::TimeTicks monotonic_time);
		void PromoteStartedKeyframeModels(AnimationEvents* events);

		void MarkKeyframeModelsForDeletion(base::TimeTicks, AnimationEvents* events);
		void MarkFinishedKeyframeModels(base::TimeTicks monotonic_time);

		void GenerateEvent(AnimationEvents* events,
			const KeyframeModel& keyframe_model,
			AnimationEvent::Type type,
			base::TimeTicks monotonic_time);
		void GenerateTakeoverEventForScrollAnimation(
			AnimationEvents* events,
			const KeyframeModel& keyframe_model,
			base::TimeTicks monotonic_time);
		std::vector<std::unique_ptr<KeyframeModel>> keyframe_models_;
		Animation* animation_;

		KeyframeEffectId id_;
		//ElementId element_id_;

		// element_animations_ is non-null if controller is attached to an element.
		//scoped_refptr<ElementAnimations> element_animations_;

		// Only try to start KeyframeModels when new keyframe models are added or
		// when the previous attempt at starting KeyframeModels failed to start all
		// KeyframeModels.
		bool needs_to_start_keyframe_models_{ false };

		bool scroll_offset_animation_was_interrupted_{ false };

		bool is_ticking_{ false };

		base::TimeTicks last_tick_time_;
		bool needs_push_properties_{ false };
	};
}