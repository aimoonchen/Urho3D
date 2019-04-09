#pragma once
#include <string>
#include <memory>
#include <optional>
#include "AnimationBase.h"

namespace animation
{
	class AnimationCurve;

	class KeyframeModel
	{
	public:
		enum RunState {
			WAITING_FOR_TARGET_AVAILABILITY = 0,
			WAITING_FOR_DELETION,
			STARTING,
			RUNNING,
			PAUSED,
			FINISHED,
			ABORTED,
			ABORTED_BUT_NEEDS_COMPLETION,
			// This sentinel must be last.
			LAST_RUN_STATE = ABORTED_BUT_NEEDS_COMPLETION
		};
		static std::string ToString(RunState);

		enum class Direction { NORMAL, REVERSE, ALTERNATE_NORMAL, ALTERNATE_REVERSE };

		enum class FillMode { NONE, FORWARDS, BACKWARDS, BOTH, AUTO };

		enum class Phase { BEFORE, ACTIVE, AFTER };

		static std::unique_ptr<KeyframeModel> Create(
			std::unique_ptr<AnimationCurve> curve,
			int keyframe_model_id,
			int group_id,
			int target_property_id);

		std::unique_ptr<KeyframeModel> CreateImplInstance(RunState initial_run_state) const;

		virtual ~KeyframeModel();
		int id() const { return id_; }
		int group() const { return group_; }
		int target_property_id() const { return target_property_id_; }

		RunState run_state() const { return run_state_; }
		void SetRunState(RunState run_state, base::TimeTicks monotonic_time);

		// This is the number of times that the keyframe model will play. If this
		// value is zero the keyframe model will not play. If it is negative, then
		// the keyframe model will loop indefinitely.
		double iterations() const { return iterations_; }
		void set_iterations(double n) { iterations_ = n; }

		double iteration_start() const { return iteration_start_; }
		void set_iteration_start(double iteration_start) {
			iteration_start_ = iteration_start;
		}

		base::TimeTicks start_time() const { return start_time_; }

		void set_start_time(base::TimeTicks monotonic_time) {
			start_time_ = monotonic_time;
		}
		bool has_set_start_time() const { return start_time_ != -1; }

		base::TimeDelta time_offset() const { return time_offset_; }
		void set_time_offset(base::TimeDelta monotonic_time) {
			time_offset_ = monotonic_time;
		}

		// Pause the keyframe effect at local time |pause_offset|.
		void Pause(base::TimeDelta pause_offset);

		Direction direction() { return direction_; }
		void set_direction(Direction direction) { direction_ = direction; }

		FillMode fill_mode() { return fill_mode_; }
		void set_fill_mode(FillMode fill_mode) { fill_mode_ = fill_mode; }

		double playback_rate() { return playback_rate_; }
		void set_playback_rate(double playback_rate) {
			playback_rate_ = playback_rate;
		}

		bool IsFinishedAt(base::TimeTicks monotonic_time) const;
		bool is_finished() const {
			return run_state_ == FINISHED || run_state_ == ABORTED ||
				run_state_ == WAITING_FOR_DELETION;
		}

		bool InEffect(base::TimeTicks monotonic_time) const;

		AnimationCurve* curve() { return curve_.get(); }
		const AnimationCurve* curve() const { return curve_.get(); }

		// If this is true, even if the keyframe model is running, it will not be
		// tickable until it is given a start time. This is true for KeyframeModels
		// running on the main thread.
		bool needs_synchronized_start_time() const {
			return needs_synchronized_start_time_;
		}
		void set_needs_synchronized_start_time(bool needs_synchronized_start_time) {
			needs_synchronized_start_time_ = needs_synchronized_start_time;
		}

		// This is true for KeyframeModels running on the main thread when the
		// FINISHED event sent by the corresponding impl keyframe model has been
		// received.
		bool received_finished_event() const { return received_finished_event_; }
		void set_received_finished_event(bool received_finished_event) {
			received_finished_event_ = received_finished_event;
		}
		// Takes the given absolute time, and using the start time and the number
		// of iterations, returns the relative time in the current iteration.
		base::TimeDelta TrimTimeToCurrentIteration(base::TimeTicks monotonic_time) const;
		void set_is_controlling_instance_for_test(bool is_controlling_instance) {
			is_controlling_instance_ = is_controlling_instance;
		}
		bool is_controlling_instance() const { return is_controlling_instance_; }
		
		void SetIsImplOnly();
		bool is_impl_only() const { return is_impl_only_; }

		void set_affects_active_elements(bool affects_active_elements) {
			affects_active_elements_ = affects_active_elements;
		}
		bool affects_active_elements() const { return affects_active_elements_; }

		void set_affects_pending_elements(bool affects_pending_elements) {
			affects_pending_elements_ = affects_pending_elements;
		}
		bool affects_pending_elements() const { return affects_pending_elements_; }
	protected:
	private:
		KeyframeModel(std::unique_ptr<AnimationCurve> curve,
			int keyframe_model_id,
			int group_id,
			int target_property_id);
		base::TimeDelta ConvertMonotonicTimeToLocalTime(base::TimeTicks monotonic_time) const;
		KeyframeModel::Phase CalculatePhase(base::TimeDelta local_time) const;
		std::optional<base::TimeDelta> CalculateActiveTime(base::TimeTicks monotonic_time) const;
	private:
		int id_;
		int group_;
		int target_property_id_;
		RunState run_state_{ WAITING_FOR_TARGET_AVAILABILITY };

		bool needs_synchronized_start_time_{ false };
		bool received_finished_event_{ false };
		bool is_controlling_instance_{ false };
		bool is_impl_only_{ false };

		bool affects_active_elements_{ true };
		bool affects_pending_elements_{ true };

		base::TimeTicks start_time_{ -1 };
		base::TimeDelta time_offset_;
		base::TimeTicks pause_time_;
		base::TimeDelta total_paused_duration_;

		Direction direction_{ Direction::NORMAL };
		double playback_rate_{ 1 };
		FillMode fill_mode_{ FillMode::BOTH };
		double iterations_{ 1 };
		double iteration_start_{ 0 };
		std::unique_ptr<AnimationCurve> curve_;

	};
}