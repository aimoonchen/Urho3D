#include "KeyframeEffect.h"
#include <memory>
#include <vector>
#include <algorithm>
#include "KeyframeModel.h"
#include "AnimationCurve.h"
#include "Animation.h"
namespace {

	bool NeedsFinishedEvent(animation::KeyframeModel* keyframe_model) {
		// The controlling instance (i.e., impl instance), sends the finish event and
		// does not need to receive it.
		if (keyframe_model->is_controlling_instance())
			return false;

		return !keyframe_model->received_finished_event();
	}

	// Returns indices for keyframe_models that have matching group id.
	std::vector<size_t> FindAnimationsWithSameGroupId(
		const std::vector<std::unique_ptr<animation::KeyframeModel>>& keyframe_models,
		int group_id) {
		std::vector<size_t> group;
		for (size_t i = 0; i < keyframe_models.size(); ++i) {
			if (keyframe_models[i]->group() != group_id)
				continue;

			group.push_back(i);
		}
		return group;
	}

}  // namespace

namespace animation {
	KeyframeEffect::KeyframeEffect(KeyframeEffectId id)
		: animation_(),
		id_(id),
		//element_animations_(),
		needs_to_start_keyframe_models_(false),
		scroll_offset_animation_was_interrupted_(false),
		is_ticking_(false)
		//needs_push_properties_(false)
	{}

	KeyframeEffect::~KeyframeEffect() {
		//DCHECK(!has_bound_element_animations());
	}

	std::unique_ptr<KeyframeEffect> KeyframeEffect::Create(KeyframeEffectId id) {
		return std::make_unique<KeyframeEffect>(id);
	}

	std::unique_ptr<KeyframeEffect> KeyframeEffect::CreateImplInstance() const {
		return KeyframeEffect::Create(id());
	}

	void KeyframeEffect::SetNeedsPushProperties() {
		needs_push_properties_ = true;

		// TODO(smcgruer): We only need the below calls when needs_push_properties_
		// goes from false to true - see http://crbug.com/764405
		//DCHECK(element_animations());
		//element_animations()->SetNeedsPushProperties();

		animation_->SetNeedsPushProperties();
	}


	void KeyframeEffect::Tick(base::TimeTicks monotonic_time) {
		//DCHECK(has_bound_element_animations());
// 		if (!element_animations_->has_element_in_any_list())
// 			return;

		if (needs_to_start_keyframe_models_)
			StartKeyframeModels(monotonic_time);

		for (auto& keyframe_model : keyframe_models_) {
			TickKeyframeModel(monotonic_time, keyframe_model.get(),
				nullptr/*element_animations_.get()*/);
		}

		last_tick_time_ = monotonic_time;
		//element_animations_->UpdateClientAnimationState();
	}

	void KeyframeEffect::TickKeyframeModel(base::TimeTicks monotonic_time,
		KeyframeModel* keyframe_model,
		AnimationTarget* target) {
		if ((keyframe_model->run_state() != KeyframeModel::STARTING &&
			keyframe_model->run_state() != KeyframeModel::RUNNING &&
			keyframe_model->run_state() != KeyframeModel::PAUSED) ||
			!keyframe_model->InEffect(monotonic_time)) {
			return;
		}

		AnimationCurve* curve = keyframe_model->curve();
		base::TimeDelta trimmed =
			keyframe_model->TrimTimeToCurrentIteration(monotonic_time);

		switch (curve->Type()) {
// 		case AnimationCurve::TRANSFORM:
// 			target->NotifyClientTransformOperationsAnimated(
// 				curve->GetValue(trimmed),
// 				keyframe_model->target_property_id(), keyframe_model);
// 			break;
		case AnimationCurve::FLOAT:
			target->NotifyClientFloatAnimated(
				curve->GetValue(trimmed),
				keyframe_model->target_property_id(), keyframe_model);
			break;
// 		case AnimationCurve::FILTER:
// 			target->NotifyClientFilterAnimated(
// 				curve->GetValue(trimmed),
// 				keyframe_model->target_property_id(), keyframe_model);
// 			break;
// 		case AnimationCurve::COLOR:
// 			target->NotifyClientColorAnimated(
// 				curve->GetValue(trimmed),
// 				keyframe_model->target_property_id(), keyframe_model);
// 			break;
// 		case AnimationCurve::SCROLL_OFFSET:
// 			target->NotifyClientScrollOffsetAnimated(
// 				curve->GetValue(trimmed),
// 				keyframe_model->target_property_id(), keyframe_model);
// 			break;
// 		case AnimationCurve::SIZE:
// 			target->NotifyClientSizeAnimated(
// 				curve->GetValue(trimmed),
// 				keyframe_model->target_property_id(), keyframe_model);
// 			break;
		}
	}

