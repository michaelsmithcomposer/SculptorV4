#include <Geode/Geode.hpp>
#include "lib/Geometry2.hpp"

using namespace geode::prelude;
using namespace Clipper2Lib;

namespace Geometry {

	CCPoint moveTowards(CCPoint point, CCPoint target, float distance) {
		return point.lerp(target, distance / point.getDistance(target));
	}
	CCPoint toPolar(float radius, float theta) {
		return ccp(radius * cos(theta), radius * sin(theta));
	}

	float Line::length() const {
		return _a.getDistance(_b);
	}
	float Line::angle() const {
		CCPoint d = (_a - _b);
		return atan2(d.y, d.x);		 
	}
	std::optional<float> Line::gradient() const {
		if (_a.x != _b.x) return (_b.y - _a.y) / (_b.x - _a.x);
		return std::nullopt;
	}
	CCPoint Line::normal() const {
		CCPoint d = (_b - _a).normalize();
		return { d.y, -d.x };
	}

	std::optional<CCPoint> Line::evaluateAt(float x) const {
		if (_a.x != _b.x) {
			float y = *gradient() * (x - _a.x) + _a.y;
			if (Sculptor::rangeContains(_a.x, _b.x, x)) {
				return ccp(x, y);
			}
		}
		return std::nullopt;
	}
	CCPoint Line::projectionOf(CCPoint point) const {
		if (_a.x == _b.x) {
			return ccp(_a.x, Sculptor::clamp(point.y, _a.y, _b.y));
		}
		else {
			CCPoint projection = (point - _b).project(_a - _b) + _b;
			float x = Sculptor::clamp(projection.x, _a.x, _b.x);
			return *evaluateAt(x);
		}
	}
	CCPoint Line::lerp(float t) const {
		return _a.lerp(_b, t);
	}
	float Line::inverseLerp(CCPoint point) const {
		return _a.getDistance(point) / length();
	}

	bool Line::contains(CCPoint point) const {
		if (_a.x == _b.x) {
			return Sculptor::isClose(point.x, _a.x) && Sculptor::rangeContains(_a.y, _b.y, point.y);
		}
		else {
			std::optional<CCPoint> result = evaluateAt(point.x);
			return result && Sculptor::isClose(result.value().y, point.y);
		}
	}
	bool Line::coincidentWith(const Line& other) const {
		return (Sculptor::isClose(_a, other.a()) && Sculptor::isClose(_b, other.b())) || (Sculptor::isClose(_a, other.b()) && Sculptor::isClose(_b, other.a()));
	}

	//

	Sequence Sequence::fromPath(Clipper2Lib::PathD path) {
		Sequence sequence;
		for (const auto& point : path) {
			sequence.push_back(ccp(point.x, point.y));
		}
		return sequence;
	}
	Sequence Sequence::dashed(const Sequence& sequence, float spacing, float width) {
		Sequence result;
		float count = std::floor(sequence.length() / spacing);
		for (int i = 0; i < count; i++) {
			result.push_back(sequence.lerp(i / count));
			result.push_back(sequence.lerp((i + width) / count));
		}
		return result;
	}

	const Lines& Sequence::computeEdges() const {
		Lines result;
		for (const auto& [a, b] : _points | std::views::pairwise) {
			result.push_back(Line(a, b));
		}
		return result;
	}

