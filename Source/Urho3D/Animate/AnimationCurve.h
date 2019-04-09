#pragma once
#include <memory>
#include <vector>
#include "AnimationBase.h"

namespace animation
{
// 	namespace base
// 	{
// 		using TimeDelta = double;
// 		// 		class TimeDelta
// 		// 		{
// 		// 		public:
// 		// 			TimeDelta(double time) : time_{ time } {}
// 		// 			double InMicroseconds() { return time_; }
// 		// 		private:
// 		// 			double time_{ 0.0f };
// 		// 		};
// 	}

#include "AnimationBase.h"
#include "TimingFunction.h"

	class KeyFrameBase
	{
	public:
		base::TimeDelta Time() const { return time_; }
		const TimingFunction* timing_function() const { return timing_function_.get(); }
	protected:
		KeyFrameBase(base::TimeDelta time, std::unique_ptr<TimingFunction> timing_function)
			: time_(time), timing_function_{ std::move(timing_function) } {}
		virtual ~KeyFrameBase() = default;
	private:
		base::TimeDelta time_;
		std::unique_ptr<TimingFunction> timing_function_;
		DISALLOW_COPY_AND_ASSIGN(KeyFrameBase);
	};

	template<typename VT>
	class Keyframe : public KeyFrameBase
	{
	public:
		static std::unique_ptr<Keyframe<VT>> Create(base::TimeDelta time, VT value, std::unique_ptr<TimingFunction> timing_function)
		{
			return base::WrapUnique(new Keyframe<VT>(time, value, std::move(timing_function)));
		}

		std::unique_ptr<Keyframe<VT>> Clone() const
		{
			std::unique_ptr<TimingFunction> func;
			if (timing_function())
				func = timing_function()->Clone();
			return Keyframe<VT>::Create(Time(), Value(), std::move(func));
		}

		VT Value() const { return value_; }
		~Keyframe() override = default;
		
	private:
		Keyframe(base::TimeDelta time, VT value, std::unique_ptr<TimingFunction> timing_function)
			: KeyFrameBase(time, std::move(timing_function)), value_{ value }
		{

		}
	private:
		VT value_;
	};

	namespace {
		class TimeUtil {
		public:
			static double Divide(base::TimeDelta dividend, base::TimeDelta divisor) {
				return static_cast<double>(dividend/*.InMicroseconds()*/) /
					static_cast<double>(divisor/*.InMicroseconds()*/);
			}
		};

		template <class KeyframeType>
		void InsertKeyframe(std::unique_ptr<KeyframeType> keyframe,
			std::vector<std::unique_ptr<KeyframeType>>* keyframes) {
			// Usually, the keyframes will be added in order, so this loop would be
			// unnecessary and we should skip it if possible.
			if (!keyframes->empty() && keyframe->Time() < keyframes->back()->Time()) {
				for (size_t i = 0; i < keyframes->size(); ++i) {
					if (keyframe->Time() < keyframes->at(i)->Time()) {
						keyframes->insert(keyframes->begin() + i, std::move(keyframe));
						return;
					}
				}
			}

			keyframes->push_back(std::move(keyframe));
		}

		template <typename KeyframeType>
		base::TimeDelta TransformedAnimationTime(
			const std::vector<std::unique_ptr<KeyframeType>>& keyframes,
			const std::unique_ptr<TimingFunction>& timing_function,
			double scaled_duration,
			base::TimeDelta time) {
			if (timing_function) {
				base::TimeDelta start_time = static_cast<base::TimeDelta>(keyframes.front()->Time() * scaled_duration);
				base::TimeDelta duration = static_cast<base::TimeDelta>(
					(keyframes.back()->Time() - keyframes.front()->Time()) *
					scaled_duration);
				double progress = TimeUtil::Divide(time - start_time, duration);

				time = static_cast<base::TimeDelta>(duration * timing_function->GetValue(progress)) + start_time;
			}

			return time;
		}

