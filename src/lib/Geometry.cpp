//#include <Geode/Geode.hpp>
//#include <ranges>
//#include "lib/Geometry.hpp"
//#include "lib/Utilities.hpp"
//#include "external/clipper2/clipper.h"
//
//using namespace geode::prelude;
//
//namespace Sculptor {
//
//	CCPoint moveTowards(CCPoint point, CCPoint target, float distance) {
//		return point.lerp(target, distance / point.getDistance(target));
//	}
//
//	CCPoint toPolar(float radius, float theta) {
//		return ccp(radius * cos(theta), radius * sin(theta));
//	}
//
//	float Line::angle() const {
//		if (!m_angle) {
//			CCPoint norm = (b - a).normalize();
//			m_angle = atan2(norm.y, norm.x);
//		}
//		return *m_angle;
//	}
//
//	float Line::length() const {
//		if (!m_length) {
//			m_length = a.getDistance(b);
//		}
//		return *m_length;
//	}
//
//	std::optional<float> Line::gradient() const {
//		if (!m_gradient && (a.x != b.x)) {
//			m_gradient = (b.y - a.y) / (b.x - a.x);
//		}
//		return m_gradient;
//	}
//
//	bool Line::contains(const CCPoint& point) const {
//		if (a.x == b.x) {
//			return isClose(point.x, a.x) && rangeContains(a.y, b.y, point.y);
//		}
//		else {
//			std::optional<CCPoint> result = evaluate(point.x);
//			return (result.has_value()) && (isClose(result.value().y, point.y));
//		}
//	}
//
//	std::optional<CCPoint> Line::evaluate(float x) const {
//		if (a.x != b.x) {
//			float y = *gradient() * (x - a.x) + a.y;
//			if (rangeContains(a.x, b.x, x)) {
//				return ccp(x, y);
//			}
//		}
//		return std::nullopt;
//	}
//
//	CCPoint Line::projectionOf(const CCPoint& point) const {
//		if (a.x == b.x) {
//			return ccp(a.x, clamp(point.y, a.y, b.y));
//		}
//		else {
//			CCPoint projection = (point - b).project(a - b) + b;
//			float x = std::clamp(projection.x, std::min(a.x, b.x), std::max(a.x, b.x));
//			return *evaluate(x);
//		}
//	}
//
//	bool Line::coincident(const Line& other) const {
//		return (isClose(a, other.a) && isClose(b, other.b)) || (isClose(a, other.b) && isClose(b, other.a));
//	}
//
//	CCPoint Line::normal() const {
//		CCPoint dir = (b - a).normalize();
//		return ccp(dir.y, -dir.x);
//	}
//
//	void Line::shrink(float distance) {
//		auto _a = a;
//		auto _b = b;
//		a = moveTowards(_a, _b, distance);
//		b = moveTowards(_b, _a, distance);
//	}
//
//	void Line::shrinkProportional(float amount) {
//		auto _a = a;
//		auto _b = b;
//		a = _a.lerp(_b, amount);
//		b = _b.lerp(_a, amount);
//	}
//
//	std::optional<float> Line::inverseLerp(CCPoint point) const {
//		if (contains(point)) {
//			return a.getDistance(point) / a.getDistance(b);
//		}
//		else {
//			return std::nullopt;
//		}
//
//	}
//
//	CCPoint projectOntoLines(CCPoint point, const Lines& lines) {
//		auto closestEdge = std::ranges::min_element(lines, {}, [point](const Line& line) { return point.getDistance(line.projectionOf(point)); });
//		return closestEdge->projectionOf(point);
//	}
//
//	//
//
//	void Sequence::recompute() const {
//		_edges.clear();
//		_path.clear();
//		for (const auto& [i, point] : std::views::enumerate(points)) {
//			if (i != points.size() - 1) {
//				_edges.push_back(Line(point, points[i + 1]));
//			}
//			_path.push_back({ point.x, point.y });
//		}
//	}
//
//	float Sequence::length() const {
//		float l = 0;
//		for (const auto& edge : edges()) {
//			l += edge.length();
//		}
//		return l;
//	}
//
//	CCPoint Sequence::centroid() const {
//		CCPoint result;
//		for (const auto& point : points) {
//			result += point;
//		}
//		return result / static_cast<float>(points.size());
//	}
//
//
//	CCPoint Sequence::normalAt(const CCPoint& point) const {
//		CCPoint normal;
//		for (const auto& edge : edges()) {
//			normal += edge.normal() * (1 / (1e-6f + point.getDistance(edge.projectionOf(point))));
//		}
//		return normal.normalize();
//	}
//
//	Sequence Sequence::fromPath(Clipper2Lib::PathD path) {
//		Sequence sequence;
//		for (const auto& point : path) {
//			sequence.push_back(ccp(point.x, point.y));
//		}
//		return sequence;
//	}
//
//	CCRect Sequence::boundingBox() const {
//		auto rect = Clipper2Lib::GetBounds(asPath());
//		return { static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top) };
//	}
//
//	bool Sequence::containsEdge(const Line& edge) const {
//		for (const auto& [i, e] : std::views::enumerate(edges())) {
//			if (e.coincident(edge)) {
//				return true;
//			}
//		}
//		return false;
//	}
//
//
//	std::vector<int> Sequence::edgeIndicesContaining(const CCPoint& point) const {
//		std::vector<int> result;
//		for (const auto& [i, line] : std::views::enumerate(edges())) {
//			if (line.contains(point)) {
//				result.push_back(i);
//			}
//		}
//		return result;
//	}
//
//	Sequence Sequence::dashed(float spacing, float width) const {
//		Sequence sequence;
//		float count = std::floor(length() / spacing);
//		for (int i = 0; i < count; i++) {
//			sequence.push_back(lerp(i / count));
//			sequence.push_back(lerp((i + width) / count));
//		}
//		return sequence;
//	}
//
//	std::vector<float> Sequence::accumulatedLengths() const {
//		std::vector<float> lengths;
//		for (auto [i, edge] : std::views::enumerate(edges())) {
//			if (i == 0) {
//				lengths.push_back(edge.length());
//			}
//			else {
//				lengths.push_back(lengths.at(i - 1) + edge.length());
//			}
//		}
//		return lengths;
//	}
//
//	CCPoint Sequence::lerp(float t) const {
//		auto lengths = accumulatedLengths();
//		float length = t * lengths.back();
//		int index = 0;
//
//		for (auto [i, l] : std::views::enumerate(lengths)) {
//			if (length <= l) {
//				index = i;
//				break;
//			}
//		}
//		Line line = Line(points.at(index), points.at((index + 1) % this->points.size()));
//		float offset = (index == 0) ? 0 : lengths.at(index - 1);
//		float line_t = (length - offset) / line.length();
//
//		return line.lerp(line_t);
//	}
//
//	std::optional<float> Sequence::inverseLerp(const CCPoint& point) const {
//		auto indices = edgeIndicesContaining(point);
//		if (!indices.empty()) {
//			int i = indices.front();
//			float size = static_cast<float>(this->size()) - 1;
//			return std::lerp((i / size), ((i + 1) / size), *edges()[i].inverseLerp(point));
//		}
//		else {
//			return std::nullopt;
//		}
//	}
//
//	CCPoint Sequence::projectionOf(const CCPoint& point) const {
//		return projectOntoLines(point, edges());
//	}
//
//	Lines sequencesToLines(const Sequences& sequences) {
//		Lines result;
//		for (const auto& seq : sequences) {
//			result.append_range(seq.edges());
//		}
//		return result;
//	}
//
//	CCPoint sequencesNormalAt(const Sequences& sequences, const CCPoint& point) {		
//		float bestDistance = FLT_MAX;
//		CCPoint bestNormal;
//		for (const auto& sequence : sequences) {			
//			float d = point.getDistance(sequence.projectionOf(point));
//			if (d < bestDistance) {
//				bestDistance = d;
//				bestNormal = sequence.normalAt(point);
//			}
//		}		
//		return bestNormal;
//	}
//
//	//
//
//	void Poly::recompute() const {
//		_edges.clear();
//		_path.clear();
//		for (const auto& [i, point] : std::views::enumerate(points)) {
//			_edges.push_back(Line(point, points[(i + 1) % points.size()]));
//			_path.push_back({ point.x, point.y });
//		}
//	}
//
//	Poly Poly::fromBezierCurves(BezierCurves curves) {
//		Poly poly;
//		for (const auto& curve : curves) {
//			for (const auto& point : curve | std::views::take(curve.size() - 1)) {
//				poly.push_back(point);
//			}
//		}
//		return poly;
//	}
//
//	Sequence Poly::asSequence() const {
//		Sequence result = Sequence{ points };
//		result.push_back(points.front());
//		return result;
//	}
//
//	bool Poly::sharesEdge(const Poly& other) const {
//		for (const auto& edge : edges()) {
//			if (other.containsEdge(edge)) {
//				return true;
//			}
//		}
//		return false;
//	}
//
//	bool Poly::contains(const CCPoint& point) const {
//		if (containsEdgePoint(point)) { return true; }
//		int hits = 0;
//		for (const auto& edge : edges()) {
//			std::optional<CCPoint> p = edge.evaluate(point.x);
//			if (p && ((*p).y < point.y) && ((edge.a.x <= (*p).x && (*p).x < edge.b.x) || (edge.a.x > (*p).x && (*p).x >= edge.b.x))) {
//				hits++;
//			}
//		}
//		return (hits % 2 == 1);
//	}
//
//
//	//	
//
//	float Triangle::area() const {
//		CCPoint ab = b() - a();
//		CCPoint ac = c() - a();
//		return abs(ab.cross(ac)) * 0.5;
//	}
//
//	std::vector<bool> Triangle::edgesAreOnPolyBoundary(const Poly& poly) const {
//		return { poly.containsEdge(Line(a(), b())), poly.containsEdge(Line(b(), c())), poly.containsEdge(Line(c(), a())) };
//	}
//
//	//
//
//	void BezierCurve::recompute() const {
//		Sequence::recompute();
//		_approximation = computeApproximation(quality);
//	}
//
//	CCPoint BezierCurve::evaluate(float t) const {
//		Points curve = points;
//
//		int n = curve.size();
//		for (int order = 1; order < n; order++) {
//			for (int i = 0; i < n - order; i++) {
//				curve[i] = curve[i].lerp(curve[i + 1], t);
//			}
//			if (order == n - 1) {
//				return curve[0];
//			}
//		}
//	}
//
//	std::pair<BezierCurve, BezierCurve> BezierCurve::split(float t) const {
//		Points curve = points;
//		Points left, right;
//
//		left.push_back(curve.front());
//		right.push_back(curve.back());
//
//		int n = curve.size();
//		for (int order = 1; order < n; order++) {
//			for (int i = 0; i < n - order; i++) {
//				curve[i] = curve[i].lerp(curve[i + 1], t);
//			}
//			left.push_back(curve.front());
//			right.push_back(curve[n - order - 1]);
//		}
//
//		std::ranges::reverse(right);
//		return std::make_pair(BezierCurve(left), BezierCurve(right));
//	}
//
//	float BezierCurve::curvature() const {
//		if (size() < 3) {
//			return 0;
//		}
//		auto e = edges();
//		float maxAngle = 0;
//		for (const auto& [i, edge] : std::views::enumerate(e | std::views::take(e.size() - 1))) {
//			float angle = acos((edge.a - edge.b).normalize().dot((e[i + 1].b - e[i + 1].a).normalize()));
//			maxAngle = std::max(maxAngle, angle);
//		}
//		return maxAngle;
//	}
//
//	Sequence BezierCurve::computeApproximation(float factor) const {
//		if (size() < 3) {
//			return { points };
//		}
//
//		float l = (length() / gdUnit) * lengthFactor;
//		float c = (curvature() / PI) * curvatureFactor;
//
//		int count = clamp(floor(std::max(factor, 0.1f) * l * c), approximationMin, approximationMax);
//
//		Sequence result;
//		result.push_back(points.front());
//		for (int i = 1; i < count; i++) {
//			float t = static_cast<float>(i) / count;
//			result.push_back(evaluate(t));
//		}
//		result.push_back(points.back());
//		return result;
//	}
//
//	//
//
//
//
//}