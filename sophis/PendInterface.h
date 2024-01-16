#pragma once
#include <array>

class PendInterface {
public:
	virtual std::array<double, 4> readStates() = 0;
	virtual void applyInput(double) = 0;
};

class SimPendulum : PendInterface {
private:
	static constexpr double aP = Jeq + msss * r * r + NUg * Kg * Kg * Jm;
	static constexpr double bP = msss * Lp * r;
	static constexpr double cP = (4 * msss * Lp * Lp) / 3;
	static constexpr double dP = msss * g * Lp;
	static constexpr double eP = Beq + (NUm * NUg * Kt * Km * Kg * Kg) / Rm;
	static constexpr double fP = (NUm * NUg * Kt * Kg) / Rm;
	static constexpr double maxx[4] = { M_PI,100, M_PI, 100 };

	static double getNormalInput(double, double, double);
	static double normalizeAngle(double);

	std::array<double, 4> states;
	double ts;


public:

	std::array<double, 4> simulate(double, std::array<double, 4>, double);
	double rewardFunc(std::array<double, 4>, double);
};