		template <typename KeyframeType>
		size_t GetActiveKeyframe(
			const std::vector<std::unique_ptr<KeyframeType>>& keyframes,
			double scaled_duration,
			base::TimeDelta time) {
			//DCHECK_GE(keyframes.size(), 2ul);
			size_t i = 0;
			for (; i < keyframes.size() - 2; ++i) {  // Last keyframe is never active.
				if (time < (keyframes[i + 1]->Time() * scaled_duration))
					break;
			}

			return i;
		}

		template <typename KeyframeType>
		double TransformedKeyframeProgress(
			const std::vector<std::unique_ptr<KeyframeType>>& keyframes,
			double scaled_duration,
			base::TimeDelta time,
			size_t i) {
			base::TimeDelta time1 = static_cast<base::TimeDelta>(keyframes[i]->Time() * scaled_duration);
			base::TimeDelta time2 = static_cast<base::TimeDelta>(keyframes[i + 1]->Time() * scaled_duration);

			double progress = TimeUtil::Divide(time - time1, time2 - time1);

			if (keyframes[i]->timing_function()) {
				progress = keyframes[i]->timing_function()->GetValue(progress);
			}

			return progress;
		}

	}
	class AnimationCurve
	{
	public:
		enum CurveType {
			COLOR = 0,
			FLOAT,
			TRANSFORM,
			FILTER,
			SCROLL_OFFSET,
			SIZE,
			// This must be last
			LAST_CURVE_TYPE = SIZE,
		};
		virtual ~AnimationCurve() = default;
		virtual base::TimeDelta Duration() const = 0;
		virtual CurveType Type() const = 0;
		virtual float GetValue(base::TimeDelta t) const = 0;
		virtual std::unique_ptr<AnimationCurve> Clone() const = 0;
	};
	template<typename VT>
	class KeyframedAnimationCurve : public AnimationCurve
	{
	public:
		static std::unique_ptr<KeyframedAnimationCurve<VT>> Create()
		{
			return std::make_unique<KeyframedAnimationCurve<VT>>();
		}
		void AddKeyframe(std::unique_ptr<Keyframe<VT>> keyframe)
		{
			InsertKeyframe(std::move(keyframe), &keyframes_);
		}
		void SetTimingFunction(std::unique_ptr<TimingFunction> timing_function) {
			timing_function_ = std::move(timing_function);
		}
		double scaled_duration() const { return scaled_duration_; }
		void set_scaled_duration(double scaled_duration) { scaled_duration_ = scaled_duration; }

		base::TimeDelta Duration() const
		{
			return static_cast<base::TimeDelta>((keyframes_.back()->Time() - keyframes_.front()->Time()) * scaled_duration());
		}

		VT GetValue(base::TimeDelta t) const override
		{
			if (t <= (keyframes_.front()->Time() * scaled_duration()))
				return keyframes_.front()->Value();

			if (t >= (keyframes_.back()->Time() * scaled_duration()))
				return keyframes_.back()->Value();

			t = TransformedAnimationTime(keyframes_, timing_function_, scaled_duration(), t);
			auto i = GetActiveKeyframe(keyframes_, scaled_duration(), t);
			auto progress = TransformedKeyframeProgress(keyframes_, scaled_duration(), t, i);

			return static_cast<VT>(keyframes_[i]->Value() + (keyframes_[i + 1]->Value() - keyframes_[i]->Value()) * progress);
		}
		std::unique_ptr<AnimationCurve> Clone() const override
		{
			std::unique_ptr<KeyframedAnimationCurve<VT>> to_return = KeyframedAnimationCurve<VT>::Create();
			for (const auto& keyframe : keyframes_)
				to_return->AddKeyframe(keyframe->Clone());

			if (timing_function_)
				to_return->SetTimingFunction(timing_function_->Clone());

			to_return->set_scaled_duration(scaled_duration());

			return std::move(to_return);
		}
		CurveType Type() const override;
		KeyframedAnimationCurve() = default;
		~KeyframedAnimationCurve() = default;
	private:
		std::vector<std::unique_ptr<Keyframe<VT>>> keyframes_;
		std::unique_ptr<TimingFunction> timing_function_;
		double scaled_duration_{ 1.0f };
	};
	template<>
	AnimationCurve::CurveType KeyframedAnimationCurve<float>::Type() const { return AnimationCurve::FLOAT; }
}