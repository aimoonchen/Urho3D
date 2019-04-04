#include "TimingFunction.h"

TimingFunction::TimingFunction() = default;

TimingFunction::~TimingFunction() = default;

std::unique_ptr<CubicBezierTimingFunction>
CubicBezierTimingFunction::CreatePreset(EaseType ease_type) {
	switch (ease_type) {
	case EaseType::EASE:
		return std::make_unique<CubicBezierTimingFunction>(ease_type, 0.25, 0.1, 0.25, 1.0);
	case EaseType::EASE_IN:
		return std::make_unique<CubicBezierTimingFunction>(ease_type, 0.42, 0.0, 1.0, 1.0);
	case EaseType::EASE_OUT:
		return std::make_unique<CubicBezierTimingFunction>(ease_type, 0.0, 0.0, 0.58, 1.0);
	case EaseType::EASE_IN_OUT:
		return std::make_unique<CubicBezierTimingFunction>(ease_type, 0.42, 0.0, 0.58, 1);
	default:
		return nullptr;
	}
}
std::unique_ptr<CubicBezierTimingFunction>
CubicBezierTimingFunction::Create(double x1, double y1, double x2, double y2) {
	return std::make_unique<CubicBezierTimingFunction>(EaseType::CUSTOM, x1, y1, x2, y2);
}

CubicBezierTimingFunction::CubicBezierTimingFunction(EaseType ease_type,
	double x1,
	double y1,
	double x2,
	double y2)
	: bezier_(x1, y1, x2, y2), ease_type_(ease_type) {}

CubicBezierTimingFunction::~CubicBezierTimingFunction() = default;

TimingFunction::Type CubicBezierTimingFunction::GetType() const {
	return Type::CUBIC_BEZIER;
}

double CubicBezierTimingFunction::GetValue(double x) const {
	return 0.0;// bezier_.Solve(x);
}

double CubicBezierTimingFunction::Velocity(double x) const {
	return 0.0;// bezier_.Slope(x);
}

std::unique_ptr<TimingFunction> CubicBezierTimingFunction::Clone() const {
	return std::make_unique<CubicBezierTimingFunction>(*this);
}

std::unique_ptr<FramesTimingFunction> FramesTimingFunction::Create(int frames) {
	return std::make_unique<FramesTimingFunction>(frames);
}

FramesTimingFunction::FramesTimingFunction(int frames) : frames_(frames) {}

FramesTimingFunction::~FramesTimingFunction() = default;

TimingFunction::Type FramesTimingFunction::GetType() const {
	return Type::FRAMES;
}

double FramesTimingFunction::GetValue(double t) const {
	return GetPreciseValue(t);
}

std::unique_ptr<TimingFunction> FramesTimingFunction::Clone() const {
	return std::make_unique<FramesTimingFunction>(*this);
}

double FramesTimingFunction::Velocity(double x) const {
	return 0;
}

double FramesTimingFunction::GetPreciseValue(double t) const {
	const double frames = static_cast<double>(frames_);
	double output_progress = std::floor(frames * t) / (frames - 1);
	if (t <= 1 && output_progress > 1)
		output_progress = 1;
	return output_progress;
}