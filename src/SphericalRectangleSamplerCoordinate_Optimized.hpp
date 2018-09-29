#include <algorithm>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace rt {
	struct Rectangle {
		Rectangle() {}
		Rectangle(glm::dvec3 s, glm::dvec3 ex, glm::dvec3 ey)
			:_s(s)
			, _ex(ex)
			, _ey(ey) {
			_exLength = glm::length(ex);
			_eyLength = glm::length(ey);
			_x = ex / _exLength;
			_y = ey / _eyLength;
			_z = glm::cross(_x, _y);
		}
		glm::dvec3 sample(double u, double v) const {
			return _s + _ex * u + _ey * v;
		}
		double area() const {
			return _exLength * _eyLength;
		}
		glm::dvec3 normal() const {
			return _z;
		}
		glm::dvec3 s() const {
			return _s;
		}
		glm::dvec3 ex() const {
			return _ex;
		}
		glm::dvec3 ey() const {
			return _ey;
		}
		glm::dvec3 x() const {
			return _x;
		}
		glm::dvec3 y() const {
			return _y;
		}
		glm::dvec3 z() const {
			return _z;
		}
		double exLength() const {
			return _exLength;
		}
		double eyLength() const {
			return _eyLength;
		}
		glm::dvec3 _s;
		glm::dvec3 _ex;
		glm::dvec3 _ey;
		double _exLength = 0.0;
		double _eyLength = 0.0;
		glm::dvec3 _x;
		glm::dvec3 _y;
		glm::dvec3 _z;
	};

	class SphericalRectangleSamplerCoordinate {
	public:
		SphericalRectangleSamplerCoordinate(const Rectangle &rectangle, const glm::dvec3 &o) :_rectangle(rectangle) {
			_o = o;

			glm::dvec3 d = rectangle.s() - o;
			_x0 = glm::dot(d, rectangle.x());
			_y0 = glm::dot(d, rectangle.y());
			_z0 = glm::dot(d, rectangle.z());

			double exLen = rectangle.exLength();
			double eyLen = rectangle.eyLength();
			_x1 = _x0 + exLen;
			_y1 = _y0 + eyLen;

			// z flip
			if (_z0 > 0.0) {
				_z0 *= -1.0;
				_rectangle._z *= -1.0;
			}

			_z0z0 = _z0 * _z0;
			_y1y1 = _y1 * _y1;
			_n[0] = glm::dvec3(
				0.0,
				_z0,
				-_y0
			);
			_n[1] = glm::dvec3(
				-_z0,
				0.0,
				_x1
			);
			_n[2] = glm::dvec3(
				0.0,
				-_z0,
				_y1
			);
			_n[3] = glm::dvec3(
				_z0,
				0.0,
				-_x0
			);

			// 必要なのは、zだけであるので、正規化はzだけ
			_y0y0 = _y0 * _y0;
			_n[0].z /= std::sqrt(_z0z0 + _y0y0);
			_n[1].z /= std::sqrt(_z0z0 + _x1 * _x1);
			_n[2].z /= std::sqrt(_z0z0 + _y1y1);
			_n[3].z /= std::sqrt(_z0z0 + _x0 * _x0);

			/*
			_n[0].x == 0
			_n[1].y == 0
			_n[2].x == 0
			_n[3].y == 0
			であるため、z成分だけで良い
			*/
			_g[0] = std::acos(-_n[0].z * _n[1].z);
			_g[1] = std::acos(-_n[1].z * _n[2].z);
			_g[2] = std::acos(-_n[2].z * _n[3].z);
			_g[3] = std::acos(-_n[3].z * _n[0].z);

			_sr = _g[0] + _g[1] + _g[2] + _g[3] - glm::two_pi<double>();
		}

		double solidAngle() const {
			return _sr;
		}

		glm::dvec3 sample(double u, double v) const {
			double AQ = _sr;
			double phi_u = u * AQ - _g[2] - _g[3] + glm::two_pi<double>();

			auto safeSqrt = [](double x) {
				return std::sqrt(std::max(x, 0.0));
			};

			double b0 = _n[0].z;
			double b1 = _n[2].z;
			double fu = (std::cos(phi_u) * b0 - b1) / std::sin(phi_u);
			double cu = std::copysign(1.0, fu) / std::sqrt(fu * fu + b0 * b0);
			double xu = -cu * _z0 / safeSqrt(1.0 - cu * cu);

			double d = std::sqrt(xu * xu + _z0z0);
			double d2 = d * d;
			double h0 = _y0 / std::sqrt(d2 + _y0y0);
			double h1 = _y1 / std::sqrt(d2 + _y1y1);
			double hv = glm::mix(h0, h1, v);
			double yv = hv * d / safeSqrt(1.0 - hv * hv);
			return _o + xu * _rectangle.x() + yv * _rectangle.y() + _z0 * _rectangle.z();
		}

		Rectangle _rectangle;

		glm::dvec3 _o;

		double _x0;
		double _x1;
		double _y0;
		double _y1;
		double _z0;

		double _z0z0;
		double _y0y0;
		double _y1y1;

		double _sr;

		glm::dvec3 _n[4];
		double _g[4];
	};
}
