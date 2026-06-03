#pragma once

#include <Geode/Geode.hpp>
#include <ranges>
#include "external/clipper2/clipper.h"
#include "lib/Utilities.hpp"

using namespace geode::prelude;
using namespace Clipper2Lib;

namespace Geometry {

	CCPoint moveTowards(CCPoint point, CCPoint target, float distance);
	CCPoint toPolar(float radius, float theta);

	using Points = std::vector<CCPoint>;

	class Line {
	public:

		Line(CCPoint a, CCPoint b) : _a(a), _b(b) {}

		const CCPoint& a() const { return _a; }
		const CCPoint& b() const { return _b; }

		float length() const;
		float angle() const;
		std::optional<float> gradient() const;
		CCPoint normal() const;

		std::optional<CCPoint> evaluateAt(float x) const;
		CCPoint projectionOf(CCPoint point) const;
		CCPoint lerp(float t) const;
		float inverseLerp(CCPoint point) const;

		bool contains(CCPoint point) const;
		bool coincidentWith(const Line& other) const;

		

	private:

		CCPoint _a;
		CCPoint _b;	

	};

	using Lines = std::vector<Line>;

	//

	class Sequence {
	public:

		#pragma region Interface	
		Sequence() = default;	
		Sequence(Points points) : points(std::move(points)) {}
		template <std::ranges::range R>
			requires (!std::derived_from<std::remove_cvref_t<R>, Sequence>)
		Sequence(R&& range) : _points(std::ranges::begin(range), std::ranges::end(range)) {}
		virtual ~Sequence() = default;

		auto size() const { return _points.size(); }
		auto begin() const { return _points.begin(); }
		auto end() const { return _points.end(); }
		auto front() const { return _points.front(); }
		auto back() const { return _points.back(); }
		
		void insert(int i, CCPoint point) {
			_points.insert(_points.begin() + i, point);
			resetCache();
		}
		void erase(int i) {
			_points.erase(_points.begin() + i);
			resetCache();
		}
		void push_back(CCPoint point) {
			_points.push_back(point);
			resetCache();
		}
		void append(const Sequence& other) {
			_points.insert(end(), other.begin(), other.end());
			resetCache();
		}
		void setPoint(int i, const CCPoint& point) {
			_points.at(i) = point;
			resetCache();
		}

		const Points& points() const { return _points; }
		const CCPoint& at(int i) const { return _points.at(i); }
		#pragma endregion

		static Sequence fromPath(PathD path);
		static Sequence dashed(const Sequence& sequence, float spacing, float width);

		virtual const Lines& computeEdges() const;

		const Lines& edges() const;
		const PathD& path() const;
		const std::vector<float>& cumulativeLengths() const;
		const float& length() const;
		const CCPoint& centroid() const;
		const CCRect& boundingBox() const;

		CCPoint normalAt(const CCPoint& point) const;
		CCPoint projectionOf(CCPoint point) const;
		CCPoint lerp(float t) const;
		std::optional<float> inverseLerp(const CCPoint& point) const;		

		bool containsEdge(const Line& edge) const;
		bool containsEdgePoint(const CCPoint& point) const { return !edgeIndicesContaining(point).empty(); }
		std::vector<int> edgeIndicesContaining(const CCPoint& point) const;


	protected:

		Points _points;

		mutable std::optional<Lines> _edges;
		mutable std::optional <PathD> _path;
		mutable std::optional <std::vector<float>> _cumulativeLengths;
		mutable std::optional <CCPoint> _centroid;
		mutable std::optional <CCRect> _boundingBox;

		virtual void resetCache() {
			_edges.reset();
			_path.reset();
			_cumulativeLengths.reset();
			_centroid.reset();
			_boundingBox.reset();
		}

	};

	using Sequences = std::vector<Sequence>;

	std::pair<Line, CCPoint> nearestLineAndProjection(CCPoint point, const Lines& lines);
	CCPoint projectOntoLines(CCPoint point, const Lines& lines);

	//

	class Poly : public Sequence {
	public:

		using Sequence::Sequence;

		const Lines& computeEdges() const override;

		bool contains(CCPoint point) const;
		Sequence asSequence() const;

	};

	using Polys = std::vector<Poly>;

	//

	struct Triangle : Poly {

		using Poly::Poly;

		const CCPoint& a() const { return at(0); }
		const CCPoint& b() const { return at(1); }
		const CCPoint& c() const { return at(2); }

		const float ab() const { return a().getDistance(b()); }
		const float bc() const { return b().getDistance(c()); }
		const float ca() const { return c().getDistance(a()); }

		float area() const;

	};

	using Triangles = std::vector<Triangle>;

	//

	class BezierCurve : public Sequence {
	public:

		using Sequence::Sequence;

		std::pair<BezierCurve, BezierCurve> split(float t) const;
		const Sequence& approximation() const;
		CCPoint evaluate(float t) const;
		float curvature() const;
		

		float quality = 1;


	private:

		mutable std::optional <Sequence> _approximation;

		void resetCache() override {
			_approximation.reset();
		}
	};

	using BezierCurves = std::vector<BezierCurve>;

	//

	struct Circle {
		CCPoint origin;
		float radius;
	};

	using Circles = std::vector<Circle>;

}

//

template <>
struct fmt::formatter<Geometry::Line> {
	constexpr auto parse(fmt::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const Geometry::Line& line, fmt::format_context& ctx) const {
		fmt::format_to(ctx.out(), "[{:.4f}, {:.4f} -> {:.4f}, {:.4f}]", line.a().x, line.a().y, line.b().x, line.b().y);
		return ctx.out();
	}
};

template <>
struct fmt::formatter<Geometry::Sequence> {
	constexpr auto parse(fmt::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const Geometry::Sequence& seq, fmt::format_context& ctx) const {
		fmt::format_to(ctx.out(), "Sequence[{}]: ", seq.size());
		for (const auto& p : seq.points()) {
			fmt::format_to(ctx.out(), "({:.4f}, {:.4f}) ", p.x, p.y);
		}
		return ctx.out();
	}
};