	void KeyframeEffect::RemoveFromTicking() {
		is_ticking_ = false;
		// Resetting last_tick_time_ here ensures that calling ::UpdateState
		// before ::Animate doesn't start a keyframe model.
		last_tick_time_ = base::TimeTicks();
		animation_->KeyframeModelRemovedFromTicking();
	}

	void KeyframeEffect::UpdateState(bool start_ready_keyframe_models,
		AnimationEvents* events) {
		DCHECK(has_bound_element_animations());
		if (!element_animations_->has_element_in_active_list())
			return;

		// Animate hasn't been called, this happens if an element has been added
		// between the Commit and Draw phases.
		if (last_tick_time_ == base::TimeTicks())
			return;

		if (start_ready_keyframe_models)
			PromoteStartedKeyframeModels(events);

		MarkFinishedKeyframeModels(last_tick_time_);
		MarkKeyframeModelsForDeletion(last_tick_time_, events);
		PurgeKeyframeModelsMarkedForDeletion(/* impl_only */ true);

		if (start_ready_keyframe_models) {
			if (needs_to_start_keyframe_models_) {
				StartKeyframeModels(last_tick_time_);
				PromoteStartedKeyframeModels(events);
			}
		}
	}

	void KeyframeEffect::UpdateTickingState(UpdateTickingType type) {
		bool force = type == UpdateTickingType::FORCE;
		if (animation_->has_animation_host()) {
			bool was_ticking = is_ticking_;
			is_ticking_ = HasNonDeletedKeyframeModel();

			bool has_element_in_any_list =
				element_animations_->has_element_in_any_list();

			if (is_ticking_ && ((!was_ticking && has_element_in_any_list) || force)) {
				animation_->AddToTicking();
			}
			else if (!is_ticking_ && (was_ticking || force)) {
				RemoveFromTicking();
			}
		}
	}

	void KeyframeEffect::Pause(base::TimeDelta pause_offset) {
		for (auto& keyframe_model : keyframe_models_)
			keyframe_model->Pause(pause_offset);

		if (has_bound_element_animations()) {
			animation_->SetNeedsCommit();
			SetNeedsPushProperties();
		}
	}

	void KeyframeEffect::AddKeyframeModel(std::unique_ptr<KeyframeModel> keyframe_model)
	{
// 		DCHECK(keyframe_model->target_property_id() !=
// 			TargetProperty::SCROLL_OFFSET ||
// 			(animation_->animation_host()->SupportsScrollAnimations()));
// 		DCHECK(!keyframe_model->is_impl_only() ||
// 			keyframe_model->target_property_id() == TargetProperty::SCROLL_OFFSET);
// 		// This is to make sure that keyframe models in the same group, i.e., start
// 		// together, don't animate the same property.
// 		DCHECK(std::none_of(
// 			keyframe_models_.begin(), keyframe_models_.end(),
// 			[&](const auto& existing_keyframe_model) {
// 			return keyframe_model->target_property_id() ==
// 				existing_keyframe_model->target_property_id() &&
// 				keyframe_model->group() == existing_keyframe_model->group();
// 		}));

		keyframe_models_.push_back(std::move(keyframe_model));

		if (has_bound_element_animations()) {
			KeyframeModelAdded();
			SetNeedsPushProperties();
		}
	}

