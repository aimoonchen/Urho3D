#include "KeyframeModel.h"
#include "AnimationCurve.h"
#include <algorithm>
namespace {
	// This should match the RunState enum.
	static const char* const s_runStateNames[] = { "WAITING_FOR_TARGET_AVAILABILITY",
												  "WAITING_FOR_DELETION",
												  "STARTING",
												  "RUNNING",
												  "PAUSED",
												  "FINISHED",
												  "ABORTED",
												  "ABORTED_BUT_NEEDS_COMPLETION" };

	static_assert(static_cast<int>(animation::KeyframeModel::LAST_RUN_STATE) + 1 ==
		animation::base::size(s_runStateNames),
		"RunStateEnumSize should equal the number of elements in "
		"s_runStateNames");

	static const char* const s_curveTypeNames[] = {
		"COLOR", "FLOAT", "TRANSFORM", "FILTER", "SCROLL_OFFSET", "SIZE" };

	static_assert(static_cast<int>(animation::AnimationCurve::LAST_CURVE_TYPE) + 1 ==
		animation::base::size(s_curveTypeNames),
		"CurveType enum should equal the number of elements in "
		"s_runStateNames");

}  // namespace

namespace animation
{
	std::string KeyframeModel::ToString(RunState state)
	{
		return s_runStateNames[state];
	}

	std::unique_ptr<KeyframeModel> KeyframeModel::Create(
		std::unique_ptr<AnimationCurve> curve,
		int keyframe_model_id,
		int group_id,
		int target_property_id)
	{
		return base::WrapUnique(new KeyframeModel(std::move(curve), keyframe_model_id,
			group_id, target_property_id));
	}

	std::unique_ptr<KeyframeModel> KeyframeModel::CreateImplInstance(RunState initial_run_state) const
	{
		// Should never clone a model that is the controlling instance as it ends up
		// creating multiple controlling instances.
		//DCHECK(!is_controlling_instance_);
		std::unique_ptr<KeyframeModel> to_return(new KeyframeModel(curve_->Clone(), id_, group_, target_property_id_));
		//to_return->element_id_ = element_id_;
		to_return->run_state_ = initial_run_state;
		to_return->iterations_ = iterations_;
		to_return->iteration_start_ = iteration_start_;
		to_return->start_time_ = start_time_;
		to_return->pause_time_ = pause_time_;
		to_return->total_paused_duration_ = total_paused_duration_;
		to_return->time_offset_ = time_offset_;
		to_return->direction_ = direction_;
		to_return->playback_rate_ = playback_rate_;
		to_return->fill_mode_ = fill_mode_;
		//DCHECK(!to_return->is_controlling_instance_);
		//to_return->is_controlling_instance_ = true;
		return to_return;
	}

	KeyframeModel::KeyframeModel(std::unique_ptr<AnimationCurve> curve,
		int keyframe_model_id,
		int group_id,
		int target_property_id)
		: curve_(std::move(curve)),
		id_(keyframe_model_id),
		group_(group_id),
		target_property_id_(target_property_id),
		run_state_(WAITING_FOR_TARGET_AVAILABILITY),
		iterations_(1),
		iteration_start_(0),
		direction_(Direction::NORMAL),
		playback_rate_(1),
		fill_mode_(FillMode::BOTH),
		needs_synchronized_start_time_(false),
		received_finished_event_(false),
		is_controlling_instance_(false),
		is_impl_only_(false)
	{}

	KeyframeModel::~KeyframeModel() {
		if (run_state_ == RUNNING || run_state_ == PAUSED)
			SetRunState(ABORTED, base::TimeTicks());
	}

	void KeyframeModel::SetRunState(RunState run_state,
		base::TimeTicks monotonic_time) {
		char name_buffer[256];
		snprintf(name_buffer, sizeof(name_buffer), "%s-%d-%d",
			s_curveTypeNames[curve_->Type()], target_property_id_, group_);

		bool is_waiting_to_start =
			run_state_ == WAITING_FOR_TARGET_AVAILABILITY || run_state_ == STARTING;

		if (is_controlling_instance_ && is_waiting_to_start && run_state == RUNNING) {
			;// TRACE_EVENT_ASYNC_BEGIN1("cc", "KeyframeModel", this, "Name", TRACE_STR_COPY(name_buffer));
		}

		bool was_finished = is_finished();

		const char* old_run_state_name = s_runStateNames[run_state_];

		if (run_state == RUNNING && run_state_ == PAUSED)
			total_paused_duration_ += (monotonic_time - pause_time_);
		else if (run_state == PAUSED)
			pause_time_ = monotonic_time;
		run_state_ = run_state;

		const char* new_run_state_name = s_runStateNames[run_state];

// 		if (is_controlling_instance_ && !was_finished && is_finished())
// 			TRACE_EVENT_ASYNC_END0("cc", "KeyframeModel", this);

		char state_buffer[256];
		snprintf(state_buffer, sizeof(state_buffer), "%s->%s", old_run_state_name, new_run_state_name);

// 		TRACE_EVENT_INSTANT2(
// 			"cc", "ElementAnimations::SetRunState", TRACE_EVENT_SCOPE_THREAD, "Name",
// 			TRACE_STR_COPY(name_buffer), "State", TRACE_STR_COPY(state_buffer));
	}

