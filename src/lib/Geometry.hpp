//#pragma once
//
//#include <Geode/Geode.hpp>
//#include <ranges>
//#include "external/clipper2/clipper.h"
//
//using namespace geode::prelude;
//using namespace Clipper2Lib;
//
//namespace Sculptor {
//
//	CCPoint moveTowards(CCPoint point, CCPoint target, float distance);
//	CCPoint toPolar(float radius, float theta);
//	
//
//	using Points = std::vector<CCPoint>;
//
//	struct BezierCurve;
//	using BezierCurves = std::vector<BezierCurve>;
//
//	struct Line {
//	public:
//		CCPoint a, b;
//
//		Line(CCPoint a, CCPoint b) : a(a), b(b) {}
//
//		float angle() const;
//		float length() const;
//		std::optional<float> gradient() const;
//
//		bool contains(const CCPoint& point) const;
//		std::optional<CCPoint> evaluate(float x) const;
//		CCPoint projectionOf(const CCPoint& point) const;
//		CCPoint normal() const;
//		CCPoint lerp(float t) const {
//			return a.lerp(b, t);
//		}
//		CCPoint midpoint() const {
//			return lerp(0.5);
//		}
//		std::optional<float> inverseLerp(CCPoint) const;
//
//		bool coincident(const Line& other) const;
//		void shrink(float distance);
//		void shrinkProportional(float amount);
//
//	private:
//		mutable std::optional<float> m_angle;
//		mutable std::optional<float> m_length;
//		mutable std::optional<float> m_gradient;
//	};
//
//	using Lines = std::vector<Line>;
//
//	CCPoint projectOntoLines(CCPoint point, const Lines& lines);
//
//	struct Sequence {
//	public:
//
//		const Points& getPoints() const { return points; }
//
//		Lines edges() const {
//			recomputeIfDirty();
//			return _edges;
//		}
//		PathD asPath() const {
//			recomputeIfDirty();
//			return _path;
//		}
//		void recomputeIfDirty() const {
//			if (dirty) {
//				dirty = false;
//				recompute();
//			}
//		}
//		virtual void recompute() const;
//
//		float length() const;
//		std::vector<float> accumulatedLengths() const;
//		CCPoint centroid() const;
//		CCPoint normalAt(const CCPoint& point) const;
//		CCPoint lerp(float t) const;
//		std::optional<float> inverseLerp(const CCPoint& point) const;
//		CCPoint projectionOf(const CCPoint& point) const;
//		bool containsEdge(const Line& edge) const;
//		bool containsEdgePoint(const CCPoint& point) const { return !edgeIndicesContaining(point).empty(); }
//		std::vector<int> edgeIndicesContaining(const CCPoint& point) const;
//		CCRect boundingBox() const;
//
//		Sequence dashed(float spacing, float width) const;
//		static Sequence fromPath(PathD path);
//
//#pragma region Interface
//		Sequence() = default;
//		Sequence(Points points) : points(std::move(points)) {}
//		Sequence(std::initializer_list<CCPoint> points) : points(points) {}
//		template <std::ranges::range R>
//			requires (!std::derived_from<std::remove_cvref_t<R>, Sequence>)
//		Sequence(R&& range) : points(std::ranges::begin(range), std::ranges::end(range)) {}
//		virtual ~Sequence() = default;
//
//		auto size() const { return points.size(); }
//		auto begin() const { return points.begin(); }
//		auto end() const { return points.end(); }
//		auto front() const { return points.front(); }
//		auto back() const { return points.back(); }
//
//		const CCPoint& at(int i) const { return points.at(i); }
//
//		void insert(int i, CCPoint point) {
//			points.insert(points.begin() + i, std::move(point));
//			dirty = true;
//		}
//		void erase(int i) {
//			points.erase(points.begin() + i);
//			dirty = true;
//		}
//		void push_back(CCPoint point) {
//			points.push_back(point);
//			dirty = true;
//		}
//		void append(const Sequence& other) {
//			points.insert(points.end(), other.points.begin(), other.points.end());
//		}
//		void setPoint(int i, const CCPoint& point) {
//			points.at(i) = point;
//			dirty = true;
//		}
//#pragma endregion
//
//
//	protected:
//
//		Points points;
//
//		mutable bool dirty = true;
//		mutable Lines _edges;
//		mutable PathD _path;
//
//	};
//
//	using Sequences = std::vector<Sequence>;
//
//	Lines sequencesToLines(const Sequences& sequences);
//	CCPoint sequencesNormalAt(const Sequences& sequences, const CCPoint& point);
//
//	struct Poly : Sequence {
//
//		Poly() = default;
//		Poly(Sequence sequence) : Sequence(std::move(sequence)) {}
//
//		void recompute() const;
//
//		bool sharesEdge(const Poly& other) const;
//		bool contains(const CCPoint& point) const;
//		//CCPoint normalAt(const CCPoint& point) const;
//		
//
//		Sequence asSequence() const;
//		static Poly fromBezierCurves(BezierCurves curves);
//	};
//
//	using Polys = std::vector<Poly>;
//
//	struct Triangle : Poly {
//
//		Triangle() = default;
//		Triangle(Sequence sequence) : Poly(std::move(sequence)) {}
//
//		const CCPoint& a() const { return points[0]; }
//		const CCPoint& b() const { return points[1]; }
//		const CCPoint& c() const { return points[2]; }
//
//		const float ab() const { return a().getDistance(b()); }
//		const float bc() const { return b().getDistance(c()); }
//		const float ca() const { return c().getDistance(a()); }
//
//		float area() const;
//		std::vector<bool> edgesAreOnPolyBoundary(const Poly& poly) const;
//
//	};
//
//	using Triangles = std::vector<Triangle>;
//
//	struct BezierCurve : Sequence {
//	public:
//
//		using Sequence::Sequence;
//
//		float quality = 1;
//
//		static constexpr int approximationMin = 5;
//		static constexpr int approximationMax = 12;
//
//		static constexpr int lengthFactor = 5;
//		static constexpr int curvatureFactor = 1;
//
//		Sequence approximation() const {
//			recomputeIfDirty();
//			return _approximation;
//		}
//
//		void recompute() const;
//
//		CCPoint evaluate(float t) const;
//
//		float curvature() const;
//
//		std::pair<BezierCurve, BezierCurve> split(float t) const;
//
//		Sequence computeApproximation(float factor) const;
//
//	private:
//
//		mutable Sequence _approximation;
//
//	};
//
//	struct Circle {
//		CCPoint origin;
//		float radius;
//	};
//
//
//
//}
//
//template <>
//struct fmt::formatter<Sculptor::Line> {
//	constexpr auto parse(fmt::format_parse_context& ctx) {
//		return ctx.begin();
//	}
//
//	auto format(const Sculptor::Line& line, fmt::format_context& ctx) const {
//		fmt::format_to(ctx.out(), "[{:.4f}, {:.4f} -> {:.4f}, {:.4f}]", line.a.x, line.a.y, line.b.x, line.b.y);
//		return ctx.out();
//	}
//};
//
//template <>
//struct fmt::formatter<Sculptor::Sequence> {
//	constexpr auto parse(fmt::format_parse_context& ctx) {
//		return ctx.begin();
//	}
//
//	auto format(const Sculptor::Sequence& seq, fmt::format_context& ctx) const {
//		fmt::format_to(ctx.out(), "Sequence[{}]: ", seq.size());
//		for (const auto& p : seq.getPoints()) {
//			fmt::format_to(ctx.out(), "({:.4f}, {:.4f}) ", p.x, p.y);
//		}
//		return ctx.out();
//	}
//};