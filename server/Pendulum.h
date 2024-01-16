#pragma once
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define g 9.80665		// gravity
#define Beq 0.004      // Equivalent viscous damM_PIng coefficient
#define Jeq 0.0035842  // Moment of inertia of the arm and pendulum about the axis of θ
#define Jm 3.87e-7     // Moment of inertia of the rotor of the motor
#define Kg 70        // SRV02 sys. gear ratio (14*5)
#define Km 0.00767     // Back-emf const.
#define Kt 0.00767     // Motor-torque const.
#define Lp 0.1685       // half length of pendulum
#define msss 0.127        // mass of pendulum
#define r 0.216        // rotating arm length
#define Rm 2.6         // armature resistance
#define NUg 0.9        // gearbox efficiency
#define NUm 0.69       // motor efficiency

class Pendulum {
private:
  static constexpr double aP = Jeq + msss * r * r + NUg * Kg * Kg * Jm;
  static constexpr double bP = msss * Lp * r;
  static constexpr double cP = (4 * msss * Lp * Lp) / 3;
  static constexpr double dP = msss * g * Lp;
  static constexpr double eP = Beq + (NUm * NUg * Kt * Km * Kg * Kg) / Rm;
  static constexpr double fP = (NUm * NUg * Kt * Kg) / Rm;

  static constexpr double maxx[4] = { M_PI,100, M_PI, 100 };
public:
  static std::array<double, 4> simulate(double, std::array<double, 4>, double);
  static double rewardFunc(std::array<double, 4>, double);
  static double getNormalInput(double, double, double);
  static double normalizeAngle(double);
};