	void KeyframeModel::Pause(base::TimeDelta pause_offset)
	{
		// Convert pause offset which is in local time to monotonic time.
		// TODO(yigu): This should be scaled by playbackrate. http://crbug.com/912407
		base::TimeTicks monotonic_time = pause_offset + start_time_ + total_paused_duration_;
		SetRunState(PAUSED, monotonic_time);
	}

	bool KeyframeModel::IsFinishedAt(base::TimeTicks monotonic_time) const
	{
		if (is_finished())
			return true;

		if (needs_synchronized_start_time_)
			return false;

		if (playback_rate_ == 0)
			return false;

		return run_state_ == RUNNING && iterations_ >= 0 &&
			(curve_->Duration() * (iterations_ / std::abs(playback_rate_))) <=
			(ConvertMonotonicTimeToLocalTime(monotonic_time) + time_offset_);
	}

// 	KeyframeModel::Phase KeyframeModel::CalculatePhaseForTesting(
// 		base::TimeDelta local_time) const {
// 		return CalculatePhase(local_time);
// 	}

	KeyframeModel::Phase KeyframeModel::CalculatePhase(base::TimeDelta local_time) const
	{
		base::TimeDelta opposite_time_offset = time_offset_ == std::numeric_limits<int64_t>::min()
			? std::numeric_limits<int64_t>::max()
			: -time_offset_;
		base::TimeDelta before_active_boundary_time =
			std::max(opposite_time_offset, base::TimeDelta());
		if (local_time < before_active_boundary_time ||
			(local_time == before_active_boundary_time && playback_rate_ < 0)) {
			return KeyframeModel::Phase::BEFORE;
		}
		// Scaling the duration is against spec but needed to comply with the cc
		// implementation. By spec (in blink) the playback rate is an Animation level
		// concept but in cc it's per KeyframeModel. We grab the active time
		// calculated here and later scale it with the playback rate in order to get a
		// proper progress. Therefore we need to un-scale it here. This can be fixed
		// once we scale the local time by playback rate. See
		// https://crbug.com/912407.
		base::TimeDelta active_duration = static_cast<base::TimeDelta>(
			curve_->Duration() * iterations_ / std::abs(playback_rate_));
		// TODO(crbug.com/909794): By spec end time = max(start delay + duration +
		// end delay, 0). The logic should be updated once "end delay" is supported.
		base::TimeDelta active_after_boundary_time =
			// Negative iterations_ represents "infinite iterations".
			iterations_ >= 0
			? std::max(opposite_time_offset + active_duration, base::TimeDelta())
			: std::numeric_limits<int64_t>::max();
		if (local_time > active_after_boundary_time ||
			(local_time == active_after_boundary_time && playback_rate_ > 0)) {
			return KeyframeModel::Phase::AFTER;
		}
		return KeyframeModel::Phase::ACTIVE;
	}

	std::optional<base::TimeDelta> KeyframeModel::CalculateActiveTime(base::TimeTicks monotonic_time) const
	{
		base::TimeDelta local_time = ConvertMonotonicTimeToLocalTime(monotonic_time);
		KeyframeModel::Phase phase = CalculatePhase(local_time);
		//DCHECK(playback_rate_);
		switch (phase) {
		case KeyframeModel::Phase::BEFORE:
			if (fill_mode_ == FillMode::BACKWARDS || fill_mode_ == FillMode::BOTH)
				return std::max(local_time + time_offset_, base::TimeDelta());
			return std::nullopt;
		case KeyframeModel::Phase::ACTIVE:
			return local_time + time_offset_;
		case KeyframeModel::Phase::AFTER:
			if (fill_mode_ == FillMode::FORWARDS || fill_mode_ == FillMode::BOTH) {
				//DCHECK_GE(iterations_, 0);
				base::TimeDelta active_duration = static_cast<base::TimeDelta>(
					curve_->Duration() * iterations_ / std::abs(playback_rate_));
				return std::max(std::min(local_time + time_offset_, active_duration),
					base::TimeDelta());
			}
			return std::nullopt;
		default:
			//NOTREACHED();
			return std::nullopt;
		}
	}