	void KeyframeEffect::PauseKeyframeModel(int keyframe_model_id,
		double time_offset) {
		const base::TimeDelta pause_offset =
			base::TimeDelta::FromSecondsD(time_offset);
		for (auto& keyframe_model : keyframe_models_) {
			if (keyframe_model->id() == keyframe_model_id) {
				keyframe_model->Pause(pause_offset);
			}
		}

		if (has_bound_element_animations()) {
			animation_->SetNeedsCommit();
			SetNeedsPushProperties();
		}
	}

	void KeyframeEffect::RemoveKeyframeModel(int keyframe_model_id) {
		bool keyframe_model_removed = false;

		// Since we want to use the KeyframeModels that we're going to remove, we
		// need to use a stable_parition here instead of remove_if. Remove_if leaves
		// the removed items in an unspecified state.
		auto keyframe_models_to_remove = std::stable_partition(
			keyframe_models_.begin(), keyframe_models_.end(),
			[keyframe_model_id](
				const std::unique_ptr<KeyframeModel>& keyframe_model) {
			return keyframe_model->id() != keyframe_model_id;
		});
		for (auto it = keyframe_models_to_remove; it != keyframe_models_.end();
			++it) {
			if ((*it)->target_property_id() == TargetProperty::SCROLL_OFFSET) {
				if (has_bound_element_animations())
					scroll_offset_animation_was_interrupted_ = true;
			}
			else if (!(*it)->is_finished()) {
				keyframe_model_removed = true;
			}
		}

		keyframe_models_.erase(keyframe_models_to_remove, keyframe_models_.end());

		if (has_bound_element_animations()) {
			UpdateTickingState(UpdateTickingType::NORMAL);
			if (keyframe_model_removed)
				element_animations_->UpdateClientAnimationState();
			animation_->SetNeedsCommit();
			SetNeedsPushProperties();
		}
	}

	void KeyframeEffect::AbortKeyframeModel(int keyframe_model_id) {
		if (KeyframeModel* keyframe_model = GetKeyframeModelById(keyframe_model_id)) {
			if (!keyframe_model->is_finished()) {
				keyframe_model->SetRunState(KeyframeModel::ABORTED, last_tick_time_);
				if (has_bound_element_animations())
					element_animations_->UpdateClientAnimationState();
			}
		}

		if (has_bound_element_animations()) {
			animation_->SetNeedsCommit();
			SetNeedsPushProperties();
		}
	}

	void KeyframeEffect::ActivateKeyframeEffects() {
		//DCHECK(has_bound_element_animations());

		bool keyframe_model_activated = false;
		for (auto& keyframe_model : keyframe_models_) {
			if (keyframe_model->affects_active_elements() !=
				keyframe_model->affects_pending_elements()) {
				keyframe_model_activated = true;
			}
			keyframe_model->set_affects_active_elements(
				keyframe_model->affects_pending_elements());
		}

		if (keyframe_model_activated)
			element_animations_->UpdateClientAnimationState();

		scroll_offset_animation_was_interrupted_ = false;
	}

	void KeyframeEffect::KeyframeModelAdded() {
		//DCHECK(has_bound_element_animations());

		animation_->SetNeedsCommit();
		needs_to_start_keyframe_models_ = true;

		UpdateTickingState(UpdateTickingType::NORMAL);
		element_animations_->UpdateClientAnimationState();
	}

	bool KeyframeEffect::NotifyKeyframeModelStarted(const AnimationEvent& event) {
		//DCHECK(!event.is_impl_only);
		for (auto& keyframe_model : keyframe_models_) {
			if (keyframe_model->group() == event.group_id &&
				keyframe_model->target_property_id() == event.target_property &&
				keyframe_model->needs_synchronized_start_time()) {
				keyframe_model->set_needs_synchronized_start_time(false);
				if (!keyframe_model->has_set_start_time())
					keyframe_model->set_start_time(event.monotonic_time);
				animation_->NotifyKeyframeModelStarted(event);
				return true;
			}
		}
		return false;
	}

