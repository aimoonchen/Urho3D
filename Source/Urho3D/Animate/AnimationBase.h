#pragma once
#include <bitset>

#define DISALLOW_COPY(TypeName) \
  TypeName(const TypeName&) = delete

#define DISALLOW_ASSIGN(TypeName) TypeName& operator=(const TypeName&) = delete

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  DISALLOW_COPY(TypeName);                 \
  DISALLOW_ASSIGN(TypeName)

namespace animation {
	namespace base
	{
		template <typename Container>
		constexpr auto size(const Container& c) -> decltype(c.size()) {
			return c.size();
		}

		template <typename T, size_t N>
		constexpr size_t size(const T(&array)[N]) noexcept {
			return N;
		}

		using TimeTicks = int64_t;
		using TimeDelta = int64_t;
		// 	using TimeTicks = double;
		// 	using TimeDelta = double;

		template <typename T>
		std::unique_ptr<T> WrapUnique(T* ptr) {
			return std::unique_ptr<T>(ptr);
		}

	}

	class SkColor;
	class FilterOperations;
	class SizeF;

	class TransformOperations;
	class ScrollOffset;
	class KeyframeModel;

	class AnimationTarget{
	 public:
	  virtual ~AnimationTarget() {}
	  virtual void NotifyClientFloatAnimated(float opacity,
											 int target_property_id,
											 KeyframeModel* keyframe_model) = 0;
	  virtual void NotifyClientFilterAnimated(const FilterOperations& filter,
											  int target_property_id,
											  KeyframeModel* keyframe_model) = 0;
	  virtual void NotifyClientSizeAnimated(const SizeF& size,
											int target_property_id,
											KeyframeModel* keyframe_model) = 0;
	  virtual void NotifyClientColorAnimated(const SkColor& color,
											 int target_property_id,
											 KeyframeModel* keyframe_model) = 0;
	  virtual void NotifyClientTransformOperationsAnimated(
		  const TransformOperations& operations,
		  int target_property_id,
		  KeyframeModel* keyframe_model) = 0;
	  virtual void NotifyClientScrollOffsetAnimated(
		  const ScrollOffset& scroll_offset,
		  int target_property_id,
		  KeyframeModel* keyframe_model) = 0;
	};

	static constexpr size_t kMaxTargetPropertyIndex = 32u;
	// A set of target properties.
	using TargetProperties = std::bitset<kMaxTargetPropertyIndex>;

	namespace TargetProperty {

		// Must be zero-based as this will be stored in a bitset.
		enum Type {
			TRANSFORM = 0,
			OPACITY,
			FILTER,
			SCROLL_OFFSET,
			BACKGROUND_COLOR,
			BOUNDS,
			CSS_CUSTOM_PROPERTY,
			// These sentinels must be last
			FIRST_TARGET_PROPERTY = TRANSFORM,
			LAST_TARGET_PROPERTY = CSS_CUSTOM_PROPERTY
		};

	}  // namespace TargetProperty

}