	bool KeyframeModel::InEffect(base::TimeTicks monotonic_time) const
	{
		return CalculateActiveTime(monotonic_time).has_value();
	}

	// TODO(yigu): Local time should be scaled by playback rate by spec.
	// https://crbug.com/912407.
	base::TimeDelta KeyframeModel::ConvertMonotonicTimeToLocalTime(base::TimeTicks monotonic_time) const
	{
		// When waiting on receiving a start time, then our global clock is 'stuck' at
		// the initial state.
		if ((run_state_ == STARTING && !has_set_start_time()) ||
			needs_synchronized_start_time())
			return base::TimeDelta();

		// If we're paused, time is 'stuck' at the pause time.
		base::TimeTicks time = (run_state_ == PAUSED) ? pause_time_ : monotonic_time;
		return time - start_time_ - total_paused_duration_;
	}

	base::TimeDelta KeyframeModel::TrimTimeToCurrentIteration(base::TimeTicks monotonic_time) const
	{
		//DCHECK(playback_rate_);
		//DCHECK_GE(iteration_start_, 0);

		//DCHECK(InEffect(monotonic_time));
		base::TimeDelta active_time = CalculateActiveTime(monotonic_time).value();
		base::TimeDelta start_offset = static_cast<base::TimeDelta>(curve_->Duration() * iteration_start_);

		// Return start offset if we are before the start of the keyframe model
		if (active_time < base::TimeDelta())
			return start_offset;
		// Always return zero if we have no iterations.
		if (!iterations_)
			return base::TimeDelta();

		// Don't attempt to trim if we have no duration.
		if (curve_->Duration() <= base::TimeDelta())
			return base::TimeDelta();

		base::TimeDelta repeated_duration = static_cast<base::TimeDelta>(curve_->Duration() * iterations_);
		base::TimeDelta active_duration = static_cast<base::TimeDelta>(
			repeated_duration / std::abs(playback_rate_));

		// Calculate the scaled active time
		base::TimeDelta scaled_active_time;
		if (playback_rate_ < 0) {
			scaled_active_time =
				static_cast<base::TimeDelta>((active_time - active_duration) * playback_rate_) + start_offset;
		}
		else {
			scaled_active_time = static_cast<base::TimeDelta>(active_time * playback_rate_) + start_offset;
		}

		// Calculate the iteration time
		base::TimeDelta iteration_time;
		if (scaled_active_time - start_offset == repeated_duration &&
			fmod(iterations_ + iteration_start_, 1) == 0)
			iteration_time = curve_->Duration();
		else
			iteration_time = scaled_active_time % curve_->Duration();

		// Calculate the current iteration
		int iteration;
		if (scaled_active_time <= base::TimeDelta())
			iteration = 0;
		else if (iteration_time == curve_->Duration())
			iteration = static_cast<int>(ceil(iteration_start_ + iterations_ - 1));
		else
			iteration = static_cast<int>(scaled_active_time / curve_->Duration());

		// Check if we are running the keyframe model in reverse direction for the
		// current iteration
		bool reverse =
			(direction_ == Direction::REVERSE) ||
			(direction_ == Direction::ALTERNATE_NORMAL && iteration % 2 == 1) ||
			(direction_ == Direction::ALTERNATE_REVERSE && iteration % 2 == 0);

		// If we are running the keyframe model in reverse direction, reverse the
		// result
		if (reverse)
			iteration_time = curve_->Duration() - iteration_time;

		return iteration_time;
	}

// 	void KeyframeModel::PushPropertiesTo(KeyframeModel* other) const {
// 		other->element_id_ = element_id_;
// 		if (run_state_ == KeyframeModel::PAUSED ||
// 			other->run_state_ == KeyframeModel::PAUSED) {
// 			other->run_state_ = run_state_;
// 			other->pause_time_ = pause_time_;
// 			other->total_paused_duration_ = total_paused_duration_;
// 		}
// 	}
	void KeyframeModel::SetIsImplOnly() {
		is_impl_only_ = true;
		// Impl only animations have a single instance which by definition is the
		// controlling instance.
		is_controlling_instance_ = true;
	}
}