	bool KeyframeEffect::NotifyKeyframeModelFinished(const AnimationEvent& event) {
		//DCHECK(!event.is_impl_only);
		for (auto& keyframe_model : keyframe_models_) {
			if (keyframe_model->group() == event.group_id &&
				keyframe_model->target_property_id() == event.target_property) {
				keyframe_model->set_received_finished_event(true);
				animation_->NotifyKeyframeModelFinished(event);
				return true;
			}
		}

		// This is for the case when a keyframe_model is already removed on main
		// thread, but the impl version of it sent a finished event and is now waiting
		// for deletion. We would need to delete that keyframe_model during push
		// properties.
		SetNeedsPushProperties();
		return false;
	}

	void KeyframeEffect::NotifyKeyframeModelTakeover(const AnimationEvent& event) {
		//DCHECK(!event.is_impl_only);

		// We need to purge KeyframeModels marked for deletion on CT.
		SetNeedsPushProperties();

		animation_->NotifyKeyframeModelTakeover(event);
	}

	bool KeyframeEffect::NotifyKeyframeModelAborted(const AnimationEvent& event) {
		//DCHECK(!event.is_impl_only);
		for (auto& keyframe_model : keyframe_models_) {
			if (keyframe_model->group() == event.group_id &&
				keyframe_model->target_property_id() == event.target_property) {
				keyframe_model->SetRunState(KeyframeModel::ABORTED, event.monotonic_time);
				keyframe_model->set_received_finished_event(true);
				animation_->NotifyKeyframeModelAborted(event);
				return true;
			}
		}
		return false;
	}

	void KeyframeEffect::PushPropertiesTo(KeyframeEffect* keyframe_effect_impl) {
		if (!needs_push_properties_)
			return;
		needs_push_properties_ = false;

		// Synchronize the keyframe_model target between main and impl side.
		if (element_id_ != keyframe_effect_impl->element_id_) {
			// We have to detach/attach via the Animation as it may need to inform
			// the host as well.
			if (keyframe_effect_impl->has_attached_element()) {
				keyframe_effect_impl->animation_->DetachElementForKeyframeEffect(
					keyframe_effect_impl->element_id_, keyframe_effect_impl->id_);
			}
			if (element_id_) {
				keyframe_effect_impl->animation_->AttachElementForKeyframeEffect(
					element_id_, id_);
			}
		}

		// If neither main nor impl have any KeyframeModels, there is nothing further
		// to synchronize.
		if (!has_any_keyframe_model() &&
			!keyframe_effect_impl->has_any_keyframe_model())
			return;

		// Synchronize the main-thread and impl-side keyframe model lists, removing
		// aborted KeyframeModels and pushing any new animations.
		MarkAbortedKeyframeModelsForDeletion(keyframe_effect_impl);
		PurgeKeyframeModelsMarkedForDeletion(/* impl_only */ false);
		RemoveKeyframeModelsCompletedOnMainThread(keyframe_effect_impl);
		PushNewKeyframeModelsToImplThread(keyframe_effect_impl);

		// Now that the keyframe model lists are synchronized, push the properties for
		// the individual KeyframeModels.
		for (const auto& keyframe_model : keyframe_models_) {
			KeyframeModel* current_impl =
				keyframe_effect_impl->GetKeyframeModelById(keyframe_model->id());
			if (current_impl)
				keyframe_model->PushPropertiesTo(current_impl);
		}
		keyframe_effect_impl->scroll_offset_animation_was_interrupted_ =
			scroll_offset_animation_was_interrupted_;
		scroll_offset_animation_was_interrupted_ = false;

		keyframe_effect_impl->UpdateTickingState(UpdateTickingType::NORMAL);
	}

	void KeyframeEffect::SetAnimation(Animation* animation) {
		animation_ = animation;
	}

	std::string KeyframeEffect::KeyframeModelsToString() const {
		std::string str;
		for (size_t i = 0; i < keyframe_models_.size(); i++) {
			if (i > 0)
				str.append(", ");
			str.append(keyframe_models_[i]->ToString());
		}
		return str;
	}

