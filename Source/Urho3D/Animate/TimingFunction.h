#pragma once
#include "AnimationBase.h"
#include <memory>

class TimingFunction {
public:
	enum class EaseType { EASE, EASE_IN, EASE_OUT, EASE_IN_OUT, CUSTOM };
	virtual ~TimingFunction();

	// Note that LINEAR is a nullptr TimingFunction (for now).
	enum class Type { LINEAR, CUBIC_BEZIER, STEPS, FRAMES };

	virtual Type GetType() const = 0;
	virtual double GetValue(double t) const = 0;
	virtual double Velocity(double time) const = 0;
	virtual std::unique_ptr<TimingFunction> Clone() const = 0;

protected:
	TimingFunction();

	DISALLOW_ASSIGN(TimingFunction);
};
namespace gfx
{
	class CubicBezier
	{
	public:
		CubicBezier(/*TimingFunction::EaseType ease_type, */double x1,
			double y1,
			double x2,
			double y2)
			: x1_{ x1 }
			, y1_{ y1 }
			, x2_{ x2 }
			, y2_{ y2 }
		{

		}
		double x1_;
		double y1_;
		double x2_;
		double y2_;
	};
}
class CubicBezierTimingFunction : public TimingFunction {
public:
	enum class EaseType { EASE, EASE_IN, EASE_OUT, EASE_IN_OUT, CUSTOM };

	static std::unique_ptr<CubicBezierTimingFunction> CreatePreset(
		EaseType ease_type);
	static std::unique_ptr<CubicBezierTimingFunction> Create(double x1,
		double y1,
		double x2,
		double y2);
	~CubicBezierTimingFunction() override;

	// TimingFunction implementation.
	Type GetType() const override;
	double GetValue(double time) const override;
	double Velocity(double time) const override;
	std::unique_ptr<TimingFunction> Clone() const override;

	EaseType ease_type() const { return ease_type_; }
	const gfx::CubicBezier& bezier() const { return bezier_; }


	CubicBezierTimingFunction(EaseType ease_type,
		double x1,
		double y1,
		double x2,
		double y2);
private:
	gfx::CubicBezier bezier_;
	EaseType ease_type_;

	DISALLOW_ASSIGN(CubicBezierTimingFunction);
};

// class CC_ANIMATION_EXPORT StepsTimingFunction : public TimingFunction {
// public:
// 	// Web Animations specification, 3.12.4. Timing in discrete steps.
// 	enum class StepPosition { START, MIDDLE, END };
// 
// 	static std::unique_ptr<StepsTimingFunction> Create(
// 		int steps,
// 		StepPosition step_position);
// 	~StepsTimingFunction() override;
// 
// 	// TimingFunction implementation.
// 	Type GetType() const override;
// 	double GetValue(double t) const override;
// 	std::unique_ptr<TimingFunction> Clone() const override;
// 	double Velocity(double time) const override;
// 
// 	int steps() const { return steps_; }
// 	StepPosition step_position() const { return step_position_; }
// 	double GetPreciseValue(double t) const;
// 
// private:
// 	StepsTimingFunction(int steps, StepPosition step_position);
// 
// 	float GetStepsStartOffset() const;
// 
// 	int steps_;
// 	StepPosition step_position_;
// 
// 	DISALLOW_ASSIGN(StepsTimingFunction);
// };

class FramesTimingFunction : public TimingFunction {
public:
	static std::unique_ptr<FramesTimingFunction> Create(int frames);
	~FramesTimingFunction() override;

	// TimingFunction implementation.
	Type GetType() const override;
	double GetValue(double t) const override;
	std::unique_ptr<TimingFunction> Clone() const override;
	double Velocity(double time) const override;

	int frames() const { return frames_; }
	double GetPreciseValue(double t) const;

	explicit FramesTimingFunction(int frames);
private:
	int frames_;

	DISALLOW_ASSIGN(FramesTimingFunction);
};