	const Lines& Sequence::edges() const {
		if (!_edges) {
			_edges = computeEdges();
		}
		return *_edges;
	}
	const PathD& Sequence::path() const {
		if (!_path) {
			PathD result;
			for (const auto& point : _points) {
				result.push_back({ point.x, point.y });
			}
			_path = result;
		}
		return *_path;
	}
	const std::vector<float>& Sequence::cumulativeLengths() const {
		if (!_cumulativeLengths) {
			std::vector<float> result;
			for (auto [i, edge] : std::views::enumerate(edges())) {
				result.push_back(((i > 0) ? result[i - 1] : 0) + edge.length());
			}
			_cumulativeLengths = result;
		}
		return *_cumulativeLengths;
	}
	const float& Sequence::length() const {
		return cumulativeLengths().back();
	}
	const CCPoint& Sequence::centroid() const {
		if (!_centroid) {
			CCPoint result;
			for (const auto& point : _points) {
				result += point;
			}
			_centroid = result / static_cast<float>(size());
		}
		return *_centroid;
	}
	const CCRect& Sequence::boundingBox() const {
		if (!_boundingBox) {
			auto rect = Clipper2Lib::GetBounds(path());
			_boundingBox = { static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top) };
		}
		return *_boundingBox;
	}

	CCPoint Sequence::normalAt(const CCPoint& point) const {
		auto [edge, projection] = nearestLineAndProjection(point, edges());
		return edge.normal();
	}
	CCPoint Sequence::projectionOf(CCPoint point) const {
		return projectOntoLines(point, edges());
	}
	CCPoint Sequence::lerp(float t) const {	
		int index = 0;
		for (auto [i, l] : cumulativeLengths() | std::views::enumerate) {
			if (length() <= l) {
				index = i;
				break;
			}
		}
		Line line = Line(at(index), at((index + 1) % size()));
		float offset = (index == 0) ? 0 : cumulativeLengths().at(index - 1);
		float line_t = (length() - offset) / line.length();
		return line.lerp(line_t);
	}
	std::optional<float> Sequence::inverseLerp(const CCPoint& point) const {
		auto indices = edgeIndicesContaining(point);
		if (!indices.empty()) {
			int i = indices.front();
			float size = static_cast<float>(this->size()) - 1;
			return std::lerp((i / size), ((i + 1) / size), edges()[i].inverseLerp(point));
		}
		else {
			return std::nullopt;
		}
	}

	bool Sequence::containsEdge(const Line& edge) const {
		for (const auto& [i, e] : std::views::enumerate(edges())) {
			if (e.coincidentWith(edge)) {
				return true;
			}
		}
		return false;
	}
	std::vector<int> Sequence::edgeIndicesContaining(const CCPoint& point) const {
		std::vector<int> result;
		for (const auto& [i, line] : std::views::enumerate(edges())) {
			if (line.contains(point)) {
				result.push_back(i);
			}
		}
		return result;
	}

	std::pair<Line, CCPoint> nearestLineAndProjection(CCPoint point, const Lines& lines) {
		float bestDistance = FLT_MAX;
		CCPoint bestPoint;
		int bestIndex;
		for (const auto& [i, line] : lines | std::views::enumerate) {
			CCPoint projection = line.projectionOf(point);
			float d = projection.getDistance(point);
			if (d < bestDistance) {
				bestDistance = d;
				bestPoint = projection;
				bestIndex = i;
			}
		}	
		return std::make_pair(lines[bestIndex], bestPoint);
	}
	CCPoint projectOntoLines(CCPoint point, const Lines& lines) {
		return nearestLineAndProjection(point, lines).second;
	}
	
	//

	const Lines& Poly::computeEdges() const {
		Lines result = Sequence::computeEdges();
		result.push_back(Line(back(), front()));
		return result;
	}
	bool Poly::contains(CCPoint point) const {		
		auto p = path();
		auto result = PointInPolygon({ point.x, point.y }, p);
		return result != Clipper2Lib::PointInPolygonResult::IsOutside;
	}
	Sequence Poly::asSequence() const {
		Sequence result = Sequence{ _points };
		result.push_back(front());
		return result;
	}

	//

	float Triangle::area() const {
		CCPoint ab = b() - a();
		CCPoint ac = c() - a();
		return abs(ab.cross(ac)) * 0.5;
	}

	//

	std::pair<BezierCurve, BezierCurve> BezierCurve::split(float t) const {
		Points curve = _points;
		Points left, right;

		left.push_back(curve.front());
		right.push_back(curve.back());

		int n = curve.size();
		for (int order = 1; order < n; order++) {
			for (int i = 0; i < n - order; i++) {
				curve[i] = curve[i].lerp(curve[i + 1], t);
			}
			left.push_back(curve.front());
			right.push_back(curve[n - order - 1]);
		}

		std::ranges::reverse(right);
		return std::make_pair(BezierCurve(left), BezierCurve(right));
	}

	const Sequence& BezierCurve::approximation() const {
		if (!_approximation) {
			if (size() < 3) {
				_approximation = { _points };
			}
			else {
				float l = (length() / Sculptor::gdUnit) * Sculptor::bezierLengthFactor;
				float c = (curvature() / PI) * Sculptor::bezierCurveFactor;

				int count = Sculptor::clamp(floor(std::max(quality, 0.1f) * l * c), Sculptor::bezierMinEdges, Sculptor::bezierMaxEdges);

				Sequence result;
				result.push_back(front());
				for (int i = 1; i < count; i++) {
					float t = static_cast<float>(i) / count;
					result.push_back(evaluate(t));
				}
				result.push_back(back());
				_approximation = result;
			}			
		}
		return *_approximation;
	}

	CCPoint BezierCurve::evaluate(float t) const {
		Points curve = _points;

		int n = curve.size();
		for (int order = 1; order < n; order++) {
			for (int i = 0; i < n - order; i++) {
				curve[i] = curve[i].lerp(curve[i + 1], t);
			}
			if (order == n - 1) {
				return curve[0];
			}
		}
	}

	float BezierCurve::curvature() const {
		if (size() < 3) {
			return 0;
		}
		auto e = edges();
		float maxAngle = 0;
		for (const auto& [i, edge] : std::views::enumerate(e | std::views::take(e.size() - 1))) {
			float angle = acos((edge.a() - edge.b()).normalize().dot((e[i + 1].b() - e[i + 1].a()).normalize()));
			maxAngle = std::max(maxAngle, angle);
		}
		return maxAngle;
	}
}