	void KeyframeEffect::StartKeyframeModels(base::TimeTicks monotonic_time) {
		//DCHECK(needs_to_start_keyframe_models_);
		needs_to_start_keyframe_models_ = false;

		// First collect running properties affecting each type of element.
		TargetProperties blocked_properties_for_active_elements;
		TargetProperties blocked_properties_for_pending_elements;
		std::vector<size_t> keyframe_models_waiting_for_target;

		keyframe_models_waiting_for_target.reserve(keyframe_models_.size());
		for (size_t i = 0; i < keyframe_models_.size(); ++i) {
			auto& keyframe_model = keyframe_models_[i];
			if (keyframe_model->run_state() == KeyframeModel::STARTING ||
				keyframe_model->run_state() == KeyframeModel::RUNNING) {
				int property = keyframe_model->target_property_id();
				if (keyframe_model->affects_active_elements()) {
					blocked_properties_for_active_elements[property] = true;
				}
				if (keyframe_model->affects_pending_elements()) {
					blocked_properties_for_pending_elements[property] = true;
				}
			}
			else if (keyframe_model->run_state() ==
				KeyframeModel::WAITING_FOR_TARGET_AVAILABILITY) {
				keyframe_models_waiting_for_target.push_back(i);
			}
		}

		for (size_t i = 0; i < keyframe_models_waiting_for_target.size(); ++i) {
			// Collect all properties for KeyframeModels with the same group id (they
			// should all also be in the list of KeyframeModels).
			size_t keyframe_model_index = keyframe_models_waiting_for_target[i];
			KeyframeModel* keyframe_model_waiting_for_target =
				keyframe_models_[keyframe_model_index].get();
			// Check for the run state again even though the keyframe_model was waiting
			// for target because it might have changed the run state while handling
			// previous keyframe_model in this loop (if they belong to same group).
			if (keyframe_model_waiting_for_target->run_state() ==
				KeyframeModel::WAITING_FOR_TARGET_AVAILABILITY) {
				TargetProperties enqueued_properties;
				bool affects_active_elements =
					keyframe_model_waiting_for_target->affects_active_elements();
				bool affects_pending_elements =
					keyframe_model_waiting_for_target->affects_pending_elements();
				enqueued_properties[keyframe_model_waiting_for_target
					->target_property_id()] = true;
				for (size_t j = keyframe_model_index + 1; j < keyframe_models_.size();
					++j) {
					if (keyframe_model_waiting_for_target->group() ==
						keyframe_models_[j]->group()) {
						enqueued_properties[keyframe_models_[j]->target_property_id()] = true;
						affects_active_elements |=
							keyframe_models_[j]->affects_active_elements();
						affects_pending_elements |=
							keyframe_models_[j]->affects_pending_elements();
					}
				}

				// Check to see if intersection of the list of properties affected by
				// the group and the list of currently blocked properties is null, taking
				// into account the type(s) of elements affected by the group. In any
				// case, the group's target properties need to be added to the lists of
				// blocked properties.
				bool null_intersection = true;
				for (int property = TargetProperty::FIRST_TARGET_PROPERTY;
					property <= TargetProperty::LAST_TARGET_PROPERTY; ++property) {
					if (enqueued_properties[property]) {
						if (affects_active_elements) {
							if (blocked_properties_for_active_elements[property])
								null_intersection = false;
							else
								blocked_properties_for_active_elements[property] = true;
						}
						if (affects_pending_elements) {
							if (blocked_properties_for_pending_elements[property])
								null_intersection = false;
							else
								blocked_properties_for_pending_elements[property] = true;
						}
					}
				}

				// If the intersection is null, then we are free to start the
				// KeyframeModels in the group.
				if (null_intersection) {
					keyframe_model_waiting_for_target->SetRunState(KeyframeModel::STARTING,
						monotonic_time);
					for (size_t j = keyframe_model_index + 1; j < keyframe_models_.size();
						++j) {
						if (keyframe_model_waiting_for_target->group() ==
							keyframe_models_[j]->group()) {
							keyframe_models_[j]->SetRunState(KeyframeModel::STARTING,
								monotonic_time);
						}
					}
				}
				else {
					needs_to_start_keyframe_models_ = true;
				}
			}
		}
	}

