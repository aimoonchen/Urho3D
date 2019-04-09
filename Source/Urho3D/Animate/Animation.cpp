#include "Animation.h"
#include "AnimationDelegate.h"
#include "AnimationEvent.h"
#include "KeyframeEffect.h"
#include "AnimationCurve.h"

namespace animation
{
	std::unique_ptr<Animation> Animation::Create(int id) {
		return std::make_unique<Animation>(id);
	}

	Animation::Animation(int id)
		: animation_host_(),
		animation_timeline_(),
		animation_delegate_(),
		id_(id),
		ticking_keyframe_effects_count(0) {
		//DCHECK(id_);
	}

	Animation::~Animation() {
		//DCHECK(!animation_timeline_);
	}

	std::unique_ptr<Animation> Animation::CreateImplInstance() const {
		return Animation::Create(id());
	}

	void Animation::PushPropertiesTo(Animation* animation_impl) {
		// In general when pushing proerties to impl thread we first push attached
		// properties to impl followed by removing the detached ones. However, we
		// never remove individual keyframe effect from an animation so there is no
		// need to remove the detached ones.
		PushAttachedKeyframeEffectsToImplThread(animation_impl);
		PushPropertiesToImplThread(animation_impl);
	}

	void Animation::Tick(base::TimeTicks monotonic_time) {
		//DCHECK(!monotonic_time.is_null());
		for (auto& keyframe_effect : keyframe_effects_)
			keyframe_effect->Tick(monotonic_time);
	}

	void Animation::UpdateState(bool start_ready_animations,
		AnimationEvents* events) {
		for (auto& keyframe_effect : keyframe_effects_) {
			keyframe_effect->UpdateState(start_ready_animations, events);
			keyframe_effect->UpdateTickingState(UpdateTickingType::NORMAL);
		}
	}

	void Animation::AddToTicking() {
		++ticking_keyframe_effects_count;
		if (ticking_keyframe_effects_count > 1)
			return;
		//DCHECK(animation_host_);
		//animation_host_->AddToTicking(this);
	}

	void Animation::KeyframeModelRemovedFromTicking() {
		//DCHECK_GE(ticking_keyframe_effects_count, 0);
		if (!ticking_keyframe_effects_count)
			return;
		--ticking_keyframe_effects_count;
		//DCHECK(animation_host_);
		//DCHECK_GE(ticking_keyframe_effects_count, 0);
		if (ticking_keyframe_effects_count)
			return;
		//animation_host_->RemoveFromTicking(this);
	}

	void Animation::NotifyKeyframeModelStarted(const AnimationEvent& event) {
		if (animation_delegate_) {
			animation_delegate_->NotifyAnimationStarted(
				event.monotonic_time, event.target_property, event.group_id);
		}
	}

	void Animation::NotifyKeyframeModelFinished(const AnimationEvent& event) {
		if (animation_delegate_) {
			animation_delegate_->NotifyAnimationFinished(
				event.monotonic_time, event.target_property, event.group_id);
		}
	}

	void Animation::NotifyKeyframeModelAborted(const AnimationEvent& event) {
		if (animation_delegate_) {
			animation_delegate_->NotifyAnimationAborted(
				event.monotonic_time, event.target_property, event.group_id);
		}
	}

