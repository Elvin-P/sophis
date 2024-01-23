#include <array>
#include <cmath>
#include "Pendulum.h"
#include "client_comm.h"

#define maxf(a,b) (a>b?a:b)
#define minf(a,b) (a<b?a:b)
#define maxu 9
#define odesteps 10
#ifndef M
#define M 3
#endif // !M



std::array<double, 4> StaticPendulum::simulate(double u, std::array<double, 4> x0, double ts = 0.05) {
  u = u * 2 * maxu - maxu;
  u = maxf(-maxu, minf(maxu, u));

  std::array<double, 4> x = x0;
  double dt = ts / odesteps;
  for (int i = 0; i < odesteps; i++) {
    double thetadot = x[1];
    double alpha = x[2];
    double alphadot = x[3];
    double frac = 1 / (aP * cP - bP * bP * pow(cos(alpha), 2));
    double thetadotdot = frac * (-bP * cP * sin(alpha) * pow(alphadot, 2) + bP * dP * sin(alpha) * cos(alpha) - cP * eP * thetadot + cP * fP * u);
    double alphadotdot = frac * (aP * dP * sin(alpha) - pow(bP, 2) * sin(alpha) * cos(alpha) * pow(alphadot, 2) - bP * eP * cos(alpha) * thetadot + bP * fP * cos(alpha) * u);
    x[0] += dt * thetadot;
    x[1] += dt * thetadotdot;
    x[2] += dt * alphadot;
    x[3] += dt * alphadotdot;
  }
  return x;
}

double StaticPendulum::rewardFunc(std::array<double, 4> x, double u) {
    u = u * 2 * maxu - maxu;
  double reward = 0;
  constexpr std::array<double, 4> rewardWeights = { 1, 0.0, 1, 0.005 };
  constexpr double Rrew = 0.08;
  double nonnorm_max = Rrew * maxu * maxu;

  x[0] = StaticPendulum::normalizeAngle(x[0]);
  x[2] = StaticPendulum::normalizeAngle(x[2]);

  for (int i = 0; i < 4; i++) {
    nonnorm_max += maxx[i] * maxx[i] * rewardWeights[i];
  }

  double xqrewx = 0;
  for (int i = 0; i < 4; i++) {
    xqrewx += x[i] * x[i] * rewardWeights[i];
  }

  reward = 1 - (xqrewx + Rrew * u * u) / nonnorm_max;
  reward = maxf(0, minf(reward, 1));

  return reward;
}

double StaticPendulum::getNormalInput(double uc_left, double uc_right, double ud) {
  double realAction = (uc_left + uc_right) / 2;
  if ((int)ud == 0)
  {
    return realAction;
  }
  if (realAction < 1.0 / M)
  {
    return (1 / (2.0 * M));
  }
  if (realAction > 2.0 / M)
  {
    return ((2.0 + M) / (2.0 * M));
  }
  return 0.5;
}

double StaticPendulum::normalizeAngle(double a) {
  const double mPi = (a >= 0) ? M_PI : -M_PI;
  return fmod(a + mPi, 2.0 * M_PI) - mPi;
}

RealPend::RealPend() {
    this->states = this->readStates();
}

void RealPend::applyInput(double u) {
    double voltageCmd = -(u * 18 - 9);
    write_pend(voltageCmd);
    return;
}
std::array<double, 4> RealPend::readStates() {
    this->states = read_pend();
    return this->states;
}