	void KeyframeEffect::PromoteStartedKeyframeModels(AnimationEvents* events) {
		for (auto& keyframe_model : keyframe_models_) {
			if (keyframe_model->run_state() == KeyframeModel::STARTING &&
				keyframe_model->affects_active_elements()) {
				keyframe_model->SetRunState(KeyframeModel::RUNNING, last_tick_time_);
				if (!keyframe_model->has_set_start_time() &&
					!keyframe_model->needs_synchronized_start_time())
					keyframe_model->set_start_time(last_tick_time_);

				base::TimeTicks start_time;
				if (keyframe_model->has_set_start_time())
					start_time = keyframe_model->start_time();
				else
					start_time = last_tick_time_;

				GenerateEvent(events, *keyframe_model, AnimationEvent::STARTED,
					start_time);
			}
		}
	}

	void KeyframeEffect::MarkKeyframeModelsForDeletion(
		base::TimeTicks monotonic_time,
		AnimationEvents* events) {
		bool marked_keyframe_model_for_deletion = false;
		auto MarkForDeletion = [&](KeyframeModel* keyframe_model) {
			keyframe_model->SetRunState(KeyframeModel::WAITING_FOR_DELETION,
				monotonic_time);
			marked_keyframe_model_for_deletion = true;
		};

		// Non-aborted KeyframeModels are marked for deletion after a corresponding
		// AnimationEvent::FINISHED event is sent or received. This means that if
		// we don't have an events vector, we must ensure that non-aborted
		// KeyframeModels have received a finished event before marking them for
		// deletion.
		for (size_t i = 0; i < keyframe_models_.size(); i++) {
			KeyframeModel* keyframe_model = keyframe_models_[i].get();
			if (keyframe_model->run_state() == KeyframeModel::ABORTED) {
				GenerateEvent(events, *keyframe_model, AnimationEvent::ABORTED,
					monotonic_time);
				// If this is the controlling instance or it has already received finish
				// event, keyframe model can be marked for deletion.
				if (!NeedsFinishedEvent(keyframe_model))
					MarkForDeletion(keyframe_model);
				continue;
			}

			// If this is an aborted controlling instance that need completion on the
			// main thread, generate takeover event.
			if (keyframe_model->is_controlling_instance() &&
				keyframe_model->run_state() ==
				KeyframeModel::ABORTED_BUT_NEEDS_COMPLETION) {
				GenerateTakeoverEventForScrollAnimation(events, *keyframe_model,
					monotonic_time);
				// Remove the keyframe model from the impl thread.
				MarkForDeletion(keyframe_model);
				continue;
			}

			if (keyframe_model->run_state() != KeyframeModel::FINISHED)
				continue;

			// Since deleting an animation on the main thread leads to its deletion
			// on the impl thread, we only mark a FINISHED main thread animation for
			// deletion once it has received a FINISHED event from the impl thread.
			if (NeedsFinishedEvent(keyframe_model))
				continue;

			// If a keyframe model is finished, and not already marked for deletion,
			// find out if all other keyframe models in the same group are also
			// finished.
			std::vector<size_t> keyframe_models_in_same_group =
				FindAnimationsWithSameGroupId(keyframe_models_,
					keyframe_model->group());

			bool a_keyframe_model_in_same_group_is_not_finished = std::any_of(
				keyframe_models_in_same_group.cbegin(),
				keyframe_models_in_same_group.cend(), [&](size_t index) {
				KeyframeModel* keyframe_model = keyframe_models_[index].get();
				return !keyframe_model->is_finished() ||
					(keyframe_model->run_state() == KeyframeModel::FINISHED &&
						NeedsFinishedEvent(keyframe_model));
			});

			if (a_keyframe_model_in_same_group_is_not_finished)
				continue;

			// Now remove all the keyframe models which belong to the same group and are
			// not yet aborted. These will be set to WAITING_FOR_DELETION which also
			// ensures we don't try to delete them again.
			for (size_t j = 0; j < keyframe_models_in_same_group.size(); ++j) {
				KeyframeModel* keyframe_model =
					keyframe_models_[keyframe_models_in_same_group[j]].get();

				// Skip any keyframe model in this group which is already processed.
				if (keyframe_model->run_state() == KeyframeModel::WAITING_FOR_DELETION ||
					keyframe_model->run_state() == KeyframeModel::ABORTED)
					continue;

				GenerateEvent(events, *keyframe_model, AnimationEvent::FINISHED,
					monotonic_time);
				MarkForDeletion(keyframe_model);
			}
		}

		// We need to purge KeyframeModels marked for deletion, which happens in
		// PushPropertiesTo().
		if (marked_keyframe_model_for_deletion)
			SetNeedsPushProperties();
	}