	void Animation::NotifyKeyframeModelTakeover(const AnimationEvent& event) {
		//DCHECK(event.target_property == TargetProperty::SCROLL_OFFSET);

		if (animation_delegate_) {
			//DCHECK(event.curve);
			std::unique_ptr<AnimationCurve> animation_curve = event.curve->Clone();
			animation_delegate_->NotifyAnimationTakeover(
				event.monotonic_time, event.target_property, event.animation_start_time,
				std::move(animation_curve));
		}
	}

// 	void Animation::RegisterKeyframeEffect(ElementId element_id,
// 		KeyframeEffectId keyframe_effect_id) {
// 		//DCHECK(animation_host_);
// 		KeyframeEffect* keyframe_effect = GetKeyframeEffectById(keyframe_effect_id);
// 		//DCHECK(!keyframe_effect->has_bound_element_animations());
// 
// 		if (!keyframe_effect->has_attached_element())
// 			return;
// 		animation_host_->RegisterKeyframeEffectForElement(element_id,
// 			keyframe_effect);
// 	}
// 
// 	void Animation::UnregisterKeyframeEffect(ElementId element_id,
// 		KeyframeEffectId keyframe_effect_id) {
// 		//DCHECK(animation_host_);
// 		KeyframeEffect* keyframe_effect = GetKeyframeEffectById(keyframe_effect_id);
// 		//DCHECK(keyframe_effect);
// 		if (keyframe_effect->has_attached_element() &&
// 			keyframe_effect->has_bound_element_animations()) {
// 			animation_host_->UnregisterKeyframeEffectForElement(element_id,
// 				keyframe_effect);
// 		}
// 	}
	void Animation::RegisterKeyframeEffects() {
// 		for (auto& element_id_keyframe_effect_id :
// 			element_to_keyframe_effect_id_map_) {
// 			const ElementId element_id = element_id_keyframe_effect_id.first;
// 			const std::unordered_set<KeyframeEffectId>& keyframe_effect_ids =
// 				element_id_keyframe_effect_id.second;
// 			for (auto& keyframe_effect_id : keyframe_effect_ids)
// 				RegisterKeyframeEffect(element_id, keyframe_effect_id);
// 		}
	}

	void Animation::UnregisterKeyframeEffects() {
// 		for (auto& element_id_keyframe_effect_id :
// 			element_to_keyframe_effect_id_map_) {
// 			const ElementId element_id = element_id_keyframe_effect_id.first;
// 			const std::unordered_set<KeyframeEffectId>& keyframe_effect_ids =
// 				element_id_keyframe_effect_id.second;
// 			for (auto& keyframe_effect_id : keyframe_effect_ids)
// 				UnregisterKeyframeEffect(element_id, keyframe_effect_id);
// 		}
// 		animation_host_->RemoveFromTicking(this);
	}

	void Animation::PushAttachedKeyframeEffectsToImplThread(
		Animation* animation_impl) const {
		for (auto& keyframe_effect : keyframe_effects_) {
			KeyframeEffect* keyframe_effect_impl =
				animation_impl->GetKeyframeEffectById(keyframe_effect->id());
			if (keyframe_effect_impl)
				continue;

			std::unique_ptr<KeyframeEffect> to_add =
				keyframe_effect->CreateImplInstance();
			animation_impl->AddKeyframeEffect(std::move(to_add));
		}
	}

	void Animation::PushPropertiesToImplThread(Animation* animation_impl) {
		for (auto& keyframe_effect : keyframe_effects_) {
			if (KeyframeEffect* keyframe_effect_impl =
				animation_impl->GetKeyframeEffectById(keyframe_effect->id())) {
				keyframe_effect->PushPropertiesTo(keyframe_effect_impl);
			}
		}
	}

	bool Animation::IsWorkletAnimation() const {
		return false;
	}

	void Animation::AddKeyframeEffect(
		std::unique_ptr<KeyframeEffect> keyframe_effect) {
		keyframe_effect->SetAnimation(this);
		keyframe_effects_.push_back(std::move(keyframe_effect));

		//SetNeedsPushProperties();
	}

	KeyframeEffect* Animation::GetKeyframeEffectById(
		KeyframeEffectId keyframe_effect_id) const {
		// May return nullptr when syncing keyframe_effects_ to impl.
		return keyframe_effects_.size() > keyframe_effect_id
			? keyframe_effects_[keyframe_effect_id].get()
			: nullptr;
	}

	void Animation::SetNeedsCommit() {
		//DCHECK(animation_host_);
		//animation_host_->SetNeedsCommit();
	}

	void Animation::SetNeedsPushProperties() {
		if (!animation_timeline_)
			return;
		//animation_timeline_->SetNeedsPushProperties();
	}

	void Animation::ActivateKeyframeEffects() {
		for (auto& keyframe_effect : keyframe_effects_) {
			keyframe_effect->ActivateKeyframeEffects();
			keyframe_effect->UpdateTickingState(UpdateTickingType::NORMAL);
		}
	}
}