#include <iostream>
#include <cmath>
#include <limits>

template <typename ValueType = float>
struct Spherical
{
	ValueType distance, theta, fi;

	Spherical(ValueType gdistance, ValueType gtheta, ValueType gfi) :
		distance(gdistance) , theta(gtheta), fi(gfi) { }

	ValueType getX() const { return distance * std::cos(theta)*std::cos(fi); }
	ValueType getY() const { return distance * std::sin(theta); }
	ValueType getZ() const { return distance * std::cos(theta)*std::sin(fi); }

	friend std::ostream& operator<<(std::ostream& stream, const Spherical& sp) {
		stream << "Cartesian: distance=" << sp.distance << ", theta=" << sp.theta << ", fi=" << sp.fi;
		return stream;
	}
};

template <typename ValueType = float>
struct Cartesian {
	ValueType x, y, z;

	Cartesian(ValueType x, ValueType y, ValueType z) :
		x(x), y(y), z(z) { }

	ValueType getX() { return x; }
	ValueType getY() { return y; }
	ValueType getZ() { return z; }

	friend std::ostream& operator<<(std::ostream& stream, const Cartesian& cart) {
		stream << "Cartesian: x=" << cart.x << ", y=" << cart.y << ", z=" << cart.z;
		return stream;
	}
};

template <typename ValueType = float>
struct LookDirection {
	// alfa: left - right - -M_PI : M_PI
	// gamma: bottom - top - -M_PI/2 : M_PI/2
	ValueType alfa, gamma;

	LookDirection(ValueType alfa, ValueType gamma) :
		alfa(alfa), gamma(gamma) { }
};

template <typename ValueType = float>
struct CartesianDirected : public Cartesian<ValueType>, public LookDirection<ValueType> {
private:
	ValueType mymod(ValueType x, ValueType y) {
		return x - y * std::floor(x/y);
	}
	static constexpr ValueType view_bias = 0.1;
public:
	using Cartesian = Cartesian<ValueType>;
	using LookDirection = LookDirection<ValueType>;

	using Cartesian::x;
	using Cartesian::y;
	using Cartesian::z;
	using LookDirection::alfa;
	using LookDirection::gamma;

	CartesianDirected(Cartesian car, LookDirection dir) : Cartesian(car), LookDirection(dir) { }
	CartesianDirected(const CartesianDirected&) = default;
	CartesianDirected(CartesianDirected&&) = default;

	CartesianDirected& moveForward(ValueType dist) {
		x += std::cos(alfa) * std::cos(gamma) * dist;
		y += std::sin(alfa) * std::cos(gamma) * dist;
		z += std::sin(gamma) * dist;
		return *this;
	}
	CartesianDirected& moveBackward(ValueType dist) {
		return moveForward(-dist);
	}
	CartesianDirected& moveRight(ValueType dist) {
		x += std::sin(alfa) * dist;
		y -= std::cos(alfa) * dist;
		return *this;
	}
	CartesianDirected& moveLeft(ValueType dist) {
		return moveRight(-dist);
	}
	CartesianDirected& moveUp(ValueType dist) {
		z += dist;
		return *this;
	}
	CartesianDirected& moveDown(ValueType dist) {
		return moveUp(-dist);
	}
	CartesianDirected& turnRight(ValueType angle) {
		alfa += angle;
		alfa = mymod(alfa, 2*M_PI);
		return *this;
	}
	CartesianDirected& turnLeft(ValueType angle) {
		return turnRight(-angle);
	}
	Cartesian& turnUp(ValueType angle) {
		gamma += angle;
		gamma = std::min<ValueType>(gamma, M_PI/2.f - view_bias);
		gamma = std::max<ValueType>(gamma, -M_PI/2.f + view_bias);
		return *this;
	}
	Cartesian& turnDown(ValueType angle) {
		return turnUp(angle);
	}

	Cartesian getCurrentPoint() const {
		return *this;
	}
	Cartesian getLookAtPoint() const {
		auto result = *this;
		result.moveForward(static_cast<ValueType>(1));
		return result;
	}
	Cartesian getNorth() const {
		auto result = *this;
		result.z = std::numeric_limits<ValueType>::max();
		return result;
	}
	friend std::ostream& operator<<(std::ostream& stream, const CartesianDirected& cart) {
		stream << "CartesianDirected: x=" << cart.x <<
			", y=" << cart.y << ", z=" << cart.z << ", alfa=" << 
			cart.alfa << ", gamma=" << cart.gamma;
		return stream;
	}
};