	void KeyframeEffect::MarkFinishedKeyframeModels(
		base::TimeTicks monotonic_time) {
		//DCHECK(has_bound_element_animations());

		bool keyframe_model_finished = false;
		for (auto& keyframe_model : keyframe_models_) {
			if (!keyframe_model->is_finished() &&
				keyframe_model->IsFinishedAt(monotonic_time)) {
				keyframe_model->SetRunState(KeyframeModel::FINISHED, monotonic_time);
				keyframe_model_finished = true;
				SetNeedsPushProperties();
			}
			if (!keyframe_model->affects_active_elements() &&
				!keyframe_model->affects_pending_elements()) {
				switch (keyframe_model->run_state()) {
				case KeyframeModel::WAITING_FOR_TARGET_AVAILABILITY:
				case KeyframeModel::STARTING:
				case KeyframeModel::RUNNING:
				case KeyframeModel::PAUSED:
					keyframe_model->SetRunState(KeyframeModel::FINISHED, monotonic_time);
					keyframe_model_finished = true;
					break;
				default:
					break;
				}
			}
		}
		if (keyframe_model_finished)
			element_animations_->UpdateClientAnimationState();
	}

	void KeyframeEffect::GenerateEvent(AnimationEvents* events,
		const KeyframeModel& keyframe_model,
		AnimationEvent::Type type,
		base::TimeTicks monotonic_time) {
		if (!events)
			return;

		AnimationEvent event(type, /*element_id_, */keyframe_model.group(),
			keyframe_model.target_property_id(), monotonic_time);
		event.is_impl_only = keyframe_model.is_impl_only();
		if (!event.is_impl_only) {
			events->events_.push_back(event);
			return;
		}
		// For impl only animations notify delegate directly, do not record the event.
		switch (type) {
		case AnimationEvent::FINISHED:
			animation_->NotifyKeyframeModelFinished(event);
			break;
		case AnimationEvent::STARTED:
			animation_->NotifyKeyframeModelStarted(event);
			break;
		case AnimationEvent::ABORTED:
			animation_->NotifyKeyframeModelAborted(event);
			break;
		case AnimationEvent::TAKEOVER:
			// We never expect to receive a TAKEOVER notification on impl only
			// animations.
			//NOTREACHED();
			break;
		}
	}

	void KeyframeEffect::GenerateTakeoverEventForScrollAnimation(
		AnimationEvents* events,
		const KeyframeModel& keyframe_model,
		base::TimeTicks monotonic_time) {
		//DCHECK_EQ(keyframe_model.target_property_id(), TargetProperty::SCROLL_OFFSET);
		if (!events)
			return;

		AnimationEvent takeover_event(
			AnimationEvent::TAKEOVER, /*element_id_, */keyframe_model.group(),
			keyframe_model.target_property_id(), monotonic_time);
		takeover_event.animation_start_time = keyframe_model.start_time();
		const ScrollOffsetAnimationCurve* scroll_offset_animation_curve =
			keyframe_model.curve()->ToScrollOffsetAnimationCurve();
		takeover_event.curve = scroll_offset_animation_curve->Clone();
		// Notify the compositor that the animation is finished.
		animation_->NotifyKeyframeModelFinished(takeover_event);
		// Notify main thread.
		events->events_.push_back(takeover_event);
	}
}